#pragma once
#include <string>

struct Trade {
    std::string type;      // "LONG" or "SHORT"
    double entry;          // Entry price
    double exit;           // Exit price
    double pnl;            // Profit/Loss
    std::string entry_time; // Entry timestamp
    std::string exit_time;  // Exit timestamp
};