#pragma once
#include <pistache/router.h>

class TradingController {
public:
    explicit TradingController(Pistache::Rest::Router& router);
    void getTicker(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
};ī