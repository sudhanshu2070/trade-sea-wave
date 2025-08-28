#include "services/DeltaService.h"
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <iomanip>
#include <cstdlib>   // getenv
#include <sstream>
#include <chrono>
#include <iostream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Utility: HMAC SHA256
std::string hmacSha256(const std::string& key, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha256(),
                  key.c_str(), key.size(),
                  (unsigned char*)data.c_str(), data.size(),
                  NULL, NULL);

    std::ostringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

// Utility: get current timestamp (ms)
std::string currentTimestampMs() {
    using namespace std::chrono;
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return std::to_string(now);
}

DeltaService::DeltaService() {
    // Load API key/secret from environment (optional for now)
    apiKey_   = std::getenv("DELTA_API_KEY")   ? std::getenv("DELTA_API_KEY")   : "";
    apiSecret_= std::getenv("DELTA_API_SECRET")? std::getenv("DELTA_API_SECRET"): "";

    // Set India endpoint
    baseUrl_ = "https://api.india.delta.exchange/v2";
}

std::string DeltaService::sendGetRequest(const std::string& endpoint, bool auth) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = baseUrl_ + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Add API Key for private endpoints (not needed for markets/ohlcv)
        struct curl_slist* headers = NULL;
        if (auth && !apiKey_.empty()) {
            headers = curl_slist_append(headers, ("X-Delta-API-Key: " + apiKey_).c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return response;
}

std::string DeltaService::fetchMarkets() {
    return sendGetRequest("/products", false);
}

std::string DeltaService::fetchOHLCV(const std::string& symbol, const std::string& resolution, long start, long end) {
    std::stringstream endpoint;
    endpoint << "/history/candles?symbol=" << symbol
             << "&resolution=" << resolution;
    if (start > 0) endpoint << "&start=" << start;
    if (end > 0)   endpoint << "&end=" << end;

    return sendGetRequest(endpoint.str(), false);
}


// POST request with authentication
std::string DeltaService::sendPostRequest(const std::string& endpoint, const std::string& body, bool auth) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = baseUrl_ + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

        if (auth) {
            std::string timestamp = currentTimestampMs();
            std::string payload = timestamp + "POST" + endpoint + body;
            std::string sig = hmacSha256(apiSecret_, payload);

            headers = curl_slist_append(headers, ("api-key: " + apiKey_).c_str());
            headers = curl_slist_append(headers, ("timestamp: " + timestamp).c_str());
            headers = curl_slist_append(headers, ("signature: " + sig).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return response;
}

// High-level order creation
std::string DeltaService::placeOrder(int productId, const std::string& side,
                                     const std::string& orderType,
                                     const std::string& price,
                                     int size) {
    std::ostringstream body;
    body << "{"
         << "\"product_id\": " << productId << ","
         << "\"limit_price\": \"" << price << "\","
         << "\"size\": " << size << ","
         << "\"side\": \"" << side << "\","
         << "\"order_type\": \"" << orderType << "\""
         << "}";

    return sendPostRequest("/orders", body.str(), true);
}