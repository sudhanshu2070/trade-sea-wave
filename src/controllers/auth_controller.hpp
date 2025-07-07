#pragma once
#include "base_controller.hpp"
#include "../services/auth_service.hpp"  

class AuthController : public BaseController {
public:
    AuthController();
    void setupRoutes(Pistache::Rest::Router& router) override;

private:
    AuthService authService;  
    
    void signup(const Pistache::Rest::Request& request, 
               Pistache::Http::ResponseWriter response);
               
    void login(const Pistache::Rest::Request& request,
              Pistache::Http::ResponseWriter response);
};