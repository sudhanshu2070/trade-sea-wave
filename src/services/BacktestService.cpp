#include "services/BacktestService.h"
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

    if (candles.size() < 50) {
        std::cerr << "Not enough candles for backtest (" 
                  << candles.size() << " received)." << std::endl;
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

// ----------------------
// Renko + Ichimoku Logic
// ----------------------
void BacktestService::runRenkoIchimoku(std::vector<Candle>& candles, 
                                       BacktestResult& result,
                                       const std::string& symbol) {
    Backtester bt(10000.0);
    bt.run(candles);

    std::vector<std::string> actions(candles.size(), "HOLD");
    std::vector<double> cashHistory(candles.size(), 10000.0);
    std::vector<double> posHistory(candles.size(), 0.0);
    std::vector<double> pnlHistory(candles.size(), 0.0);

    for (auto& t : bt.trades) {
        for (size_t i = 0; i < candles.size(); i++) {
            if (std::to_string(candles[i].time) == t.entry_time) actions[i] = t.type;
            if (std::to_string(candles[i].time) == t.exit_time)  actions[i] = (t.type == "LONG") ? "SELL" : "BUY";
        }
    }

    // Fill cash/position/pnl history (simplified)
    double cash = 10000.0;
    double position = 0.0;
    for (size_t i = 0; i < candles.size(); i++) {
        double price = candles[i].close;
        cashHistory[i] = cash;
        posHistory[i] = position;
        pnlHistory[i] = cash + position * price - 10000.0;
    }

    result.pnl = bt.balance - 10000.0;
    result.trades = bt.trades.size();

    exportTradesToCSV(candles, actions, cashHistory, posHistory, pnlHistory, symbol, "backtest_results.csv");
}

// ----------------------
// CSV Export with cash, position, PnL
// ----------------------
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