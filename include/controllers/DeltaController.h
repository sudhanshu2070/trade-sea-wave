#pragma once
#include <pistache/router.h>
#include "services/DeltaService.h"

class DeltaController {
public:
    explicit DeltaController(Pistache::Rest::Router& router);

private:
    void setupRoutes();

    // Existing
    void getMarkets(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response);

    // Add this declaration
    void getOHLCV(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter response);

    Pistache::Rest::Router& router_;
    DeltaService deltaService_;
};