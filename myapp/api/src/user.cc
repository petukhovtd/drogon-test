#include <api/v1/user.h>
#include <libapi/checkers.h>
#include <libapi/api_keys.h>

#include <cpp-base64/base64.h>

#include <string>
#include <optional>

using namespace api::v1;

User::User(const myapp::UserDbPtr &userDb)
    : userDb_(userDb) {
}

void User::Create(const HttpRequestPtr &request, Callback &&callback) {
  LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort();

  auto requestJsonBody = request->getJsonObject();

  if (!requestJsonBody) {
    myapp::Error error(myapp::Error::Code::ExpectJsonBody);
    LOG_ERROR << error.GetMessage();

    ErrorResponse(HttpStatusCode::k400BadRequest, {error}, callback);
    return;
  }

  std::vector<myapp::Error> errors;

  std::string username;
  {
    const auto usernameVariant = myapp::ExtractUsername(*requestJsonBody);
    if (std::holds_alternative<std::string>(usernameVariant)) {
      username = std::get<std::string>(usernameVariant);
    } else {
      const auto &error = std::get<myapp::Error>(usernameVariant);
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  std::string password;
  {
    const auto passowordVariant = myapp::ExtractPassword(*requestJsonBody);
    if (std::holds_alternative<std::string>(passowordVariant)) {
      password = std::get<std::string>(passowordVariant);
    } else {
      const auto &error = std::get<myapp::Error>(passowordVariant);
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback)) {
    return;
  }

  auto user = userDb_->AddUser(username, password);
  if (!user) {
    myapp::Error error(myapp::Error::Code::UserAlreadyExist, {{myapp::key::username, username}});
    LOG_ERROR << error.GetMessage();
    ErrorResponse(HttpStatusCode::k409Conflict, {error}, callback);
    return;
  }

  LOG_INFO << "User create. username: \"" << username << "\""
           << ", password: \"" << password << "\""
           << ", id: \"" << user->GetId() << "\"";

  Json::Value response;
  response[myapp::key::userId] = static_cast<Json::UInt64>(user->GetId());

  JsonResponse(k201Created, response, callback);
}

void User::List(const HttpRequestPtr &request,
                std::function<void(const HttpResponsePtr &)> &&callback,
                const std::string &limitParam,
                const std::string &offsetParam) {
  LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
            << ", limit: " << limitParam << ", offset: " << offsetParam;

  std::vector<myapp::Error> errors;

  std::optional<size_t> limitOpt = std::nullopt;
  if (!limitParam.empty()) {
    limitOpt = myapp::Extract<size_t>(limitParam);
    if (!limitOpt.has_value()) {
      myapp::Error error(myapp::Error::Code::ConvertParameterFailed, {{"key", "limit"}});
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  std::optional<size_t> offsetOpt = std::nullopt;
  if (!offsetParam.empty()) {
    offsetOpt = myapp::Extract<size_t>(offsetParam);
    if (!offsetOpt.has_value()) {
      myapp::Error error(myapp::Error::Code::ConvertParameterFailed, {{"key", "offset"}});
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback)) {
    return;
  }

  size_t total = userDb_->size();
  size_t offset = 0;
  size_t limit = total;
  size_t size = total;
  size_t end = total;

  if (offsetOpt.has_value()) {
    offset = offsetOpt.value();
  }

  if (limitOpt.has_value()) {
    limit = limitOpt.value();
    end = offset + limit;
    if (end > total) {
      end = total;
    }
  }

  if (offset > total) {
    size = 0;
  } else {
    size = end - offset;
  }

  Json::Value responseJsonBody;
  responseJsonBody["users"] = Json::arrayValue;
  responseJsonBody["size"] = static_cast<unsigned int>(size);
  responseJsonBody["total"] = static_cast<unsigned int>(total);
  if (offsetOpt.has_value()) {
    responseJsonBody["offset"] = static_cast<unsigned int>(offset);
  }
  if (limitOpt.has_value()) {
    responseJsonBody["limit"] = static_cast<unsigned int>(limit);
  }

  if (0 == size) {
    LOG_INFO << "Empty response";
    JsonResponse(k200OK, responseJsonBody, callback);
    return;
  }

  std::vector<myapp::UserPtr> users;
  for (auto [id, user] : *userDb_) {
    users.push_back(user);
  }

  std::sort(users.begin(),
            users.end(),
            [](const myapp::UserPtr &lhs, const myapp::UserPtr &rhs) { return lhs->GetId() < rhs->GetId(); });

  for (size_t i = offset; i < end; ++i) {
    const myapp::UserPtr &user = users[i];
    Json::Value userJson;
    userJson[myapp::key::userId] = static_cast<Json::UInt64>(user->GetId());
    userJson[myapp::key::username] = user->GetUsername();
    responseJsonBody["users"].append(userJson);
  }

  LOG_INFO << "User list. response size: " << size;

  auto response = HttpResponse::newHttpJsonResponse(responseJsonBody);
  response->setStatusCode(k200OK);
  callback(response);
}

void User::Change(const HttpRequestPtr &request,
                  std::function<void(const HttpResponsePtr &)> &&callback,
                  const std::string &userId) {
  LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
            << ", userId: " << userId;

  auto userIdOpt = myapp::Extract<myapp::UserId>(userId);
  if (!userIdOpt.has_value()) {
    myapp::Error error(myapp::Error::Code::ConvertParameterFailed);
    LOG_ERROR << error.GetMessage();
    ErrorResponse(k400BadRequest, {error}, callback);
    return;
  }

  auto authRes = AuthorizateUser(request);
  if (std::holds_alternative<myapp::UserPtr>(authRes)) {
    // good
  } else if (std::holds_alternative<myapp::Error>(authRes)) {
    auto error = std::get<myapp::Error>(authRes);
    auto response = HttpResponse::newHttpJsonResponse(error.GetJson());
    response->addHeader("WWW-Authenticate", "Basic");
    response->setStatusCode(k401Unauthorized);
    callback(response);
    return;
  }

  myapp::UserPtr user = std::get<myapp::UserPtr>(authRes);
  if (userIdOpt.has_value()) {
    if (user->GetId() != userIdOpt.value()) {
      auto error =
          myapp::Error(myapp::Error::Code::InvalidUserId, {{"why", "authorization user and parameter not equal"}});
      LOG_ERROR << error.GetMessage();
      ErrorResponse(k400BadRequest, {error}, callback);
      return;
    }
  }

  auto processResult = UserMethodProcess(request, *user);
  if (std::holds_alternative<ResponseData>(processResult)) {
    auto responseData = std::get<ResponseData>(processResult);
    JsonResponse(responseData.code, responseData.body, callback);
    return;
  } else if (std::holds_alternative<std::vector<myapp::Error>>(processResult)) {
    auto responseErrors = std::get<std::vector<myapp::Error>>(processResult);
    ErrorResponse(HttpStatusCode::k400BadRequest, responseErrors, callback);
    return;
  }

  User::HttpResponse(k400BadRequest, callback);
}

void User::ChangeUsername(const HttpRequestPtr &request, Callback &&callback, const std::string &userId) {
  LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
            << ", userId: " << userId;

  auto userIdOpt = myapp::Extract<myapp::UserId>(userId);
  if (!userIdOpt.has_value()) {
    myapp::Error error(myapp::Error::Code::ConvertParameterFailed);
    LOG_ERROR << error.GetMessage();
    ErrorResponse(k400BadRequest, {error}, callback);
    return;
  }

  auto requestJsonBody = request->getJsonObject();
  if (!requestJsonBody) {
    myapp::Error error(myapp::Error::Code::ExpectJsonBody);
    LOG_ERROR << error.GetMessage();

    ErrorResponse(HttpStatusCode::k400BadRequest, {error}, callback);
    return;
  }

  auto authRes = AuthorizateUser(request);
  if (std::holds_alternative<myapp::UserPtr>(authRes)) {
    // good
  } else if (std::holds_alternative<myapp::Error>(authRes)) {
    auto error = std::get<myapp::Error>(authRes);
    auto response = HttpResponse::newHttpJsonResponse(error.GetJson());
    response->addHeader("WWW-Authenticate", "Basic");
    response->setStatusCode(k401Unauthorized);
    callback(response);
    return;
  }

  myapp::UserPtr user = std::get<myapp::UserPtr>(authRes);
  if (userIdOpt.has_value()) {
    if (user->GetId() != userIdOpt.value()) {
      auto error =
          myapp::Error(myapp::Error::Code::InvalidUserId, {{"why", "authorization user and parameter not equal"}});
      LOG_ERROR << error.GetMessage();
      ErrorResponse(k400BadRequest, {error}, callback);
      return;
    }
  }

  std::vector<myapp::Error> errors;
  std::string username;
  {
    const auto usernameVariant = myapp::ExtractUsername(*requestJsonBody);
    if (std::holds_alternative<std::string>(usernameVariant)) {
      username = std::get<std::string>(usernameVariant);
    } else {
      const auto &error = std::get<myapp::Error>(usernameVariant);
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback)) {
    return;
  }

  const auto newUser = userDb_->ChangeUsername(*user, username);
  if (!newUser) {
    myapp::Error error(myapp::Error::Code::UserAlreadyExist, {{myapp::key::username, username}});
    LOG_ERROR << error.GetMessage();
    ErrorResponse(HttpStatusCode::k404NotFound, {error}, callback);
    return;
  }

  Json::Value userJson;
  userJson[myapp::key::userId] = static_cast<Json::UInt64>(newUser->GetId());
  userJson[myapp::key::username] = newUser->GetUsername();

  auto response = HttpResponse::newHttpJsonResponse(userJson);
  response->setStatusCode(k200OK);
  callback(response);
}

void User::ChangePassword(const HttpRequestPtr &request, User::Callback &&callback, const std::string &userId) {
  LOG_DEBUG << request->getPeerAddr().toIp() << ":" << request->getPeerAddr().toPort()
            << ", userId: " << userId;

  auto userIdOpt = myapp::Extract<myapp::UserId>(userId);
  if (!userIdOpt.has_value()) {
    myapp::Error error(myapp::Error::Code::ConvertParameterFailed);
    LOG_ERROR << error.GetMessage();
    ErrorResponse(k400BadRequest, {error}, callback);
    return;
  }

  auto requestJsonBody = request->getJsonObject();
  if (!requestJsonBody) {
    myapp::Error error(myapp::Error::Code::ExpectJsonBody);
    LOG_ERROR << error.GetMessage();

    ErrorResponse(HttpStatusCode::k400BadRequest, {error}, callback);
    return;
  }

  auto authRes = AuthorizateUser(request);
  if (std::holds_alternative<myapp::UserPtr>(authRes)) {
    // good
  } else if (std::holds_alternative<myapp::Error>(authRes)) {
    auto error = std::get<myapp::Error>(authRes);
    auto response = HttpResponse::newHttpJsonResponse(error.GetJson());
    response->addHeader("WWW-Authenticate", "Basic");
    response->setStatusCode(k401Unauthorized);
    callback(response);
    return;
  }

  myapp::UserPtr user = std::get<myapp::UserPtr>(authRes);
  if (userIdOpt.has_value()) {
    if (user->GetId() != userIdOpt.value()) {
      auto error =
          myapp::Error(myapp::Error::Code::InvalidUserId, {{"why", "authorization user and parameter not equal"}});
      LOG_ERROR << error.GetMessage();
      ErrorResponse(k400BadRequest, {error}, callback);
      return;
    }
  }

  std::vector<myapp::Error> errors;
  std::string password;
  {
    const auto passwordVariant = myapp::ExtractPassword(*requestJsonBody);
    if (std::holds_alternative<std::string>(passwordVariant)) {
      password = std::get<std::string>(passwordVariant);
    } else {
      const auto &error = std::get<myapp::Error>(passwordVariant);
      LOG_ERROR << error.GetMessage();
      errors.push_back(error);
    }
  }

  if (ErrorResponse(HttpStatusCode::k400BadRequest, errors, callback)) {
    return;
  }

  const auto newUser = userDb_->ChangePassword(*user, password);

  auto response = HttpResponse::newHttpResponse();
  response->setStatusCode(k204NoContent);
  callback(response);
}

bool User::ErrorResponse(HttpStatusCode code, const std::vector<myapp::Error> &errors, Callback &callback) {
  if (errors.empty()) {
    return false;
  }

  Json::Value errorArray(Json::arrayValue);
  for (const auto &error : errors) {
    errorArray.append(error.GetJson());
  }

  Json::Value jsonResponse;
  jsonResponse["error"] = errorArray;

  JsonResponse(code, jsonResponse, callback);

  return true;
}

void User::JsonResponse(HttpStatusCode code, const Json::Value &json, Callback &callback) {
  auto response = HttpResponse::newHttpJsonResponse(json);
  response->setStatusCode(code);
  callback(response);
}

void User::HttpResponse(HttpStatusCode code, Callback &callback) {
  auto response = HttpResponse::newHttpResponse();
  response->setStatusCode(code);
  callback(response);
}

std::variant<myapp::UserPtr, myapp::Error> User::AuthorizateUser(const HttpRequestPtr &request) const {
  const auto &auth = request->getHeader("Authorization");
  if (auth.empty()) {
    auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "Authorization header don't found"}});
    LOG_ERROR << error.GetMessage();
    return error;
  }

  std::string_view authStr(auth);
  std::string_view credinalsBase;
  {
    size_t pos = authStr.find(' ');
    if (pos == std::string::npos) {
      auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid authorization header"}});
      LOG_ERROR << error.GetMessage();
      return error;
    }
    std::string_view authType = authStr.substr(0, pos);
    if (authType != "Basic") {
      auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "authorization type not Basic"}});
      LOG_ERROR << error.GetMessage();
      return error;
    }
    credinalsBase = authStr.substr(++pos);
  }

  std::string credentialsStr = base64_decode(credinalsBase);
  std::string_view credentials(credentialsStr);

  size_t pos = credentials.find(':');
  if (pos == std::string::npos) {
    auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid autorization credentials"}});
    LOG_ERROR << error.GetMessage();
    return error;
  }
  std::string_view name = credentials.substr(0, pos);
  std::string_view password = credentials.substr(++pos);

  auto user = userDb_->GetUser(std::string(name));
  if (!user) {
    auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "user not found"}});
    LOG_ERROR << error.GetMessage();
    return error;
  }
  if (!user->CheckPassword(std::string(password))) {
    auto error = myapp::Error(myapp::Error::Code::AuthorizateFailed, {{"why", "invalid password"}});
    LOG_ERROR << error.GetMessage();
    return error;
  }
  return user;
}

