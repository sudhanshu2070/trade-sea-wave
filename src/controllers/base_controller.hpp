#pragma once
#include <pistache/router.h>

class BaseController {
protected:
    virtual void setupRoutes(Pistache::Rest::Router& router) = 0;
    static void handleOptions(const Pistache::Rest::Request&, 
                            Pistache::Http::ResponseWriter response) {
        response.send(Pistache::Http::Code::Ok);
    }
};