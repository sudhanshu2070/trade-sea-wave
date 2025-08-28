#pragma once
#include <string>
#include <vector> 
#include "../models/BacktestResult.h"
#include "../models/Backtester.h"
#include "DeltaService.h"

class BacktestService {
private:
    DeltaService& deltaService_;

    // New helper functions
    void runSMA(const std::vector<Candle>& candles, BacktestResult& result, const std::string& symbol);
    void runRenkoIchimoku(std::vector<Candle>& candles, BacktestResult& result, const std::string& symbol);

public:
    BacktestService(DeltaService& deltaService);

    BacktestResult run(const std::string& strategy,
                       const std::string& symbol,
                       long start,
                       long end);

    // Updated CSV export
    void exportTradesToCSV(const std::vector<Candle>& candles,
                           const std::vector<std::string>& actions,
                           const std::vector<double>& cashHistory,
                           const std::vector<double>& posHistory,
                           const std::vector<double>& pnlHistory,
                           const std::string& symbol,
                           const std::string& filename);
};