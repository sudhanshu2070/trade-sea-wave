#pragma once
#include <string>

class DeltaService {
public:
    std::string fetchMarkets();
    std::string fetchOHLCV(const std::string& symbol, const std::string& resolution, long start = 0, long end = 0);
};