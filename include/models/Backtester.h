#pragma once
#include <vector>
#include <string>
#include "../models/Candle.h"

struct Trade {
    std::string type;
    double entry;
    double exit;
    double pnl;
    std::string entry_time;
    std::string exit_time;
};

class Backtester {
public:
    double balance;
    double entryPrice;
    int position; // 1 = long, -1 = short, 0 = flat
    std::vector<Trade> trades;

public:
    Backtester(double initial_balance = 10000.0);

    void run(std::vector<Candle>& candles);

    const std::vector<Trade>& getTrades() const { return trades; }
    double getBalance() const { return balance; }
};