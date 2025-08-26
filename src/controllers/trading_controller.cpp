#include "trading_controller.h"

TradingController::TradingController(Pistache::Rest::Router& router) {
    using namespace Pistache::Rest;
    Routes::Get(router, "/api/trading/ticker", Routes::bind(&TradingController::getTicker, this));
}

void TradingController::getTicker(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
    // Here you would call CCXT C++ wrapper or REST API to get ticker data
    response.send(Pistache::Http::Code::Ok, R"({"symbol":"BTC/USDT","price":50000})", MIME(Application, Json));
}