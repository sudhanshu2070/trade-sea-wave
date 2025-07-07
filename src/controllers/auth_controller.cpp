#include "auth_controller.hpp"
#include "../services/auth_service.hpp"
#include "../utils/response_helper.hpp"

// Implement the constructor
AuthController::AuthController() : authService() {}

void AuthController::setupRoutes(Pistache::Rest::Router& router) {
    using namespace Pistache::Rest;
    
    Routes::Post(router, "/auth/signup", Routes::bind(&AuthController::signup, this));
    Routes::Post(router, "/auth/login", Routes::bind(&AuthController::login, this));
}

void AuthController::signup(const Pistache::Rest::Request& request, 
                          Pistache::Http::ResponseWriter response) {
    authService.handleSignup(request, response);
}

void AuthController::login(const Pistache::Rest::Request& request,
                         Pistache::Http::ResponseWriter response) {
    authService.handleLogin(request, response);
}