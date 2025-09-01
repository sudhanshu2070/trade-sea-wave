#include "models/Backtester.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <deque>
#include <limits>
#include <optional>
#include <algorithm>

// ----------------------------
// RenkoBrick structure
// ----------------------------
struct RenkoBrick {
    double open;
    double close;
    std::string dir;
    std::string ts;
    
    json to_json() const {
        return {
            {"open", open},
            {"close", close},
            {"dir", dir},
            {"ts", ts}
        };
    }
};

// ----------------------------
// Renko builder
// ----------------------------
class RenkoBuilder {
public:
    explicit RenkoBuilder(double box=40.0): box_(box), last_close_(std::numeric_limits<double>::quiet_NaN()) {}
    void set_box(double b) { box_ = b; }
    
    // feed price; returns optional last completed brick (may create multiple bricks but returns last)
    std::optional<RenkoBrick> feed(double price, const std::string &ts) {
        if (std::isnan(last_close_)) { 
            last_close_ = price; 
            return std::nullopt; 
        }
        
        if (price >= last_close_ + box_) {
            int cnt = static_cast<int>((price - last_close_) / box_);
            for (int i=0; i<cnt; i++) {
                RenkoBrick b; 
                b.open = last_close_ + i * box_; 
                b.close = last_close_ + (i+1) * box_;
                b.dir = "UP"; 
                b.ts = ts; 
                bricks_.push_back(b);
            }
            last_close_ += cnt * box_;
            return bricks_.back();
        } else if (price <= last_close_ - box_) {
            int cnt = static_cast<int>((last_close_ - price) / box_);
            for (int i=0; i<cnt; i++) {
                RenkoBrick b; 
                b.open = last_close_ - i * box_; 
                b.close = last_close_ - (i+1) * box_;
                b.dir = "DOWN"; 
                b.ts = ts; 
                bricks_.push_back(b);
            }
            last_close_ -= cnt * box_;
            return bricks_.back();
        }
        return std::nullopt;
    }
    
    const std::vector<RenkoBrick>& history() const { return bricks_; }
    
    std::vector<json> latest_json(size_t n=200) const {
        std::vector<json> out; 
        size_t s = bricks_.size();
        size_t start = (s > n) ? s - n : 0;
        for (size_t i = start; i < s; i++) {
            out.push_back(bricks_[i].to_json());
        }
        return out;
    }

private:
    double box_;
    double last_close_;
    std::vector<RenkoBrick> bricks_;
};

// ----------------------------
// Ichimoku functions (Donchian mid, forward shift)
// ----------------------------
static std::vector<double> sliding_max(const std::vector<double>& a, int w) {
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

static std::vector<double> sliding_min(const std::vector<double>& a, int w) {
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

static std::vector<double> donchian_mid(const std::vector<double>& highs, const std::vector<double>& lows, int len) {
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

static std::vector<double> shift_forward(const std::vector<double>& x, int disp) {
    int n = static_cast<int>(x.size()); 
    std::vector<double> y(n, std::numeric_limits<double>::quiet_NaN());
    for (int i = disp; i < n; i++) {
        y[i] = x[i - disp];
    }
    return y;
}

struct IchimokuSeries {
    std::vector<double> conversion;
    std::vector<double> base;
    std::vector<double> lead1_f;
    std::vector<double> lead2_f;
    std::vector<double> close;
    std::vector<std::string> time;
};

static IchimokuSeries compute_ichimoku_from_candles(const std::vector<Candle>& candles, int conv=5, int basep=26, int spanb=52, int disp=26) {
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
    RenkoBuilder renkoBuilder(40.0); // Fixed box size
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
    auto ichimokuData = compute_ichimoku_from_candles(candles);
    
    // Log Ichimoku data
    std::cout << "Ichimoku Data:\n";
    for (size_t i = 0; i < ichimokuData.base.size(); ++i) {
        std::cout << "Index: " << i
                  << ", Base: " << ichimokuData.base[i]
                  << ", Lead1: " << ichimokuData.lead1_f[i]
                  << ", Lead2: " << ichimokuData.lead2_f[i] << "\n";
    }
    
    // Trading logic
    for (size_t i = 0; i < renkoBricks.size(); ++i) {
        if (i >= ichimokuData.base.size()) break; // Ensure we don't go out of bounds
        
        auto renkoClose = renkoBricks[i].close;
        auto base = ichimokuData.base[i];
        auto lead1 = ichimokuData.lead1_f[i];
        auto lead2 = ichimokuData.lead2_f[i];
        std::string now = renkoBricks[i].ts;
        
        // Skip if Ichimoku values are not available
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