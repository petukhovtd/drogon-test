#include "api_v1_User.h"

#include <string>
#include <iomanip>

using namespace api::v1;

User::User(const myapp::UserDbPtr &userDb)
    : userDb_(userDb)
{
}

void User::create(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << "User create";

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

    LOG_DEBUG << "User create login: \"" << login << "\", password: \"" << password << "\", id: \"" << user->GetId() << "\"";

    auto response = HttpResponse::newHttpResponse();
    response->addHeader("Location", "/api/v1/user/" + std::to_string(user->GetId()));
    response->setStatusCode(k201Created);
    callback(response);
}
