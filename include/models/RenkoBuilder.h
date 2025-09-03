#pragma once
#include "RenkoBrick.h"
#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <cmath>

class RenkoBuilder {
public:
    explicit RenkoBuilder(double box=40.0) : box_(box), last_close_(std::numeric_limits<double>::quiet_NaN()), trend_(Trend::NONE) {}

    void set_box(double b) { box_ = b; }

    // Feed one price (live mode)
    std::optional<RenkoBrick> feed(double price, const std::string &ts="") {
        if (std::isnan(last_close_)) { 
            last_close_ = price; 
            return std::nullopt; 
        }

        double movement = price - last_close_;

        if (trend_ == Trend::NONE) {
            if (std::abs(movement) >= box_) {
                int num = static_cast<int>(std::floor(std::abs(movement) / box_));
                trend_ = (movement > 0) ? Trend::UP : Trend::DOWN;
                for (int j=0; j<num; j++) {
                    RenkoBrick b;
                    b.open = last_close_;
                    b.close = (trend_ == Trend::UP) ? last_close_ + box_ : last_close_ - box_;
                    b.trend = trend_;
                    b.ts = ts;
                    bricks_.push_back(b);
                    last_close_ = b.close;
                }
                return bricks_.back();
            }
        } 
        else if (trend_ == Trend::UP) {
            double cont = last_close_ + box_;
            double rev  = last_close_ - 2 * box_;
            if (price >= cont) {
                int num = static_cast<int>(std::floor((price - last_close_) / box_));
                for (int j=0; j<num; j++) {
                    RenkoBrick b;
                    b.open = last_close_;
                    b.close = last_close_ + box_;
                    b.trend = Trend::UP;
                    b.ts = ts;
                    bricks_.push_back(b);
                    last_close_ = b.close;
                }
                return bricks_.back();
            } else if (price <= rev) {
                trend_ = Trend::DOWN;
                int num = static_cast<int>(std::floor((last_close_ - price) / box_)) - 1;
                for (int j=0; j<num; j++) {
                    RenkoBrick b;
                    b.open = last_close_;
                    b.close = last_close_ - box_;
                    b.trend = Trend::DOWN;
                    b.ts = ts;
                    bricks_.push_back(b);
                    last_close_ = b.close;
                }
                return bricks_.back();
            }
        } 
        else if (trend_ == Trend::DOWN) {
            double cont = last_close_ - box_;
            double rev  = last_close_ + 2 * box_;
            if (price <= cont) {
                int num = static_cast<int>(std::floor((last_close_ - price) / box_));
                for (int j=0; j<num; j++) {
                    RenkoBrick b;
                    b.open = last_close_;
                    b.close = last_close_ - box_;
                    b.trend = Trend::DOWN;
                    b.ts = ts;
                    bricks_.push_back(b);
                    last_close_ = b.close;
                }
                return bricks_.back();
            } else if (price >= rev) {
                trend_ = Trend::UP;
                int num = static_cast<int>(std::floor((price - last_close_) / box_)) - 1;
                for (int j=0; j<num; j++) {
                    RenkoBrick b;
                    b.open = last_close_;
                    b.close = last_close_ + box_;
                    b.trend = Trend::UP;
                    b.ts = ts;
                    bricks_.push_back(b);
                    last_close_ = b.close;
                }
                return bricks_.back();
            }
        }
        return std::nullopt;
    }

    // Batch mode: from vector of closes
    static std::vector<RenkoBrick> build_from_closes(const std::vector<double>& closes, double box) {
        RenkoBuilder rb(box);
        for (size_t i=0; i<closes.size(); ++i) {
            rb.feed(closes[i]);
        }
        return rb.history();
    }

    // History accessor
    const std::vector<RenkoBrick>& history() const { return bricks_; }

    // Latest bricks in JSON format
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
    Trend trend_;
    std::vector<RenkoBrick> bricks_;
};