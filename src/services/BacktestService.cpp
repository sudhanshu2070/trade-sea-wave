    #include "services/BacktestService.h"
    #include <nlohmann/json.hpp>
    #include <vector>
    #include <iostream>
    #include <fstream>
    #include <iomanip>
    #include <ctime>

    using json = nlohmann::json;
    

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
        std::vector<std::string> actions(candles.size(), "HOLD"); // For CSV export

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
                actions[i] = "BUY"; 
                std::cout << "[BUY] at " << price << " (time=" << candles[i].time << ")\n";
            }
            // Detect crossover: bull → bear
            else if (prevBull && !bull && position > 0) {
                cash = position * price; // sell BTC
                result.pnl += (price - entryPrice) * position; // PnL of this trade
                position = 0;
                result.trades++;
                actions[i] = "SELL"; 
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
            actions.back() = "SELL";
        }
        
        // Finally, export CSV
        exportTradesToCSV(candles, actions, symbol, "backtest_results.csv");
        
        return result;
    }

    void BacktestService::exportTradesToCSV(const std::vector<Candle>& candles,
                                            const std::vector<std::string>& actions,
                                            const std::string& symbol,
                                            const std::string& filename) 
    {
        std::ofstream file(filename);
        file << "Time(IST),Open,High,Low,Close,Volume,Action\n";

        for (size_t i = 0; i < candles.size() && i < actions.size(); i++) {
            // Convert UTC timestamp to IST
            std::time_t t = candles[i].time;
            t += 5 * 3600 + 30 * 60; // IST offset
            std::tm* istTime = std::gmtime(&t);

            char buf[32];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", istTime);

            file << buf << ","
                << candles[i].open << ","
                << candles[i].high << ","
                << candles[i].low << ","
                << candles[i].close << ","
                << candles[i].volume << ","
                << actions[i] << "\n";
        }

        file.close();
        std::cout << "Backtest trades exported to " << filename << std::endl;
    }