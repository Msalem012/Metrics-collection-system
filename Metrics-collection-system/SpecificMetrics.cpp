#include "SpecificMetrics.h"
#include "MetricUtilities.h"
#include <thread>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace MetricsSystem {

    // CPUMetric Implementation
    CPUMetric::CPUMetric(const std::string& name, int cores) 
        : TypedMetric<double>(name), cpu_cores_(cores), max_utilization_(0.0) {
        
        if (cores <= 0) {
            // Auto-detect CPU cores
            cpu_cores_ = static_cast<int>(std::thread::hardware_concurrency());
            if (cpu_cores_ <= 0) {
                cpu_cores_ = 1; // Fallback to 1 core if detection fails
            }
        }
        
        max_utilization_ = static_cast<double>(cpu_cores_);
    }

    void CPUMetric::recordValue(double value) {
        if (!isValidUtilization(value)) {
            throw std::invalid_argument("Invalid CPU utilization value: " + std::to_string(value) + 
                                      ". Must be between 0 and " + std::to_string(max_utilization_));
        }
        
        // Call parent implementation
        TypedMetric<double>::recordValue(value);
    }

    double CPUMetric::getUtilizationPercentage() const {
        auto current_value = getAccumulatedValue();
        auto typed_value = dynamic_cast<TypedMetricValue<double>*>(current_value.get());
        
        if (!typed_value || cpu_cores_ == 0) {
            return 0.0;
        }
        
        // Convert to percentage (0-100% per core)
        return (typed_value->getValue() / static_cast<double>(cpu_cores_)) * 100.0;
    }

    bool CPUMetric::isValidUtilization(double value) const {
        return value >= 0.0 && value <= max_utilization_;
    }

    std::unique_ptr<CPUMetric> CPUMetric::createAverageUtilization(const std::string& name) {
        return std::make_unique<CPUMetric>(name);
    }

    std::unique_ptr<CPUMetric> CPUMetric::createPerCoreUtilization(const std::string& name, int core_count) {
        return std::make_unique<CPUMetric>(name, core_count);
    }

    // HTTPRequestMetric Implementation
    HTTPRequestMetric::HTTPRequestMetric(const std::string& name) 
        : TypedMetric<int>(name), total_requests_(0) {
        start_time_ = TimestampUtils::getCurrentTime();
        last_reset_ = start_time_;
    }

    void HTTPRequestMetric::recordValue(int requests) {
        if (requests < 0) {
            throw std::invalid_argument("HTTP request count cannot be negative: " + std::to_string(requests));
        }
        
        total_requests_ += requests;
        
        // Call parent implementation
        TypedMetric<int>::recordValue(requests);
    }

    void HTTPRequestMetric::reset() {
        last_reset_ = TimestampUtils::getCurrentTime();
        
        // Call parent reset
        TypedMetric<int>::reset();
    }

    double HTTPRequestMetric::getCurrentRPS() const {
        auto now = TimestampUtils::getCurrentTime();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_reset_);
        
        if (duration.count() <= 0) {
            return 0.0;
        }
        
        auto current_value = getAccumulatedValue();
        auto typed_value = dynamic_cast<TypedMetricValue<int>*>(current_value.get());
        
        if (!typed_value) {
            return 0.0;
        }
        
        return static_cast<double>(typed_value->getValue()) / duration.count();
    }

    std::chrono::seconds HTTPRequestMetric::getUptime() const {
        auto now = TimestampUtils::getCurrentTime();
        return std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    }

    std::unique_ptr<HTTPRequestMetric> HTTPRequestMetric::createRPSMetric(const std::string& name) {
        return std::make_unique<HTTPRequestMetric>(name);
    }

    std::unique_ptr<HTTPRequestMetric> HTTPRequestMetric::createRequestCounter(const std::string& name) {
        return std::make_unique<HTTPRequestMetric>(name);
    }

    // MemoryMetric Implementation
    MemoryMetric::MemoryMetric(const std::string& name, bool trackPeak) 
        : TypedMetric<double>(name), peak_usage_(0.0), track_peak_(trackPeak) {}

    void MemoryMetric::recordValue(double memoryMB) {
        if (memoryMB < 0.0) {
            throw std::invalid_argument("Memory usage cannot be negative: " + std::to_string(memoryMB));
        }
        
        if (track_peak_ && memoryMB > peak_usage_) {
            peak_usage_ = memoryMB;
        }
        
        // Call parent implementation
        TypedMetric<double>::recordValue(memoryMB);
    }

    void MemoryMetric::reset() {
        peak_usage_ = 0.0;
        
        // Call parent reset
        TypedMetric<double>::reset();
    }

    double MemoryMetric::getCurrentUsage() const {
        auto current_value = getAccumulatedValue();
        auto typed_value = dynamic_cast<TypedMetricValue<double>*>(current_value.get());
        
        return typed_value ? typed_value->getValue() : 0.0;
    }

    std::unique_ptr<MemoryMetric> MemoryMetric::createHeapUsage(const std::string& name) {
        return std::make_unique<MemoryMetric>(name, true);
    }

    std::unique_ptr<MemoryMetric> MemoryMetric::createTotalUsage(const std::string& name) {
        return std::make_unique<MemoryMetric>(name, true);
    }

    // NetworkMetric Implementation
    NetworkMetric::NetworkMetric(const std::string& name, const std::string& direction) 
        : TypedMetric<long>(name), total_bytes_(0), direction_(direction) {
        
        if (direction != "in" && direction != "out" && direction != "both") {
            throw std::invalid_argument("Invalid network direction: " + direction + 
                                      ". Must be 'in', 'out', or 'both'");
        }
    }

    void NetworkMetric::recordValue(long bytes) {
        if (bytes < 0) {
            throw std::invalid_argument("Network bytes cannot be negative: " + std::to_string(bytes));
        }
        
        total_bytes_ += bytes;
        
        // Call parent implementation
        TypedMetric<long>::recordValue(bytes);
    }

    void NetworkMetric::reset() {
        // Note: We keep total_bytes_ for lifetime statistics
        // Only reset the accumulated values for current period
        TypedMetric<long>::reset();
    }

    std::string NetworkMetric::formatThroughput(long bytesPerSecond) const {
        std::ostringstream oss;
        
        if (bytesPerSecond < 1024) {
            oss << bytesPerSecond << " B/s";
        } else if (bytesPerSecond < 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << 
                   (static_cast<double>(bytesPerSecond) / 1024.0) << " KB/s";
        } else if (bytesPerSecond < 1024 * 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << 
                   (static_cast<double>(bytesPerSecond) / (1024.0 * 1024.0)) << " MB/s";
        } else {
            oss << std::fixed << std::setprecision(2) << 
                   (static_cast<double>(bytesPerSecond) / (1024.0 * 1024.0 * 1024.0)) << " GB/s";
        }
        
        return oss.str();
    }

    std::unique_ptr<NetworkMetric> NetworkMetric::createInboundMetric(const std::string& name) {
        return std::make_unique<NetworkMetric>(name, "in");
    }

    std::unique_ptr<NetworkMetric> NetworkMetric::createOutboundMetric(const std::string& name) {
        return std::make_unique<NetworkMetric>(name, "out");
    }

    std::unique_ptr<NetworkMetric> NetworkMetric::createTotalMetric(const std::string& name) {
        return std::make_unique<NetworkMetric>(name, "both");
    }

    // MetricFactory Implementation
    std::unique_ptr<CPUMetric> MetricFactory::createCPUMetric(const std::string& name) {
        return std::make_unique<CPUMetric>(name);
    }

    std::unique_ptr<CPUMetric> MetricFactory::createCPUMetricWithCores(const std::string& name, int cores) {
        return std::make_unique<CPUMetric>(name, cores);
    }

    std::unique_ptr<HTTPRequestMetric> MetricFactory::createHTTPMetric(const std::string& name) {
        return std::make_unique<HTTPRequestMetric>(name);
    }

    std::unique_ptr<MemoryMetric> MetricFactory::createMemoryMetric(const std::string& name) {
        return std::make_unique<MemoryMetric>(name);
    }

    std::unique_ptr<NetworkMetric> MetricFactory::createNetworkMetric(const std::string& name) {
        return std::make_unique<NetworkMetric>(name);
    }

    template<typename T>
    std::unique_ptr<TypedMetric<T>> MetricFactory::createGenericMetric(const std::string& name) {
        return std::make_unique<TypedMetric<T>>(name);
    }

    // Explicit template instantiations for common types
    template std::unique_ptr<TypedMetric<int>> MetricFactory::createGenericMetric<int>(const std::string& name);
    template std::unique_ptr<TypedMetric<double>> MetricFactory::createGenericMetric<double>(const std::string& name);
    template std::unique_ptr<TypedMetric<float>> MetricFactory::createGenericMetric<float>(const std::string& name);
    template std::unique_ptr<TypedMetric<long>> MetricFactory::createGenericMetric<long>(const std::string& name);

} // namespace MetricsSystem 