#pragma once
#include <pistache/router.h>
#include "services/DeltaService.h"

class DeltaController {
public:
    explicit DeltaController(Pistache::Rest::Router& router);

private:
    void setupRoutes();
    
    // Handlers
    void getMarkets(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);
    void getOHLCV(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);
    void placeOrder(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);


    Pistache::Rest::Router& router_;
    DeltaService deltaService_;
};