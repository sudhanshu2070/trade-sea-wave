#include "models/RenkoBuilder.h"
#include <cmath>

RenkoBuilder::RenkoBuilder(double box) : box_(box), last_close_(std::numeric_limits<double>::quiet_NaN()) {}

void RenkoBuilder::set_box(double b) { box_ = b; }

std::optional<RenkoBrick> RenkoBuilder::feed(double price, const std::string &ts) {
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

const std::vector<RenkoBrick>& RenkoBuilder::history() const { return bricks_; }

std::vector<json> RenkoBuilder::latest_json(size_t n) const {
    std::vector<json> out; 
    size_t s = bricks_.size();
    size_t start = (s > n) ? s - n : 0;
    for (size_t i = start; i < s; i++) {
        out.push_back(bricks_[i].to_json());
    }
    return out;
}