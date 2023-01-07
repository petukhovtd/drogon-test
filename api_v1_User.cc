#include <api_v1_User.h>

#include <string>
#include <iomanip>
#include <optional>

using namespace api::v1;

User::User(const myapp::UserDbPtr &userDb, const myapp::UserSessionPtr &userSession)
    : userDb_(userDb), userSession_(userSession)
{
}

void User::Create(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort();

    Json::Value responseJsonBody;

    auto requestJsonBody = request->getJsonObject();

    if (requestJsonBody == nullptr)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "body is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    if (!requestJsonBody->isMember("login"))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "field \"login\" is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    if (!requestJsonBody->isMember("password"))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "field \"password\" is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    const auto login = requestJsonBody->get("login", "login").asString();
    const auto password = requestJsonBody->get("password", "password").asString();

    auto user = userDb_->AddUser(login, password);

    if (!user)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "login \"" + login + "\" already exist";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    myapp::UserSession::Token token = userSession_->Create(*user);

    LOG_INFO << "User create. login: \"" << login << "\""
             << ", password: \"" << password << "\""
             << ", id: \"" << user->GetId() << "\""
             << ", token: \"" << token << "\"";

    responseJsonBody["user_id"] = static_cast<Json::UInt64>(user->GetId());
    responseJsonBody["session"] = token;
    auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
    response->setStatusCode(k201Created);
    callback(response);
}

void User::Login(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort();

    Json::Value responseJsonBody;

    auto requestJsonBody = request->getJsonObject();

    if (requestJsonBody == nullptr)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "body is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    if (!requestJsonBody->isMember("login"))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "field \"login\" is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    if (!requestJsonBody->isMember("password"))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "field \"password\" is required";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    const auto login = requestJsonBody->get("login", "login").asString();
    const auto password = requestJsonBody->get("password", "password").asString();

    auto user = userDb_->GetUser(login);
    if (!user)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "login \"" + login + "\" don't exist";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    if (!user->CheckPassword(password))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "invalid password";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    auto token = userSession_->Create(*user);
    if (token.empty())
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "login \"" + login + "\" already authorized";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    LOG_INFO << "User login. login: \"" << login << "\""
             << ", password: \"" << password << "\""
             << ", id: \"" << user->GetId() << "\""
             << ", token: \"" << token << "\"";

    auto response = HttpResponse::newHttpResponse();
    response->addHeader("Location", "/api/v1/user/" + std::to_string(user->GetId()));
    response->addHeader("Session", token);
    response->setStatusCode(k200OK);
    callback(response);
}

void User::Logout(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, std::string id)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort();

    Json::Value responseJsonBody;

    std::stringstream ss(id);
    myapp::UserId userId;
    ss >> userId;
    if (!ss)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "convert id: \"" + id + "\" failed";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }

    const auto sessionIt = request->getHeaders().find("session");
    if (sessionIt == request->getHeaders().end())
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "don't found session token";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k400BadRequest);

        callback(response);
        return;
    }
    const myapp::UserSession::Token &token = sessionIt->second;

    auto user = userDb_->GetUser(userId);
    if (!user)
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "user id: \"" + id + "\" don't found";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    if (!userSession_->Check(userId, token))
    {
        responseJsonBody["status"] = "error";
        responseJsonBody["message"] = "user id: \"" + id + "\" invalid session token";

        auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
        response->setStatusCode(HttpStatusCode::k404NotFound);

        callback(response);
        return;
    }

    LOG_INFO << "User logout. id: \"" << userId << "\""
             << ", token: \"" << token << "\"";

    userSession_->Delete(userId);
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(k200OK);
    callback(response);
}

void User::List(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, const std::string &autorizated)
{
    LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
              << ", autorizated: \"" << autorizated << "\"";

    std::optional<bool> autorizatedFilter = [&autorizated]() -> std::optional<bool>
    {
        if (autorizated.empty())
        {
            return std::nullopt;
        }
        std::stringstream ss(autorizated);
        bool result;
        ss >> std::boolalpha >> result;
        if (!ss)
        {
            return std::nullopt;
        }
        return result;
    }();

    Json::Value responseJsonBody;
    responseJsonBody["users"] = Json::arrayValue;

    for (const auto &userIt : *userDb_)
    {
        const bool autorizatedUser = userSession_->Autorizated(userIt.first);

        if (autorizatedFilter.has_value() && autorizatedFilter.value() != autorizatedUser)
        {
            continue;
        }

        Json::Value userJson;
        userJson["user_id"] = static_cast<Json::UInt64>(userIt.first);
        userJson["login"] = userIt.second->GetLogin();
        userJson["autorizated"] = autorizatedUser;

        responseJsonBody["users"].append(userJson);
    }

    LOG_INFO << "User list. response size: " << responseJsonBody["users"].size();

    auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
    response->setStatusCode(k200OK);
    callback(response);
}
