#pragma once
#include <string>
#include <ctime>

class TimeUtils {
public:
    static std::string convertToIST(long timestamp);
    static std::string convertToIST(const std::string& timestampStr);
    static long convertToUTCTimestamp(const std::string& istTime);
};