#include "user_session.h"

#include <myapp/misc.h>

#include <jsoncpp/json/json.h>

#include <chrono>

namespace myapp
{
    namespace
    {
        std::string CreateToken(const User &user)
        {
            Json::Value root;
            Json::StreamWriterBuilder streamBuilder;
            streamBuilder["indentation"] = "";

            root["login"] = user.GetLogin();
            root["password"] = user.GetPasswordHash();
            root["timestamp"] = static_cast<Json::UInt64>(std::chrono::system_clock::now().time_since_epoch().count());

            std::string jsonToken = Json::writeString(streamBuilder, root);
            return CalculateMd5(jsonToken);
        }
    }

    const UserSession::Token &UserSession::Create(const User &user)
    {
        if (db_.count(user.GetId()))
        {
            static Token emptyToken;
            return emptyToken;
        }
        db_[user.GetId()] = CreateToken(user);
        return db_[user.GetId()];
    }

    const UserSession::Token &UserSession::Update(const User &user)
    {
        auto it = db_.find(user.GetId());
        if (it == db_.end())
        {
            static Token emptyToken;
            return emptyToken;
        }
        db_[user.GetId()] = CreateToken(user);
        return db_[user.GetId()];
    }

    bool UserSession::Check(UserId userId, const Token &token) const
    {
        const auto it = db_.find(userId);
        if (it == db_.end())
        {
            return false;
        }
        return it->second == token;
    }

    void UserSession::Delete(UserId userId)
    {
        db_.erase(userId);
    }

    bool UserSession::Autorizated(UserId userId) const
    {
        return db_.count(userId) != 0;
    }
}