std::variant<User::ResponseData, std::vector<myapp::Error>> User::UserMethodProcess(const HttpRequestPtr &request,
                                                                                    myapp::User &user) {
  switch (request->getMethod()) {
  case Get:return ResponseData{k200OK, GetUser(user)};
  case Put: {
    auto errors = PutUser(request, user);
    if (errors.empty()) {
      return ResponseData{k204NoContent};
    } else {
      return errors;
    }
  }
  case Patch: {
    auto patchResult = PatchUser(request, user);
    if (std::holds_alternative<Json::Value>(patchResult)) {
      return ResponseData{k200OK, std::get<Json::Value>(patchResult)};
    } else {
      return std::get<std::vector<myapp::Error>>(patchResult);
    }
  }
  case Delete: {
    userDb_->DeleteUser(user);
    return ResponseData{k204NoContent};
  }
  default:break;
  }

  return {};
}

Json::Value User::GetUser(const myapp::User &user) {
  Json::Value result;

  result[myapp::key::userId] = static_cast<Json::UInt64>(user.GetId());
  result[myapp::key::username] = user.GetUsername();
  result[myapp::key::info::firstName] = user.GetInfo().firstName;
  result[myapp::key::info::lastName] = user.GetInfo().lastName;

  return result;
}

std::vector<myapp::Error> User::PutUser(const HttpRequestPtr &request, myapp::User &user) {
  auto requestJsonBody = request->getJsonObject();

  if (!requestJsonBody) {
    myapp::Error error(myapp::Error::Code::ExpectJsonBody);
    return {error};
  }

  myapp::User::Info newInfo;
  std::vector<myapp::Error> errors;

  const auto firstNameResult = myapp::GetString(myapp::key::info::firstName, *requestJsonBody);
  if (std::holds_alternative<std::string>(firstNameResult)) {
    newInfo.firstName = std::get<std::string>(firstNameResult);
  } else {
    errors.push_back(std::get<myapp::Error>(firstNameResult));
  }

  const auto lastNameResult = myapp::GetString(myapp::key::info::lastName, *requestJsonBody);
  if (std::holds_alternative<std::string>(lastNameResult)) {
    newInfo.lastName = std::get<std::string>(lastNameResult);
  } else {
    errors.push_back(std::get<myapp::Error>(lastNameResult));
  }

  if (!errors.empty()) {
    return errors;
  }

  user.SetInfo(std::move(newInfo));

  return {};
}

