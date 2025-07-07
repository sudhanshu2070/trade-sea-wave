#include "controllers/auth_controller.hpp"
#include <pistache/endpoint.h>
#include <iostream>

int main() {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(9080));
    auto server = std::make_shared<Pistache::Http::Endpoint>(addr);
    
    Pistache::Rest::Router router;
    AuthController authController;
    authController.setupRoutes(router);
    
    auto opts = Pistache::Http::Endpoint::options()
        .threads(2)
        .flags(Pistache::Tcp::Options::ReuseAddr);
    
    server->init(opts);
    server->setHandler(router.handler());
    
    std::cout << "Server running on port 9080\n";
    server->serve();
    
    return 0;
}