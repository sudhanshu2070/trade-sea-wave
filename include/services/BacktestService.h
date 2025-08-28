#pragma once
#include <string>
#include "../models/BacktestResult.h"
#include "DeltaService.h"

class BacktestService {
public:
    BacktestService(DeltaService& deltaService);

    BacktestResult run(const std::string& strategy,
                       const std::string& symbol,
                       long start,
                       long end);

private:
    DeltaService& deltaService_;
};