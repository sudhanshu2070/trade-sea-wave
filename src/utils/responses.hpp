#pragma once
#include <pistache/http.h>
#include <nlohmann/json.hpp>

class ResponseUtils {
public:
    static void sendJson(Pistache::Http::ResponseWriter& response,
                       Pistache::Http::Code code,
                       const nlohmann::json& json) {
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(code, json.dump());
    }
    
    static void sendError(Pistache::Http::ResponseWriter& response,
                        Pistache::Http::Code code,
                        const std::string& message) {
        nlohmann::json error = {
            {"error", message}
        };
        sendJson(response, code, error);
    }
};