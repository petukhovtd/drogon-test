#pragma once

#include <myapp/user_db.h>

#include <drogon/HttpController.h>

#include <unordered_map>

using namespace drogon;

namespace api
{
  namespace v1
  {
    class User : public drogon::HttpController<User,false>
    {
    public:
      explicit User(const myapp::UserDbPtr &userDb);

      ~User() override = default;

      METHOD_LIST_BEGIN

      METHOD_ADD(User::create, "/create", Post);

      METHOD_LIST_END

      void create(const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback);

    private:
      myapp::UserDbPtr userDb_;
    };
  }
}
