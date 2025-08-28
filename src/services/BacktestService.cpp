#include "services/BacktestService.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>

using json = nlohmann::json;

struct Candle {
    double open, high, low, close, volume;
    long time;
};

BacktestService::BacktestService(DeltaService& deltaService)
    : deltaService_(deltaService) {}

BacktestResult BacktestService::run(const std::string& strategy,
                                    const std::string& symbol,
                                    long start,
                                    long end) {
    BacktestResult result;
    result.pnl = 0.0;
    result.trades = 0;

    // Fetch OHLCV (1m resolution for now)
    std::string raw = deltaService_.fetchOHLCV(symbol, "1m", start, end);
    if (raw.empty()) {
        std::cerr << "No OHLCV data received!" << std::endl;
        return result;
    }

    std::cout << "[DEBUG] Raw OHLCV response: " << raw.substr(0, 300) << "...\n";

    // Parse JSON
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

    // ===========================
    // Strategy: SMA(5) vs SMA(20)
    // ===========================
    int shortWin = 5, longWin = 20;
    double cash = 10000.0;  // start with 10k USDT
    double position = 0.0;  // BTC position
    double entryPrice = 0.0;

    bool prevBull = false;  // track crossover direction

    for (size_t i = longWin; i < candles.size(); i++) {
        // Compute SMA
        double shortSum = 0, longSum = 0;
        for (int s = 0; s < shortWin; s++) shortSum += candles[i - s].close;
        for (int l = 0; l < longWin; l++) longSum += candles[i - l].close;

        double shortMA = shortSum / shortWin;
        double longMA  = longSum / longWin;
        double price   = candles[i].close;

        bool bull = shortMA > longMA;

        // Detect crossover: bear → bull
        if (!prevBull && bull && position == 0) {
            position = cash / price; // buy BTC
            entryPrice = price;
            cash = 0;
            result.trades++;
            std::cout << "[BUY] at " << price << " (time=" << candles[i].time << ")\n";
        }
        // Detect crossover: bull → bear
        else if (prevBull && !bull && position > 0) {
            cash = position * price; // sell BTC
            result.pnl += (price - entryPrice) * position; // PnL of this trade
            position = 0;
            result.trades++;
            std::cout << "[SELL] at " << price << " (time=" << candles[i].time << ")\n";
        }

        prevBull = bull;
    }

    // Liquidate at last candle
    if (position > 0) {
        double lastPrice = candles.back().close;
        cash = position * lastPrice;
        result.pnl += (lastPrice - entryPrice) * position;
        position = 0;
    }

    return result;
}