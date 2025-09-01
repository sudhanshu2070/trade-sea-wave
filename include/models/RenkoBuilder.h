#pragma once
#include <vector>
#include <string>
#include <optional>
#include <limits>
#include <deque>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct RenkoBrick {
    double open;
    double close;
    std::string dir;
    std::string ts;
    
    json to_json() const;
};

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