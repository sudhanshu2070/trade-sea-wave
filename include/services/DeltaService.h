#pragma once
#include <string>

class DeltaService {
public:
    std::string fetchMarkets();
    std::string fetchOHLCV(const std::string& symbol, const std::string& resolution, long start = 0, long end = 0);

private:
    std::string apiKey_;
    std::string apiSecret_;

    std::string sendGetRequest(const std::string& url, bool auth = false);
};