#include <myapp/api_v1_User.h>
#include <myapp/checkers.h>
#include <myapp/api_keys.h>

#include <cpp-base64/base64.h>

#include <string>
#include <iomanip>
#include <optional>

using namespace api::v1;

User::User(const myapp::UserDbPtr &userDb)
    : userDb_(userDb)
{
}

void User::Create(const HttpRequestPtr &request, Callback &&callback)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort();

    auto requestJsonBody = request->getJsonObject();

    if (!requestJsonBody)
    {
        myapp::Error error(myapp::Error::Code::ExpectJsonBody);
        LOG_ERROR << error.GetMessage();

        ErrorResponse(HttpStatusCode::k400BadRequest, {error}, callback);
        return;
    }

    std::vector<myapp::Error> errors;

    std::string username;
    {
        const auto usernameVariant = myapp::ExtractUsername(*requestJsonBody);
        if (std::holds_alternative<std::string>(usernameVariant))
        {
            username = std::get<std::string>(usernameVariant);
        }
        else
        {
            const auto &error = std::get<myapp::Error>(usernameVariant);
            LOG_ERROR << error.GetMessage();
            errors.push_back(error);
        }
    }

    std::string password;
    {
        const auto passowordVariant = myapp::ExtractPassword(*requestJsonBody);
        if (std::holds_alternative<std::string>(passowordVariant))
        {
            password = std::get<std::string>(passowordVariant);
        }
        else
        {
            const auto &error = std::get<myapp::Error>(passowordVariant);
            LOG_ERROR << error.GetMessage();
            errors.push_back(error);
        }
    }

    if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback))
    {
        return;
    }

    auto user = userDb_->AddUser(username, password);
    if (!user)
    {
        myapp::Error error(myapp::Error::Code::UserAlreadyExist, {{myapp::key::username, username}});
        LOG_ERROR << error.GetMessage();
        ErrorResponse(HttpStatusCode::k404NotFound, {error}, callback);
        return;
    }

    LOG_INFO << "User create. username: \"" << username << "\""
             << ", password: \"" << password << "\""
             << ", id: \"" << user->GetId() << "\"";

    Json::Value response;
    response[myapp::key::userId] = static_cast<Json::UInt64>(user->GetId());

    JsonResponse(k201Created, response, callback);
}

void User::List(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, std::string limitParam, std::string offsetParam)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
              << ", limit: " << limitParam << ", offset: " << offsetParam;

    std::vector<myapp::Error> errors;

    std::optional<size_t> limitOpt = std::nullopt;
    if (!limitParam.empty())
    {
        limitOpt = myapp::Extract<size_t>(limitParam);
        if (!limitOpt.has_value())
        {
            myapp::Error error(myapp::Error::Code::ConvertParameterFailed, {{"key", "limit"}});
            LOG_ERROR << error.GetMessage();
            errors.push_back(error);
        }
    }

    std::optional<size_t> offsetOpt = std::nullopt;
    if (!offsetParam.empty())
    {
        offsetOpt = myapp::Extract<size_t>(offsetParam);
        if (!offsetOpt.has_value())
        {
            myapp::Error error(myapp::Error::Code::ConvertParameterFailed, {{"key", "offset"}});
            LOG_ERROR << error.GetMessage();
            errors.push_back(error);
        }
    }

    if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback))
    {
        return;
    }

    size_t total = userDb_->size();
    size_t offset = 0;
    size_t limit = total;
    size_t size = total;
    size_t end = total;

    if (offsetOpt.has_value())
    {
        offset = offsetOpt.value();
    }

    if (limitOpt.has_value())
    {
        limit = limitOpt.value();
        end = offset + limit;
        if (end > total)
        {
            end = total;
        }
    }

    if (offset > total)
    {
        size = 0;
    }
    else
    {
        size = end - offset;
    }

    Json::Value responseJsonBody;
    responseJsonBody["users"] = Json::arrayValue;
    responseJsonBody["size"] = static_cast<unsigned int>(size);
    responseJsonBody["total"] = static_cast<unsigned int>(total);
    if (offsetOpt.has_value())
    {
        responseJsonBody["offset"] = static_cast<unsigned int>(offset);
    }
    if (limitOpt.has_value())
    {
        responseJsonBody["limit"] = static_cast<unsigned int>(limit);
    }

    if (0 == size)
    {
        LOG_INFO << "Empty response";
        JsonResponse(k200OK, responseJsonBody, callback);
        return;
    }

    std::vector<myapp::UserPtr> users;
    for (auto [id, user] : *userDb_)
    {
        users.push_back(user);
    }

    std::sort(users.begin(), users.end(), [](const myapp::UserPtr &lhs, const myapp::UserPtr &rhs)
              { return lhs->GetId() < rhs->GetId(); });

    for (size_t i = offset; i < end; ++i)
    {
        const myapp::UserPtr &user = users[i];
        Json::Value userJson;
        userJson[myapp::key::userId] = static_cast<Json::UInt64>(user->GetId());
        userJson[myapp::key::username] = user->GetUsername();
        responseJsonBody["users"].append(userJson);
    }

    LOG_INFO << "User list. response size: " << size;

    auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
    response->setStatusCode(k200OK);
    callback(response);
}

