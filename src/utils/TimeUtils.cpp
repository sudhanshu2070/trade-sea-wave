#include "utils/TimeUtils.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>

std::string TimeUtils::convertToIST(long timestamp) {
    std::time_t t = timestamp;
    std::tm* utcTime = std::gmtime(&t);
    
    // Convert to IST (UTC+5:30)
    utcTime->tm_hour += 5;
    utcTime->tm_min += 30;
    
    // Normalize the time (handle overflow)
    std::mktime(utcTime);
    
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", utcTime);
    return std::string(buf);
}

std::string TimeUtils::convertToIST(const std::string& timestampStr) {
    try {
        long timestamp = std::stol(timestampStr);
        return convertToIST(timestamp);
    } catch (...) {
        return timestampStr; // Return original if conversion fails
    }
}

long TimeUtils::convertToUTCTimestamp(const std::string& istTime) {
    std::tm tm = {};
    std::istringstream ss(istTime);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    
    if (ss.fail()) {
        throw std::runtime_error("Invalid IST time format");
    }
    
    // Convert back to UTC by subtracting 5:30
    tm.tm_hour -= 5;
    tm.tm_min -= 30;
    
    // Normalize
    std::mktime(&tm);
    
    return std::mktime(&tm);
}