#pragma once
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

struct RenkoBrick {
    double open;
    double close;
    std::string dir;
    std::string ts;
    
    json to_json() const;
};