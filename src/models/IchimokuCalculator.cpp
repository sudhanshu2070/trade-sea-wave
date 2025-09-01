#include "models/IchimokuCalculator.h"

std::vector<double> IchimokuCalculator::sliding_max(const std::vector<double>& a, int w) {
    int n = static_cast<int>(a.size()); 
    std::vector<double> res(n, std::numeric_limits<double>::quiet_NaN());
    if (w <= 0) return res;
    
    std::deque<int> dq;
    for (int i = 0; i < n; i++) {
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - w) dq.pop_front();
        if (i >= w - 1) res[i] = a[dq.front()];
    }
    return res;
}

std::vector<double> IchimokuCalculator::sliding_min(const std::vector<double>& a, int w) {
    int n = static_cast<int>(a.size()); 
    std::vector<double> res(n, std::numeric_limits<double>::quiet_NaN());
    if (w <= 0) return res;
    
    std::deque<int> dq;
    for (int i = 0; i < n; i++) {
        while (!dq.empty() && a[dq.back()] >= a[i]) dq.pop_back();
        dq.push_back(i);
        if (dq.front() <= i - w) dq.pop_front();
        if (i >= w - 1) res[i] = a[dq.front()];
    }
    return res;
}

std::vector<double> IchimokuCalculator::donchian_mid(const std::vector<double>& highs, const std::vector<double>& lows, int len) {
    auto hi = sliding_max(highs, len);
    auto lo = sliding_min(lows, len);
    int n = static_cast<int>(highs.size()); 
    std::vector<double> mid(n, std::numeric_limits<double>::quiet_NaN());
    for (int i = 0; i < n; i++) {
        if (!std::isnan(hi[i]) && !std::isnan(lo[i])) {
            mid[i] = (hi[i] + lo[i]) / 2.0;
        }
    }
    return mid;
}

std::vector<double> IchimokuCalculator::shift_forward(const std::vector<double>& x, int disp) {
    int n = static_cast<int>(x.size()); 
    std::vector<double> y(n, std::numeric_limits<double>::quiet_NaN());
    for (int i = disp; i < n; i++) {
        y[i] = x[i - disp];
    }
    return y;
}

IchimokuSeries IchimokuCalculator::compute_ichimoku_from_candles(const std::vector<Candle>& candles, int conv, int basep, int spanb, int disp) {
    int n = static_cast<int>(candles.size());
    std::vector<double> H(n), L(n), C(n); 
    std::vector<std::string> T(n);
    
    for (int i = 0; i < n; i++) { 
        H[i] = candles[i].high; 
        L[i] = candles[i].low; 
        C[i] = candles[i].close; 
        T[i] = std::to_string(candles[i].time); 
    }
    
    IchimokuSeries s;
    s.conversion = donchian_mid(H, L, conv);
    s.base = donchian_mid(H, L, basep);
    
    std::vector<double> lead1_now(n, std::numeric_limits<double>::quiet_NaN());
    for (int i = 0; i < n; i++) {
        if (!std::isnan(s.conversion[i]) && !std::isnan(s.base[i])) {
            lead1_now[i] = (s.conversion[i] + s.base[i]) / 2.0;
        }
    }
    
    auto spanb_now = donchian_mid(H, L, spanb);
    s.lead1_f = shift_forward(lead1_now, disp);
    s.lead2_f = shift_forward(spanb_now, disp);
    s.close = C; 
    s.time = T;
    
    return s;
}