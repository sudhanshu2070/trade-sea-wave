#include "services/BacktestService.h"
#include "models/RenkoBuilder.h"
#include "models/IchimokuCalculator.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>

using json = nlohmann::json;

// ----------------------
// Helper Enums
// ----------------------
enum class StrategyType {
    SMA_CROSSOVER,
    RENKO_ICHIMOKU
};

StrategyType parseStrategy(const std::string& strategy) {
    if (strategy == "sma_crossover") return StrategyType::SMA_CROSSOVER;
    if (strategy == "renko_ichimoku") return StrategyType::RENKO_ICHIMOKU;
    return StrategyType::SMA_CROSSOVER; // default
}

// ----------------------
// Constructor
// ----------------------
BacktestService::BacktestService(DeltaService& deltaService)
    : deltaService_(deltaService) {}

// ----------------------
// Main run function
// ----------------------
BacktestResult BacktestService::run(const std::string& strategy,
                                    const std::string& symbol,
                                    long start,
                                    long end) {
    BacktestResult result;
    result.pnl = 0.0;
    result.trades = 0;

    // Fetch OHLCV
    std::string raw = deltaService_.fetchOHLCV(symbol, "1m", start, end);
    if (raw.empty()) {
        std::cerr << "No OHLCV data received!" << std::endl;
        return result;
    }

    json j = json::parse(raw);
    std::vector<Candle> candles;
    if (j.contains("result")) {
        for (auto& c : j["result"]) {
            Candle candle;
            candle.open   = c["open"];
            candle.high   = c["high"];
            candle.low    = c["low"];
            candle.close  = c["close"];
            candle.volume = c["volume"];
            candle.time   = c["time"];
            candles.push_back(candle);
        }
    }

    if (candles.size() < 52) {
        std::cerr << "Not enough candles for backtest (" 
                  << candles.size() << " received). Need at least 52." << std::endl;
        return result;
    }

    StrategyType strat = parseStrategy(strategy);

    if (strat == StrategyType::SMA_CROSSOVER) {
        runSMA(candles, result, symbol);
    } else if (strat == StrategyType::RENKO_ICHIMOKU) {
        runRenkoIchimoku(candles, result, symbol);
    }

    return result;
}

// ----------------------
// SMA Crossover Logic
// ----------------------
void BacktestService::runSMA(const std::vector<Candle>& candles, 
                             BacktestResult& result,
                             const std::string& symbol) {
    int shortWin = 5, longWin = 20;
    double cash = 10000.0;
    double position = 0.0;
    double entryPrice = 0.0;

    bool prevBull = false;
    std::vector<std::string> actions(candles.size(), "HOLD");

    // Store cash, position, pnl per candle
    std::vector<double> cashHistory(candles.size(), cash);
    std::vector<double> posHistory(candles.size(), position);
    std::vector<double> pnlHistory(candles.size(), 0.0);

    for (size_t i = longWin; i < candles.size(); i++) {
        double shortSum = 0, longSum = 0;
        for (int s = 0; s < shortWin; s++) shortSum += candles[i - s].close;
        for (int l = 0; l < longWin; l++) longSum += candles[i - l].close;

        double shortMA = shortSum / shortWin;
        double longMA  = longSum / longWin;
        double price   = candles[i].close;
        bool bull = shortMA > longMA;

        if (!prevBull && bull && position == 0) {
            position = cash / price; entryPrice = price; cash = 0; result.trades++; actions[i] = "BUY";
        } else if (prevBull && !bull && position > 0) {
            cash = position * price; 
            double tradePnL = (price - entryPrice) * position;
            result.pnl += tradePnL;
            position = 0; result.trades++; actions[i] = "SELL";
        }

        prevBull = bull;

        // Update histories
        cashHistory[i] = cash;
        posHistory[i] = position;
        double currentValue = cash + position * price;
        pnlHistory[i] = currentValue - 10000.0;
    }

    if (position > 0) {
        double lastPrice = candles.back().close;
        cash = position * lastPrice;
        result.pnl += (lastPrice - entryPrice) * position;
        position = 0;
        actions.back() = "SELL";
        cashHistory.back() = cash;
        posHistory.back() = position;
        pnlHistory.back() = cash - 10000.0;
    }

    exportTradesToCSV(candles, actions, cashHistory, posHistory, pnlHistory, symbol, "backtest_results.csv");
}