std::variant<Json::Value, std::vector<myapp::Error>> User::PatchUser(const HttpRequestPtr &request, myapp::User &user) {
  const auto requestJsonBody = request->getJsonObject();

  if (!requestJsonBody) {
    myapp::Error error(myapp::Error::Code::ExpectJsonBody);
    return std::vector{error};
  }

  myapp::User::Info newInfo = user.GetInfo();
  std::vector<myapp::Error> errors;
  bool haveChange = false;

  const auto firstNameResult = myapp::GetString(myapp::key::info::firstName, *requestJsonBody);
  if (std::holds_alternative<std::string>(firstNameResult)) {
    newInfo.firstName = std::get<std::string>(firstNameResult);
    haveChange |= true;
  } else {
    const auto error = std::get<myapp::Error>(firstNameResult);
    if (myapp::Error::Code::KeyNotFound != error.GetCode()) {
      errors.push_back(error);
    }
  }

  const auto lastNameResult = myapp::GetString(myapp::key::info::lastName, *requestJsonBody);
  if (std::holds_alternative<std::string>(lastNameResult)) {
    newInfo.lastName = std::get<std::string>(lastNameResult);
    haveChange |= true;
  } else {
    const auto error = std::get<myapp::Error>(lastNameResult);
    if (myapp::Error::Code::KeyNotFound != error.GetCode()) {
      errors.push_back(error);
    }
  }

  if (!haveChange) {
    errors.emplace_back(myapp::Error::Code::KeyNotFound);
  }

  if (!errors.empty()) {
    return errors;
  }

  user.SetInfo(std::move(newInfo));

  return GetUser(user);
}
