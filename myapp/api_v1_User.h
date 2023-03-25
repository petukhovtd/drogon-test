#pragma once

#include <user/user.h>
#include <user/user_db.h>

#include <myapp/error.h>

#include <drogon/HttpController.h>

#include <unordered_map>
#include <variant>

using namespace drogon;

namespace api
{
  namespace v1
  {
    class User : public drogon::HttpController<User, false>
    {
      using Callback = std::function<void(const HttpResponsePtr &)>;

    public:
      explicit User(const myapp::UserDbPtr &userDb);

      ~User() override = default;

      METHOD_LIST_BEGIN

      METHOD_ADD(User::Create, "/create", Post);
      METHOD_ADD(User::List, "/list?limit={1}&offset={2}", Get);
      METHOD_ADD(User::Change, "/{1}", Get, Put, Patch, Delete);

      METHOD_LIST_END

      void Create(const HttpRequestPtr &request, Callback &&callback);
      void List(const HttpRequestPtr &request, Callback &&callback, std::string limit, std::string offset);
      void Change(const HttpRequestPtr &request, Callback &&callback, std::string userId);

    private:
      static bool ErrorResponse( HttpStatusCode code, const std::vector< myapp::Error >& errors, Callback &callback);

      static void JsonResponse( HttpStatusCode code, const Json::Value &json, Callback &callback);

      static void HttpResponse( HttpStatusCode code, Callback &callback);

      std::variant<myapp::UserPtr, myapp::Error> AuthorizateUser(const HttpRequestPtr &request) const;

      static std::variant<Json::Value, std::vector<myapp::Error>> UserMethodProcess(HttpMethod method, myapp::User& user);

      static Json::Value GetUser(const myapp::User& user);

    private:
      myapp::UserDbPtr userDb_;
    };
  }
}
