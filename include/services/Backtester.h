#pragma once
#include <vector>
#include "models/Candle.h"
#include "models/Trade.h"

class Backtester {
public:
    Backtester(double initial_balance);
    void run(std::vector<Candle>& candles);
    
    double balance;
    double entryPrice;
    int position;
    std::vector<Trade> trades;
};