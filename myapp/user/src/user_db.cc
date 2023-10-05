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

    void UserDb::DeleteUser(UserId id)
    {
        const auto userPtr = GetUser(id);
        if(!userPtr)
        {
            return;
        }
        DeleteUser(*userPtr);
    }

    void UserDb::DeleteUser(const std::string &username)
    {
        const auto userPtr = GetUser(username);
        if(!userPtr)
        {
            return;
        }
        DeleteUser(*userPtr);
    }

    void UserDb::DeleteUser(const User &user)
    {
        db_.erase(user.GetId());
        loginIndex_.erase(user.GetUsername());
    }

    UserPtr UserDb::ChangeUsername(const User &user, const std::string &newUsername)
    {
        const auto it = loginIndex_.find(newUsername);
        if(loginIndex_.end() != it)
        {
            return nullptr;
        }
        auto newUser = std::make_shared<User>(user, newUsername);
        db_[user.GetId()] = newUser;
        loginIndex_.erase(user.GetUsername());
        loginIndex_[newUser->GetUsername()]=newUser;
        return newUser;
    }

    UserPtr UserDb::ChangePassword(const User &user, const std::string &newPassword)
    {
        auto newUser = std::make_shared<User>(user.GetUsername(), newPassword, user.GetId());
        newUser->SetInfo(user.GetInfo());
        db_[user.GetId()] = newUser;
        loginIndex_[user.GetUsername()] = newUser;
        return newUser;
    }

    UserDb::UserDbConstIt UserDb::begin() const
    {
        return db_.cbegin();
    }

    UserDb::UserDbConstIt UserDb::end() const
    {
        return db_.cend();
    }

    UserDb::UserDbIt UserDb::begin()
    {
        return db_.begin();
    }

    UserDb::UserDbIt UserDb::end()
    {
        return db_.end();
    }

    size_t UserDb::size() const
    {
        return db_.size();
    }

}
