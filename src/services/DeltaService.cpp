#include "services/DeltaService.h"
#include <curl/curl.h>
#include <iostream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

std::string DeltaService::fetchMarkets() {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.delta.exchange/v2/products");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return response;
}

std::string DeltaService::fetchOHLCV(const std::string& symbol, const std::string& resolution, long start, long end) {
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        std::string url = "https://api.india.delta.exchange/v2/history/candles?symbol=" + symbol +
                          "&resolution=" + resolution;

        if (start > 0) url += "&start=" + std::to_string(start);
        if (end > 0)   url += "&end=" + std::to_string(end);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return response;
}