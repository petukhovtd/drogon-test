#pragma once

#include <user/user.h>
#include <user/user_db.h>

#include <libapi/error.h>

#include <drogon/HttpController.h>

#include <variant>

namespace myapp {

class UserHttpController : public drogon::HttpController<UserHttpController, false> {
  using Callback = std::function<void(const drogon::HttpResponsePtr &)>;

public:
  explicit UserHttpController(const myapp::UserDbPtr &userDb);

  ~UserHttpController() override = default;

  METHOD_LIST_BEGIN

    ADD_METHOD_TO(UserHttpController::Create, "/api/v1/user", drogon::Post);
    ADD_METHOD_TO(UserHttpController::List, "/api/v1/user?limit={1}&offset={2}", drogon::Get);
    ADD_METHOD_TO(UserHttpController::Change, "/api/v1/user/{1}", drogon::Get, drogon::Put, drogon::Patch, drogon::Delete);
    ADD_METHOD_TO(UserHttpController::ChangeUsername, "/api/v1/user/{1}/change_username", drogon::Put);
    ADD_METHOD_TO(UserHttpController::ChangePassword, "/api/v1/user/{1}/change_password", drogon::Put);

  METHOD_LIST_END

  void Create(const drogon::HttpRequestPtr &request, Callback &&callback);
  void List(const drogon::HttpRequestPtr &request, Callback &&callback, const std::string &limit, const std::string &offset);
  void Change(const drogon::HttpRequestPtr &request, Callback &&callback, const std::string &userId);
  void ChangeUsername(const drogon::HttpRequestPtr &request, Callback &&callback, const std::string &userId);
  void ChangePassword(const drogon::HttpRequestPtr &request, Callback &&callback, const std::string &userId);

private:
  static bool ErrorResponse(drogon::HttpStatusCode code, const std::vector<myapp::Error> &errors, Callback &callback);

  static void JsonResponse(drogon::HttpStatusCode code, const Json::Value &json, Callback &callback);

  static void HttpResponse(drogon::HttpStatusCode code, Callback &callback);

  std::variant<myapp::UserPtr, myapp::Error> AuthorizateUser(const drogon::HttpRequestPtr &request) const;

  struct ResponseData {
    drogon::HttpStatusCode code;
    Json::Value body;
  };

  std::variant<ResponseData, std::vector<myapp::Error>> UserMethodProcess(const drogon::HttpRequestPtr &request,
                                                                          myapp::User &user);

  static Json::Value GetUser(const myapp::User &user);

  static std::vector<myapp::Error> PutUser(const drogon::HttpRequestPtr &request, myapp::User &user);

  static std::variant<Json::Value, std::vector<myapp::Error>> PatchUser(const drogon::HttpRequestPtr &request,
                                                                        myapp::User &user);

private:
  myapp::UserDbPtr userDb_;
};

}
