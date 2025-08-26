#pragma once
#include <pistache/router.h>
#include "../services/DeltaService.h"

class DeltaController {
public:
    explicit DeltaController(Pistache::Rest::Router& router);

private:
    void setupRoutes();
    void getMarkets(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    Pistache::Rest::Router& router_;
    DeltaService deltaService_;
};
