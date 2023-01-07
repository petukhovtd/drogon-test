#include <myapp/user_db.h>
#include <myapp/user_session.h>
#include <api_v1_User.h>

#include <drogon/drogon.h>

int main()
{
    auto userDb = std::make_shared<myapp::UserDb>();
    auto userSession = std::make_shared<myapp::UserSession>();
    auto userController = std::make_shared<api::v1::User>(userDb, userSession);

    drogon::app()
        .addListener("0.0.0.0", 3000)
        .setThreadNum(1)
        .registerController(userController)
        .run();

    return EXIT_SUCCESS;
}