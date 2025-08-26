#include "DeltaController.h"

DeltaController::DeltaController(Pistache::Rest::Router& router) : router_(router) {
    setupRoutes();
}

void DeltaController::setupRoutes() {
    using namespace Pistache::Rest;
    Routes::Get(router_, "/delta/markets", Routes::bind(&DeltaController::getMarkets, this));
}

void DeltaController::getMarkets(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
    std::string data = deltaService_.fetchMarkets();
    response.send(Pistache::Http::Code::Ok, data);
}