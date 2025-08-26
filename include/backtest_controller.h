#pragma once
#include <pistache/router.h>

class BacktestController {
public:
    explicit BacktestController(Pistache::Rest::Router& router);
    void runBacktest(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
};