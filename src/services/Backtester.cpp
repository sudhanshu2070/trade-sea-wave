#include "services/Backtester.h"
#include "models/RenkoBuilder.h"
#include "models/IchimokuCalculator.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

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
    // Build Renko bricks
    RenkoBuilder renkoBuilder(40.0);
    std::vector<RenkoBrick> renkoBricks;
    
    for (const auto& candle : candles) {
        auto brick = renkoBuilder.feed(candle.close, std::to_string(candle.time));
        if (brick) {
            renkoBricks.push_back(*brick);
        }
    }
    
    // Log Renko bricks
    std::cout << "Renko Bricks:\n";
    for (const auto& brick : renkoBricks) {
        std::cout << "Time: " << brick.ts << ", Open: " << brick.open 
                  << ", Close: " << brick.close << ", Direction: " << brick.dir << "\n";
    }
    
    // Compute Ichimoku
    auto ichimokuData = IchimokuCalculator::compute_ichimoku_from_candles(candles);
    
    // Trading logic
    for (size_t i = 0; i < renkoBricks.size(); ++i) {
        if (i >= ichimokuData.base.size()) break;
        
        auto renkoClose = renkoBricks[i].close;
        auto base = ichimokuData.base[i];
        auto lead1 = ichimokuData.lead1_f[i];
        auto lead2 = ichimokuData.lead2_f[i];
        std::string now = renkoBricks[i].ts;
        
        if (std::isnan(base) || std::isnan(lead1) || std::isnan(lead2)) {
            continue;
        }
        
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
    if (position != 0 && !renkoBricks.empty()) {
        double lastClose = renkoBricks.back().close;
        double pnl = position == 1 ? lastClose - entryPrice : entryPrice - lastClose;
        balance += pnl;
        trades.back().exit = lastClose;
        trades.back().pnl = pnl;
        trades.back().exit_time = renkoBricks.back().ts;
        position = 0;
    }
}