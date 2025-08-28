#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include "../services/BacktestService.h"

class BacktestController {
public:
    BacktestController(Pistache::Rest::Router& router, BacktestService& service);

    void setupRoutes();

private:
    Pistache::Rest::Router& router_;
    BacktestService& backtestService_;

    void runBacktest(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);
};