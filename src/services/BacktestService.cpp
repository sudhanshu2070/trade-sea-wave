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

    // Fetch OHLCV data (1m resolution for now)
    std::string raw = deltaService_.fetchOHLCV(symbol, "1m", start, end);
    if (raw.empty()) {
        std::cerr << "No OHLCV data received!" << std::endl;
        return result;
    }

    // Parse JSON response
    json j = json::parse(raw);
    std::vector<Candle> candles;

    if (j.contains("result")) {
        for (auto& c : j["result"]) {
            Candle candle;
            candle.open = c["open"];
            candle.high = c["high"];
            candle.low = c["low"];
            candle.close = c["close"];
            candle.volume = c["volume"];
            candle.time = c["time"];
            candles.push_back(candle);
        }
    }

    if (candles.size() < 50) {
        std::cerr << "Not enough candles for backtest." << std::endl;
        return result;
    }

    // ===========================
    // Example Strategy: SMA(5) vs SMA(20)
    // ===========================
    int shortWin = 5, longWin = 20;
    double cash = 10000;   // start with 10k
    double position = 0;   // BTC position
    double lastPrice = 0;

    for (size_t i = longWin; i < candles.size(); i++) {
        // Compute SMA
        double shortSum = 0, longSum = 0;
        for (int s = 0; s < shortWin; s++) shortSum += candles[i - s].close;
        for (int l = 0; l < longWin; l++) longSum += candles[i - l].close;

        double shortMA = shortSum / shortWin;
        double longMA  = longSum / longWin;

        double price = candles[i].close;

        // Buy signal: shortMA crosses above longMA
        if (shortMA > longMA && position == 0) {
            position = cash / price; // buy BTC
            cash = 0;
            result.trades++;
        }
        // Sell signal: shortMA crosses below longMA
        else if (shortMA < longMA && position > 0) {
            cash = position * price; // sell BTC
            position = 0;
            result.trades++;
        }

        lastPrice = price;
    }

    // Final PnL: if holding BTC, liquidate
    if (position > 0) {
        cash = position * lastPrice;
        position = 0;
    }

    result.pnl = cash - 10000; // net profit

    return result;
}