#pragma once
#include <string>

class DeltaService {
public:
    DeltaService();

    std::string fetchMarkets();
    std::string fetchOHLCV(const std::string& symbol, const std::string& resolution, long start = 0, long end = 0);

    std::string sendPostRequest(const std::string& endpoint, const std::string& body, bool auth);
    std::string placeOrder(int productId, const std::string& side,
                           const std::string& orderType,
                           const std::string& price, int size);

private:
    std::string apiKey_;
    std::string apiSecret_;
    std::string baseUrl_;

    std::string sendGetRequest(const std::string& endpoint, bool auth = false);
};