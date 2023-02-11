#include <user/user_db.h>

namespace myapp
{
    UserDb::UserDb()
        : db_(), loginIndex_(), lastId_(1)
    {
    }

    UserPtr UserDb::AddUser(const std::string &username, const std::string &password)
    {
        if (loginIndex_.count(username))
        {
            return nullptr;
        }
        UserId id = ++lastId_;
        auto user = std::make_shared<User>(username, password, id);
        db_[id] = user;
        loginIndex_[username] = user;
        return user;
    }

    UserPtr UserDb::GetUser(UserId id) const
    {
        const auto &it = db_.find(id);
        if (it == db_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    UserPtr UserDb::GetUser(const std::string &username) const
    {
        const auto &it = loginIndex_.find(username);
        if (it == loginIndex_.end())
        {
            return nullptr;
        }
        auto user = it->second.lock();
        return user;
    }

    UserDb::UserDbConstIt UserDb::begin() const
    {
        return db_.cbegin();
    }

    UserDb::UserDbConstIt UserDb::end() const
    {
        return db_.cend();
    }
}
