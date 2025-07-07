#pragma once
#include <pistache/http.h>
#include <nlohmann/json.hpp>

class ResponseHelper {
public:
    static void sendJsonResponse(Pistache::Http::ResponseWriter& response,
                               Pistache::Http::Code statusCode,
                               const nlohmann::json& jsonData) {
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(statusCode, jsonData.dump());
    }

    static void ok(Pistache::Http::ResponseWriter& response, const std::string& message) {
        sendJsonResponse(response, Pistache::Http::Code::Ok, 
                        {{"status", "success"}, {"message", message}});
    }

    static void badRequest(Pistache::Http::ResponseWriter& response, const std::string& error) {
        sendJsonResponse(response, Pistache::Http::Code::Bad_Request,
                        {{"error", error}});
    }

    static void unauthorized(Pistache::Http::ResponseWriter& response, const std::string& error) {
        sendJsonResponse(response, Pistache::Http::Code::Unauthorized,
                        {{"error", error}});
    }
};