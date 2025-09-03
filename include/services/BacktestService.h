#pragma once
#include <vector>
#include <string>
#include "models/Candle.h"
#include "models/Trade.h"
#include "models/RenkoBrick.h"
#include "models/IchimokuCalculator.h"
#include "services/DeltaService.h"

struct BacktestResult {
    double pnl;
    int trades;
};

class BacktestService {
public:
    BacktestService(DeltaService& deltaService);
    BacktestResult run(const std::string& strategy, const std::string& symbol, long start, long end);
    
    // Basic export for SMA
    void exportTradesToCSV(const std::vector<Candle>& candles,
                          const std::vector<std::string>& actions,
                          const std::vector<double>& cashHistory,
                          const std::vector<double>& posHistory,
                          const std::vector<double>& pnlHistory,
                          const std::string& symbol,
                          const std::string& filename);

    // Enhanced export for Renko+Ichimoku
    void exportEnhancedTradesToCSV(const std::vector<Candle>& candles,
                                  const std::vector<std::string>& actions,
                                  const std::vector<double>& cashHistory,
                                  const std::vector<double>& posHistory,
                                  const std::vector<double>& pnlHistory,
                                  const std::vector<RenkoBrick>& renkoBricks,
                                  const IchimokuSeries& ichimokuData,
                                  const std::string& symbol,
                                  const std::string& filename);

private:
    void runSMA(const std::vector<Candle>& candles, BacktestResult& result, const std::string& symbol);
    void runRenkoIchimoku(std::vector<Candle>& candles, BacktestResult& result, const std::string& symbol);
    
    DeltaService& deltaService_;
};