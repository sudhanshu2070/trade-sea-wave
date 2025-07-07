#include "auth_service.hpp"
#include "../utils/response_helper.hpp"  // Including the new helper

void AuthService::handleSignup(const Pistache::Rest::Request& request, 
                             Pistache::Http::ResponseWriter& response) {
    try {
        auto json = nlohmann::json::parse(request.body());
        std::string username = json["username"];
        std::string password = json["password"];

        if (username.empty() || password.empty()) {
            ResponseHelper::badRequest(response, "Username and password are required.");
            return;
        }

        if (users_db.find(username) != users_db.end()) {
            ResponseHelper::badRequest(response, "Username already exists");
            return;
        }

        users_db[username] = password; // Note: In production, store hashed passwords
        ResponseHelper::ok(response, "User created successfully");
    } catch (...) {
        ResponseHelper::badRequest(response, "Invalid request format");
    }
}

void AuthService::handleLogin(const Pistache::Rest::Request& request,
                            Pistache::Http::ResponseWriter& response) {
    try {
        auto json = nlohmann::json::parse(request.body());
        std::string username = json["username"];
        std::string password = json["password"];

        if (username.empty() || password.empty()) {
            ResponseHelper::badRequest(response, "Username and password are required.");
            return;
        }

        auto it = users_db.find(username);
        if (it == users_db.end() || it->second != password) {
            ResponseHelper::unauthorized(response, "Invalid credentials");
            return;
        }

        ResponseHelper::ok(response, "Login successful");
    } catch (...) {
        ResponseHelper::badRequest(response, "Invalid request format");
    }
}