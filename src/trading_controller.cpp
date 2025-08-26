#include "trading_controller.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp> // For JSON parsing (https://github.com/nlohmann/json)

TradingController::TradingController(Pistache::Rest::Router& router) {
    using namespace Pistache::Rest;
    Routes::Get(router, "/api/trading/ticker", Routes::bind(&TradingController::getTicker, this));
}

void TradingController::getTicker(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    // Binance API endpoint for ticker price
    std::string symbol = "BTCUSDT";
    std::string url = "https://api.binance.com/api/v3/ticker/price?symbol=" + symbol;

    auto r = cpr::Get(cpr::Url{url});
    if (r.status_code == 200) {
        // Optionally parse and reformat the JSON
        auto json = nlohmann::json::parse(r.text);
        response.send(Pistache::Http::Code::Ok, json.dump(), MIME(Application, Json));
    } else {
        response.send(Pistache::Http::Code::Bad_Request, R"({"error":"Failed to fetch data from Binance"})", MIME(Application, Json));
    }
}