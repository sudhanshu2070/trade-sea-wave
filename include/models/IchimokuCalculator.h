#pragma once
#include <vector>
#include <string>
#include <limits>
#include <deque>
#include <cmath>
#include "models/Candle.h"

struct IchimokuSeries {
    std::vector<double> conversion;
    std::vector<double> base;
    std::vector<double> lead1_f;
    std::vector<double> lead2_f;
    std::vector<double> close;
    std::vector<std::string> time;
};

class IchimokuCalculator {
public:
    static std::vector<double> sliding_max(const std::vector<double>& a, int w);
    static std::vector<double> sliding_min(const std::vector<double>& a, int w);
    static std::vector<double> donchian_mid(const std::vector<double>& highs, const std::vector<double>& lows, int len);
    static std::vector<double> shift_forward(const std::vector<double>& x, int disp);
    static IchimokuSeries compute_ichimoku_from_candles(const std::vector<Candle>& candles, int conv=5, int basep=26, int spanb=52, int disp=26);
};