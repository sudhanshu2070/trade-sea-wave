#include "models/Backtester.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

// Helper functions for Renko & Ichimoku (stub implementations)
std::vector<Candle> buildRenko(const std::vector<Candle>& candles, double boxSize) {
    std::vector<Candle> renko;
    if (candles.empty()) return renko;

    Candle last = candles[0];
    renko.push_back(last);

    for (size_t i = 1; i < candles.size(); ++i) {
        double diff = candles[i].close - last.close;
        int bricks = std::floor(diff / boxSize);
        if (std::abs(bricks) >= 1) {
            for (int b = 0; b < std::abs(bricks); ++b) {
                Candle brick;
                brick.time = candles[i].time;
                brick.open = last.close;
                brick.close = last.close + boxSize * (bricks > 0 ? 1 : -1);
                brick.high = std::max(brick.open, brick.close);
                brick.low = std::min(brick.open, brick.close);
                brick.volume = candles[i].volume;
                renko.push_back(brick);
                last = brick;
            }
        }
    }
    return renko;
}

double dynamicBoxSize(double lastClose) {
    // Example: 0.5% of price as box size
    return lastClose * 0.005;
}

// Simple Ichimoku calculation (stub)
struct IchimokuData {
    std::vector<double> base;
    std::vector<double> lead1;
    std::vector<double> lead2;
};

IchimokuData calculateIchimoku(const std::vector<Candle>& candles) {
    IchimokuData data;
    size_t n = candles.size();
    data.base.resize(n, 0);
    data.lead1.resize(n, 0);
    data.lead2.resize(n, 0);

    for (size_t i = 0; i < n; ++i) {
        double highMax = candles[i].high;
        double lowMin = candles[i].low;
        data.base[i] = (highMax + lowMin) / 2;
        data.lead1[i] = data.base[i] + 1; // stub
        data.lead2[i] = data.base[i] - 1; // stub
    }
    return data;
}

// ---------------------------
// Backtester implementation
// ---------------------------
Backtester::Backtester(double initial_balance) {
    balance = initial_balance;
    entryPrice = 0.0;
    position = 0;
    trades.clear();
}

void Backtester::run(std::vector<Candle>& candles) {
    auto renkoBricks = buildRenko(candles, dynamicBoxSize(candles.back().close));
    auto ichimokuData = calculateIchimoku(candles);

    for (size_t i = 0; i < renkoBricks.size(); ++i) {
        auto renkoClose = renkoBricks[i].close;
        auto base = ichimokuData.base[i];
        auto lead1 = ichimokuData.lead1[i];
        auto lead2 = ichimokuData.lead2[i];
        std::string now = std::to_string(renkoBricks[i].time);

        // Entry
        if (position == 0) {
            if (renkoClose > base && renkoClose > lead1 && renkoClose > lead2) {
                position = 1;
                entryPrice = renkoClose;
                trades.push_back({"LONG", entryPrice, 0, 0, now, ""});
            } else if (renkoClose < base && renkoClose < lead1 && renkoClose < lead2) {
                position = -1;
                entryPrice = renkoClose;
                trades.push_back({"SHORT", entryPrice, 0, 0, now, ""});
            }
        }
        // Exit
        else if (position == 1) {
            if (renkoClose < base || renkoClose < lead1) {
                double pnl = renkoClose - entryPrice;
                balance += pnl;
                trades.back().exit = renkoClose;
                trades.back().pnl = pnl;
                trades.back().exit_time = now;
                position = 0;
            }
        }
        else if (position == -1) {
            if (renkoClose > base || renkoClose > lead2) {
                double pnl = entryPrice - renkoClose;
                balance += pnl;
                trades.back().exit = renkoClose;
                trades.back().pnl = pnl;
                trades.back().exit_time = now;
                position = 0;
            }
        }
    }

    // Liquidate at the end
    if (position != 0) {
        double lastClose = renkoBricks.back().close;
        double pnl = position == 1 ? lastClose - entryPrice : entryPrice - lastClose;
        balance += pnl;
        trades.back().exit = lastClose;
        trades.back().pnl = pnl;
        trades.back().exit_time = std::to_string(renkoBricks.back().time);
        position = 0;
    }
}