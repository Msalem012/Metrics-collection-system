#pragma once

#define _CRT_SECURE_NO_WARNINGS  // Disable Windows security warnings

#include <string>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <mutex>

// Use shared_mutex if available (C++17), otherwise fall back to mutex
#if __cplusplus >= 201703L
    #include <shared_mutex>
    #define SHARED_MUTEX std::shared_mutex
    #define SHARED_LOCK std::shared_lock
    #define UNIQUE_LOCK std::unique_lock
#else
    #define SHARED_MUTEX std::mutex
    #define SHARED_LOCK std::lock_guard
    #define UNIQUE_LOCK std::lock_guard
#endif

namespace MetricsSystem {

    // Forward declaration
    class Metric;

    // Utility class for timestamp operations
    class TimestampUtils {
    public:
        // Get current timestamp
        static std::chrono::system_clock::time_point getCurrentTime();
        
        // Format timestamp to required string format: "2025-06-01 15:00:01.653"
        static std::string formatTimestamp(const std::chrono::system_clock::time_point& timePoint);
        
        // Parse timestamp from string (for testing purposes)
        static std::chrono::system_clock::time_point parseTimestamp(const std::string& timestampStr);
    };

    // Metric registry for managing named metrics
    class MetricRegistry {
    private:
        std::unordered_map<std::string, std::unique_ptr<Metric>> metrics_;
        mutable SHARED_MUTEX registry_mutex_;  // Allows multiple readers, single writer

    public:
        MetricRegistry() = default;
        ~MetricRegistry() = default;

        // Non-copyable, non-movable for thread safety
        MetricRegistry(const MetricRegistry&) = delete;
        MetricRegistry& operator=(const MetricRegistry&) = delete;
        MetricRegistry(MetricRegistry&&) = delete;
        MetricRegistry& operator=(MetricRegistry&&) = delete;

        // Register a new metric (thread-safe)
        void registerMetric(const std::string& name, std::unique_ptr<Metric> metric);

        // Get metric by name (thread-safe, returns nullptr if not found)
        Metric* getMetric(const std::string& name) const;

        // Check if metric exists
        bool hasMetric(const std::string& name) const;

        // Get all metric names (for iteration)
        std::vector<std::string> getAllMetricNames() const;

        // Get all metrics (for processing)
        std::vector<std::pair<std::string, Metric*>> getAllMetrics() const;

        // Remove all metrics (for cleanup)
        void clear();

        // Get count of registered metrics
        size_t size() const;
    };

    // Helper class for metric name validation and formatting
    class MetricNameValidator {
    public:
        // Validate metric name (must not be empty, contain quotes, etc.)
        static bool isValidName(const std::string& name);
        
        // Format metric name for output (add quotes, escape if needed)
        static std::string formatNameForOutput(const std::string& name);
        
        // Extract metric name from formatted output
        static std::string extractNameFromOutput(const std::string& formattedName);
    };

    // Value formatting utilities
    class ValueFormatter {
    public:
        // Format different value types for output
        template<typename T>
        static std::string formatValue(const T& value);
        
        // Format floating point with specific precision
        static std::string formatDouble(double value, int precision = 2);
        
        // Format integer values
        static std::string formatInteger(long value);
    };

} // namespace MetricsSystem 