#pragma once
#include <pistache/router.h>
#include <string>
#include <map>
#include <jwt-cpp/jwt.h>

// Forward declare Pistache classes to avoid circular includes
namespace Pistache {
    namespace Rest {
        class Request;
    }
    namespace Http {
        class ResponseWriter;
    }
}

class AuthService {
public:
    void handleSignup(const Pistache::Rest::Request& request, 
                     Pistache::Http::ResponseWriter& response);
    
    void handleLogin(const Pistache::Rest::Request& request,
                    Pistache::Http::ResponseWriter& response);
    
private:
    std::map<std::string, std::string> users_db;  // Simple in-memory storage
    const std::string secret_key = "your_256_bit_secret";
    
    std::string hashPassword(const std::string& password);
    std::string generateSalt();
    std::string createJWT(const std::string& username);
};