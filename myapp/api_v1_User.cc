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

    std::vector<myapp::Error> errors;

    auto userIdOpt = myapp::Extract<myapp::UserId>(userId);
    if (!userIdOpt.has_value())
    {
        myapp::Error error(myapp::Error::Code::ConvertParameterFailed);
        LOG_ERROR << error.GetMessage();
        errors.push_back(error);
    }

    auto authRes = AuthorizateUser(request);
    if (std::holds_alternative<myapp::UserPtr>(authRes))
    {
        // good
    }
    else if (std::holds_alternative<myapp::Error>(authRes))
    {
        auto error = std::get<myapp::Error>(authRes);
        auto response = HttpResponse::newHttpJsonResponse(error.GetJson());
        response->addHeader("WWW-Authenticate", "Basic");
        response->setStatusCode(k401Unauthorized);
        callback(response);
        return;
    }

    myapp::UserPtr user = std::get<myapp::UserPtr>(authRes);
    if (userIdOpt.has_value())
    {
        if (user->GetId() != userIdOpt.value())
        {
            auto error = myapp::Error(myapp::Error::Code::InvalidUserId, {{"why", "authorization user and parameter not equal"}});
            LOG_ERROR << error.GetMessage();
            errors.push_back(error);
        }
    }

    if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback))
    {
        return;
    }

    auto processRes = UserMethodProcess(request->getMethod(), *user);
    if (std::holds_alternative<Json::Value>(processRes))
    {
        auto responseJson = std::get<Json::Value>(processRes);
        JsonResponse(k200OK, responseJson, callback);
        return;
    }
    else if (std::holds_alternative<std::vector<myapp::Error>>(processRes))
    {
        auto responseError = std::get<std::vector<myapp::Error>>(processRes);
        ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback);
        return;
    }

    User::HttpResponse(k400BadRequest, callback);
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

void User::HttpResponse(HttpStatusCode code, Callback &callback)
{
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(code);
    callback(response);
}

std::variant<myapp::UserPtr, myapp::Error> User::AuthorizateUser(const HttpRequestPtr &request) const
{
    const auto &auth = request->getHeader("Authorization");
    if (auth.empty())
    {
        auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "Authorization header don't found"}});
        LOG_ERROR << error.GetMessage();
        return error;
    }

    std::string_view authStr(auth);
    std::string_view credinalsBase;
    {
        size_t pos = authStr.find(' ');
        if (pos == std::string::npos)
        {
            auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid authorization header"}});
            LOG_ERROR << error.GetMessage();
            return error;
        }
        std::string_view authType = authStr.substr(0, pos);
        if (authType != "Basic")
        {
            auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "authorization type not Basic"}});
            LOG_ERROR << error.GetMessage();
            return error;
        }
        credinalsBase = authStr.substr(++pos);
    }

    std::string credentialsStr = base64_decode(credinalsBase);
    std::string_view credentials(credentialsStr);

    size_t pos = credentials.find(':');
    if (pos == std::string::npos)
    {
        auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid autorization credentials"}});
        LOG_ERROR << error.GetMessage();
        return error;
    }
    std::string_view name = credentials.substr(0, pos);
    std::string_view password = credentials.substr(++pos);

    auto user = userDb_->GetUser(std::string(name));
    if (!user)
    {
        auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "user not found"}});
        LOG_ERROR << error.GetMessage();
        return error;
    }
    if (!user->CheckPassword(std::string(password)))
    {
        auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid password"}});
        LOG_ERROR << error.GetMessage();
        return error;
    }
    return user;
}

std::variant<Json::Value, std::vector<myapp::Error>> User::UserMethodProcess(HttpMethod method, myapp::User &user)
{
    std::vector<myapp::Error> errors;

    switch (method)
    {
    case Get:
        return GetUser(user);
    case Put:
    case Patch:
    case Delete:
    default:
        return errors;
    }

    return errors;
}

Json::Value User::GetUser(const myapp::User &user)
{
    Json::Value result;

    result[myapp::key::userId] = static_cast<Json::UInt64>(user.GetId());
    result[myapp::key::username] = user.GetUsername();
    result[myapp::key::info::firstName] = user.GetInfo().firstName;
    result[myapp::key::info::lastName] = user.GetInfo().lastName;

    return result;
}