void BacktestService::runRenkoIchimoku(std::vector<Candle>& candles, 
                                       BacktestResult& result,
                                       const std::string& symbol) {
    // Build Renko bricks using improved builder
    RenkoBuilder renkoBuilder(40.0);
    std::vector<RenkoBrick> renkoBricks;
    
    for (const auto& candle : candles) {
        auto brick = renkoBuilder.feed(candle.close, std::to_string(candle.time));
        if (brick) {
            renkoBricks.push_back(*brick);
            // Debug output for Renko bricks
            std::cout << "RENKO BRICK: " << brick->ts 
                      << " " << brick->dir()
                      << " " << brick->open 
                      << " -> " << brick->close 
                      << std::endl;
        }
    }
    
    // Compute Ichimoku
    auto ichimokuData = IchimokuCalculator::compute_ichimoku_from_candles(candles);
    
    // Trading logic
    double cash = 10000.0;
    double positionShares = 0.0;
    double entryPrice = 0.0;
    bool inPosition = false;
    
    std::vector<std::string> actions(candles.size(), "HOLD");
    std::vector<double> cashHistory(candles.size(), cash);
    std::vector<double> posHistory(candles.size(), positionShares);
    std::vector<double> pnlHistory(candles.size(), 0.0);
    
    // Process Renko bricks sequentially
    size_t renkoIndex = 0;
    
    for (size_t i = 0; i < candles.size(); i++) {
        double currentPrice = candles[i].close;
        std::string currentTime = std::to_string(candles[i].time);
        
        // Skip if Ichimoku data not available
        if (i >= ichimokuData.base.size() || 
            std::isnan(ichimokuData.base[i]) || 
            std::isnan(ichimokuData.lead1_f[i]) || 
            std::isnan(ichimokuData.lead2_f[i])) {
            continue;
        }
        
        double base = ichimokuData.base[i];
        double lead1 = ichimokuData.lead1_f[i];
        double lead2 = ichimokuData.lead2_f[i];
        
        // Check for Renko signal at this candle time
        bool hasRenkoSignal = false;
        Trend renkoTrend = Trend::NONE;
        double renkoClose = 0.0;
        
        if (renkoIndex < renkoBricks.size()) {
            if (renkoBricks[renkoIndex].ts == currentTime) {
                hasRenkoSignal = true;
                renkoTrend = renkoBricks[renkoIndex].trend;
                renkoClose = renkoBricks[renkoIndex].close;
                renkoIndex++;
            }
        }
        
        if (!hasRenkoSignal) {
            continue;
        }
        
        // ENTRY LOGIC - Use Renko trend direction
        if (!inPosition) {
            // LONG ENTRY: Renko UP trend and price above Ichimoku components
            if (renkoTrend == Trend::UP && currentPrice > base && currentPrice > lead1 && currentPrice > lead2) {
                positionShares = cash / currentPrice;
                entryPrice = currentPrice;
                cash = 0;
                inPosition = true;
                result.trades++;
                actions[i] = "LONG";
                std::cout << "LONG ENTRY: " << currentPrice << " at " << currentTime << std::endl;
            }
            // SHORT ENTRY: Renko DOWN trend and price below Ichimoku components  
            else if (renkoTrend == Trend::DOWN && currentPrice < base && currentPrice < lead1 && currentPrice < lead2) {
                positionShares = -cash / currentPrice;
                entryPrice = currentPrice;
                cash = 0;
                inPosition = true;
                result.trades++;
                actions[i] = "SHORT";
                std::cout << "SHORT ENTRY: " << currentPrice << " at " << currentTime << std::endl;
            }
        }
        // EXIT LOGIC
        else if (inPosition) {
            bool shouldExit = false;
            
            // LONG EXIT: Price falls below base or lead1, OR Renko trend reverses
            if (positionShares > 0 && 
                (currentPrice < base || currentPrice < lead1 || renkoTrend == Trend::DOWN)) {
                cash = positionShares * currentPrice;
                shouldExit = true;
                actions[i] = "SELL";
                std::cout << "LONG EXIT: " << currentPrice << " at " << currentTime << std::endl;
            }
            // SHORT EXIT: Price rises above base or lead2, OR Renko trend reverses
            else if (positionShares < 0 && 
                     (currentPrice > base || currentPrice > lead2 || renkoTrend == Trend::UP)) {
                cash = -positionShares * currentPrice;
                shouldExit = true;
                actions[i] = "COVER";
                std::cout << "SHORT EXIT: " << currentPrice << " at " << currentTime << std::endl;
            }
            
            if (shouldExit) {
                double tradePnL = cash - 10000.0;
                result.pnl += tradePnL;
                positionShares = 0;
                inPosition = false;
            }
        }
        
        // Update portfolio values
        double positionValue = positionShares * currentPrice;
        double totalValue = cash + positionValue;
        
        cashHistory[i] = cash;
        posHistory[i] = positionShares;
        pnlHistory[i] = totalValue - 10000.0;
    }
    
    // Liquidate any remaining position at the end
    if (inPosition) {
        double lastPrice = candles.back().close;
        if (positionShares > 0) {
            cash = positionShares * lastPrice;
            actions.back() = "SELL";
        } else {
            cash = -positionShares * lastPrice;
            actions.back() = "COVER";
        }
        double tradePnL = cash - 10000.0;
        result.pnl += tradePnL;
        
        cashHistory.back() = cash;
        posHistory.back() = 0;
        pnlHistory.back() = cash - 10000.0;
    }
    
    // Enhanced CSV export with Renko data
    exportTradesToCSV(candles, actions, cashHistory, posHistory, pnlHistory, renkoBricks, ichimokuData, symbol, "backtest_detailed.csv");
}

// ----------------------------------------------
// CSV Export (Only trade actions)
// ----------------------------------------------
void BacktestService::exportTradesToCSV(const std::vector<Candle>& candles,
                                        const std::vector<std::string>& actions,
                                        const std::vector<double>& cashHistory,
                                        const std::vector<double>& posHistory,
                                        const std::vector<double>& pnlHistory,
                                        const std::string& symbol,
                                        const std::string& filename) 
{
    std::ofstream file(filename);
    file << "Time(IST),Open,High,Low,Close,Volume,Action,Cash,Position,PnL\n";

    for (size_t i = 0; i < candles.size() && i < actions.size(); i++) {
        if (actions[i] == "HOLD") {
            continue;
        }

        std::time_t t = candles[i].time + 5*3600 + 30*60;
        std::tm* istTime = std::gmtime(&t);

        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", istTime);

        file << buf << ","
             << candles[i].open << ","
             << candles[i].high << ","
             << candles[i].low << ","
             << candles[i].close << ","
             << candles[i].volume << ","
             << actions[i] << ","
             << cashHistory[i] << ","
             << posHistory[i] << ","
             << pnlHistory[i] << "\n";
    }

    file.close();
    std::cout << "Backtest trades exported to " << filename << std::endl;
}