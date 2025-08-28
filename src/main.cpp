#include <pistache/endpoint.h>
#include <pistache/router.h>
#include "../include/controllers/DeltaController.h"
#include "../include/controllers/BacktestController.h" 
#include "../include/services/BacktestService.h"         
#include "../include/services/DeltaService.h"  

int main() {
    Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(9080));
    auto opts = Pistache::Http::Endpoint::options().threads(1);
    Pistache::Http::Endpoint server(addr);
    server.init(opts);

    Pistache::Rest::Router router;
    DeltaService deltaService;
    BacktestService backtestService(deltaService);

    DeltaController deltaController(router);
    BacktestController backtestController(router, backtestService);

    server.setHandler(router.handler());
    server.serve();

    return 0;
}