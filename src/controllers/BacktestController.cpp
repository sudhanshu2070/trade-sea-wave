#include "controllers/BacktestController.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

BacktestController::BacktestController(Pistache::Rest::Router& router, BacktestService& service)
    : router_(router), backtestService_(service) {
    setupRoutes();
}

void BacktestController::setupRoutes() {
    using namespace Pistache::Rest;
    Routes::Post(router_, "/backtest/run", Routes::bind(&BacktestController::runBacktest, this));
}

void BacktestController::runBacktest(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res) {
    try {
        auto bodyJson = json::parse(req.body());

        std::string strategy = bodyJson["strategy"];
        std::string symbol   = bodyJson["symbol"];
        long start           = bodyJson["start"];
        long end             = bodyJson["end"];

        auto result = backtestService_.run(strategy, symbol, start, end);

        json responseJson = {
            {"success", true},
            {"strategy", strategy},
            {"symbol", symbol},
            {"start", start},
            {"end", end},
            {"pnl", result.pnl},
            {"trades", result.trades}
        };

        res.send(Pistache::Http::Code::Ok, responseJson.dump());

    } catch (const std::exception& e) {
        json error;
        error["success"] = false;
        error["error"] = e.what();
        res.send(Pistache::Http::Code::Bad_Request, error.dump());
    }
}