void User::Change(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, std::string userId)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
              << ", userId: " << userId;

    auto authRes = AuthorizateUser(request);
    if (std::holds_alternative<myapp::UserPtr>(authRes))
    {
    }
    else if (std::holds_alternative<Json::Value>(authRes))
    {
        auto errorJson = std::get<Json::Value>(authRes);
        auto response = HttpResponse::newHttpJsonResponse(errorJson);
        response->addHeader("WWW-Authenticate", "Basic");
        response->setStatusCode(k401Unauthorized);
        callback(response);
        return;
    }

    if (request->getMethod() == Get)
    {
        Json::Value responseJson;
    }

    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(k200OK);
    callback(response);
}

bool User::ErrorResponse(HttpStatusCode code, const std::vector<myapp::Error> &errors, Callback &callback)
{
    if (errors.empty())
    {
        return false;
    }

    Json::Value errorArray(Json::arrayValue);
    for (const auto &error : errors)
    {
        errorArray.append(error.GetJson());
    }

    Json::Value jsonResponse;
    jsonResponse["error"] = errorArray;

    JsonResponse(code, jsonResponse, callback);

    return true;
}

void User::JsonResponse(HttpStatusCode code, const Json::Value &json, Callback &callback)
{
    auto response = HttpResponse::newHttpJsonResponse(json);
    response->setStatusCode(code);
    callback(response);
}

std::variant<myapp::UserPtr, Json::Value> User::AuthorizateUser(const HttpRequestPtr &request) const
{
    const auto &auth = request->getHeader("Authorization");
    if (auth.empty())
    {
        Json::Value error;
        error["message"] = "authorization header not found";
        return error;
    }

    std::string_view authStr(auth);
    std::string_view credinalsBase;
    {
        size_t pos = authStr.find(' ');
        if (pos == std::string::npos)
        {
            Json::Value error;
            error["message"] = "invalid authorization header";
            return error;
        }
        std::string_view authType = authStr.substr(0, pos);
        if (authType != "Basic")
        {
            Json::Value error;
            error["message"] = "authorization type not Basic";
            return error;
        }
        credinalsBase = authStr.substr(++pos);
    }
    std::string_view credentials = base64_decode(credinalsBase);
    size_t pos = credentials.find(':');
    if (pos == std::string::npos)
    {
        Json::Value error;
        error["message"] = "invalid autorization credentials";
        return error;
    }
    std::string_view name = credentials.substr(0, pos);
    std::string_view password = credentials.substr(++pos);

    auto user = userDb_->GetUser(std::string(name));
    if (!user)
    {
        Json::Value error;
        error["message"] = "user not found";
        return error;
    }
    if (!user->CheckPassword(std::string(password)))
    {
        Json::Value error;
        error["message"] = "invalid password";
        return error;
    }
    return user;
}