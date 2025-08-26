#include "backtest_controller.h"

BacktestController::BacktestController(Pistache::Rest::Router& router) {
    using namespace Pistache::Rest;
    Routes::Post(router, "/api/backtest/run", Routes::bind(&BacktestController::runBacktest, this));
}

void BacktestController::runBacktest(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter response) {
    // Here you would run your backtest logic
    response.send(Pistache::Http::Code::Ok, R"({"result":"Backtest completed","profit":1234.56})", MIME(Application, Json));
}