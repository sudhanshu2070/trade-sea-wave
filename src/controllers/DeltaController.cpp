#include "controllers/DeltaController.h"
#include <nlohmann/json.hpp>  // For JSON parsing

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

struct Candle {
    double open;
    double high;
    double low;
    double close;
    long time;
    double volume;
};

std::vector<Candle> parseCandles(const std::string& jsonStr) {
    std::vector<Candle> candles;
    auto j = nlohmann::json::parse(jsonStr);

    for (auto& item : j["result"]) {
        Candle c;
        c.open = item["open"];
        c.high = item["high"];
        c.low = item["low"];
        c.close = item["close"];
        c.time = item["time"];
        c.volume = item["volume"];
        candles.push_back(c);
    }

    return candles;
}

void DeltaController::getOHLCV(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res) {
    long start = 0, end = 0;

    auto startParam = req.query().get("start");
    if (startParam) start = std::stol(startParam.value());

    auto endParam = req.query().get("end");
    if (endParam) end = std::stol(endParam.value());

    std::string symbol = req.param(":symbol").as<std::string>();
    std::string resolution = req.param(":resolution").as<std::string>();

    // Fetch raw OHLCV data from DeltaService
    std::string rawData = deltaService_.fetchOHLCV(symbol, resolution, start, end);

    // Parse OHLCV JSON into structured data
    auto candles = parseCandles(rawData);

    // Example: convert back to JSON to send structured response
    nlohmann::json responseJson;
    responseJson["candles"] = nlohmann::json::array();
    for (auto& c : candles) {
        responseJson["candles"].push_back({
            {"open", c.open},
            {"high", c.high},
            {"low", c.low},
            {"close", c.close},
            {"time", c.time},
            {"volume", c.volume}
        });
    }

    res.send(Pistache::Http::Code::Ok, responseJson.dump());
}