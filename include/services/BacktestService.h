#pragma once
#include <string>
#include <vector> 
#include "../models/BacktestResult.h"
#include "DeltaService.h"

struct Candle {
    double open, high, low, close, volume;
    long time;
};

class BacktestService {
public:
    BacktestService(DeltaService& deltaService);

    BacktestResult run(const std::string& strategy,
                       const std::string& symbol,
                       long start,
                       long end);

private:
    DeltaService& deltaService_;

    void exportTradesToCSV(const std::vector<Candle>& candles,
                           const std::vector<std::string>& actions,
                           const std::string& symbol,
                           const std::string& filename);
};