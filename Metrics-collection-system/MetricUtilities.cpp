#include "MetricUtilities.h"
#include "MetricSystem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace MetricsSystem {

    // TimestampUtils implementations
    std::chrono::system_clock::time_point TimestampUtils::getCurrentTime() {
        return std::chrono::system_clock::now();
    }

    std::string TimestampUtils::formatTimestamp(const std::chrono::system_clock::time_point& timePoint) {
        auto time_t = std::chrono::system_clock::to_time_t(timePoint);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }

    std::chrono::system_clock::time_point TimestampUtils::parseTimestamp(const std::string& timestampStr) {
        // Simple implementation for testing - in production, you'd want more robust parsing
        std::istringstream ss(timestampStr);
        std::tm tm = {};
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        
        auto time_point = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        
        // Handle milliseconds if present
        if (ss.peek() == '.') {
            ss.ignore(); // skip '.'
            int ms;
            ss >> ms;
            time_point += std::chrono::milliseconds(ms);
        }
        
        return time_point;
    }

    // MetricRegistry implementations
    void MetricRegistry::registerMetric(const std::string& name, std::unique_ptr<Metric> metric) {
        if (!MetricNameValidator::isValidName(name)) {
            throw std::invalid_argument("Invalid metric name: " + name);
        }

        UNIQUE_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        
        if (metrics_.find(name) != metrics_.end()) {
            throw std::invalid_argument("Metric already registered: " + name);
        }
        
        metrics_[name] = std::move(metric);
    }

    Metric* MetricRegistry::getMetric(const std::string& name) const {
        SHARED_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        
        auto it = metrics_.find(name);
        return (it != metrics_.end()) ? it->second.get() : nullptr;
    }

    bool MetricRegistry::hasMetric(const std::string& name) const {
        SHARED_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        return metrics_.find(name) != metrics_.end();
    }

    std::vector<std::string> MetricRegistry::getAllMetricNames() const {
        SHARED_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        
        std::vector<std::string> names;
        names.reserve(metrics_.size());
        
        for (const auto& pair : metrics_) {
            names.push_back(pair.first);
        }
        
        return names;
    }

    std::vector<std::pair<std::string, Metric*>> MetricRegistry::getAllMetrics() const {
        SHARED_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        
        std::vector<std::pair<std::string, Metric*>> metrics;
        metrics.reserve(metrics_.size());
        
        for (const auto& pair : metrics_) {
            metrics.emplace_back(pair.first, pair.second.get());
        }
        
        return metrics;
    }

    void MetricRegistry::clear() {
        UNIQUE_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        metrics_.clear();
    }

    size_t MetricRegistry::size() const {
        SHARED_LOCK<SHARED_MUTEX> lock(registry_mutex_);
        return metrics_.size();
    }

    // MetricNameValidator implementations
    bool MetricNameValidator::isValidName(const std::string& name) {
        if (name.empty()) {
            return false;
        }
        
        // Check for invalid characters (quotes, control characters, etc.)
        for (char c : name) {
            if (c == '"' || c == '\n' || c == '\r' || c == '\t' || c < 32) {
                return false;
            }
        }
        
        return true;
    }

    std::string MetricNameValidator::formatNameForOutput(const std::string& name) {
        if (!isValidName(name)) {
            throw std::invalid_argument("Cannot format invalid metric name: " + name);
        }
        
        return "\"" + name + "\"";
    }

    std::string MetricNameValidator::extractNameFromOutput(const std::string& formattedName) {
        if (formattedName.length() < 2 || formattedName.front() != '"' || formattedName.back() != '"') {
            throw std::invalid_argument("Invalid formatted metric name: " + formattedName);
        }
        
        return formattedName.substr(1, formattedName.length() - 2);
    }

    // ValueFormatter implementations
    template<typename T>
    std::string ValueFormatter::formatValue(const T& value) {
        if constexpr (std::is_floating_point_v<T>) {
            return formatDouble(static_cast<double>(value));
        } else if constexpr (std::is_integral_v<T>) {
            return formatInteger(static_cast<long>(value));
        } else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

    std::string ValueFormatter::formatDouble(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    std::string ValueFormatter::formatInteger(long value) {
        return std::to_string(value);
    }

    // Explicit template instantiations for common types
    template std::string ValueFormatter::formatValue<int>(const int& value);
    template std::string ValueFormatter::formatValue<double>(const double& value);
    template std::string ValueFormatter::formatValue<float>(const float& value);
    template std::string ValueFormatter::formatValue<long>(const long& value);

} // namespace MetricsSystem 