#include "controllers/DeltaController.h"

DeltaController::DeltaController(Pistache::Rest::Router& router) : router_(router) {
    setupRoutes();
}

void DeltaController::setupRoutes() {
    using namespace Pistache::Rest;
    Routes::Get(router_, "/delta/markets", Routes::bind(&DeltaController::getMarkets, this));
    Routes::Get(router_, "/delta/ohlcv/:symbol/:resolution", Routes::bind(&DeltaController::getOHLCV, this));
}

void DeltaController::getMarkets(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
    std::string data = deltaService_.fetchMarkets();
    response.send(Pistache::Http::Code::Ok, data);
}

void DeltaController::getOHLCV(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res) {
    long start = 0, end = 0;

    auto startParam = req.query().get("start");
    if (startParam) start = std::stol(startParam.value());

    auto endParam = req.query().get("end");
    if (endParam) end = std::stol(endParam.value());

    // Get symbol and resolution from path parameters
    std::string symbol = req.param(":symbol").as<std::string>();
    std::string resolution = req.param(":resolution").as<std::string>();

    // Fetch OHLCV data from DeltaService
    std::string data = deltaService_.fetchOHLCV(symbol, resolution, start, end);

    // Send response to client
    res.send(Pistache::Http::Code::Ok, data);
}