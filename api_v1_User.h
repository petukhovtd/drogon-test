#pragma once

#include <myapp/user_db.h>
#include <myapp/user_session.h>

#include <drogon/HttpController.h>

#include <unordered_map>

using namespace drogon;

namespace api
{
  namespace v1
  {
    class User : public drogon::HttpController<User, false>
    {
    public:
      explicit User(const myapp::UserDbPtr &userDb, const myapp::UserSessionPtr &userSession);

      ~User() override = default;

      METHOD_LIST_BEGIN

      METHOD_ADD(User::Create, "/create", Post);
      METHOD_ADD(User::Login, "/login", Post);
      METHOD_ADD(User::Logout, "/{1}/logout", Post);
      METHOD_ADD(User::List, "/list?autorizated={1}", Get);

      METHOD_LIST_END

      void Create(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback);
      void Login(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback);
      void Logout(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, std::string userId);
      void List(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback, const std::string &autorizated);

    private:
      myapp::UserDbPtr userDb_;
      myapp::UserSessionPtr userSession_;
    };
  }
}
