#pragma once
#include "RenkoBrick.h"
#include <vector>
#include <string>
#include <optional>
#include <limits>

class RenkoBuilder {
public:
    explicit RenkoBuilder(double box=40.0);
    void set_box(double b);
    std::optional<RenkoBrick> feed(double price, const std::string &ts);
    const std::vector<RenkoBrick>& history() const;
    std::vector<json> latest_json(size_t n=200) const;

private:
    double box_;
    double last_close_;
    std::vector<RenkoBrick> bricks_;
};