#pragma once

#include "MetricSystem.h"
#include <memory>
#include <string>

namespace MetricsSystem {

    // CPU Utilization Metric
    // Values are floating point numbers from 0 to N (where N = number of CPU cores)
    // Value 0 = no CPU load, Value 2 = 100% load on 2 cores
    class CPUMetric : public TypedMetric<double> {
    private:
        int cpu_cores_;
        double max_utilization_;

    public:
        // Constructor with automatic core detection or manual specification
        explicit CPUMetric(const std::string& name = "CPU", int cores = -1);

        // Add CPU-specific validation (convenience method)
        void recordValue(double value);

        // Get CPU utilization as percentage (0-100% per core)
        double getUtilizationPercentage() const;

        // Get number of cores this metric tracks
        int getCoreCount() const { return cpu_cores_; }

        // Validate CPU utilization value
        bool isValidUtilization(double value) const;

        // Static factory methods for common CPU metrics
        static std::unique_ptr<CPUMetric> createAverageUtilization(const std::string& name = "CPU Usage");
        static std::unique_ptr<CPUMetric> createPerCoreUtilization(const std::string& name, int core_count);
    };

    // HTTP Request Metric  
    // Values are integer numbers from 0 to INT_MAX representing requests per second
    class HTTPRequestMetric : public TypedMetric<int> {
    private:
        long total_requests_;
        std::chrono::system_clock::time_point start_time_;
        std::chrono::system_clock::time_point last_reset_;

    public:
        explicit HTTPRequestMetric(const std::string& name = "HTTP requests RPS");

        // Add HTTP-specific tracking (convenience method)
        void recordValue(int requests);

        // Reset with timestamp tracking
        void reset() override;

        // Get total requests since creation
        long getTotalRequests() const { return total_requests_; }

        // Get requests per second since last reset
        double getCurrentRPS() const;

        // Get uptime since metric creation
        std::chrono::seconds getUptime() const;

        // Static factory methods for different HTTP metrics
        static std::unique_ptr<HTTPRequestMetric> createRPSMetric(const std::string& name = "HTTP RPS");
        static std::unique_ptr<HTTPRequestMetric> createRequestCounter(const std::string& name = "HTTP Total");
    };

    // Memory Usage Metric (Additional example)
    // Values represent memory usage in MB
    class MemoryMetric : public TypedMetric<double> {
    private:
        double peak_usage_;
        bool track_peak_;

    public:
        explicit MemoryMetric(const std::string& name = "Memory Usage MB", bool trackPeak = true);

        // Track peak memory usage (convenience method)
        void recordValue(double memoryMB);

        // Reset peak tracking
        void reset() override;

        // Get peak memory usage since last reset
        double getPeakUsage() const { return peak_usage_; }

        // Get current memory usage (last recorded value)
        double getCurrentUsage() const;

        // Static factory methods
        static std::unique_ptr<MemoryMetric> createHeapUsage(const std::string& name = "Heap Memory MB");
        static std::unique_ptr<MemoryMetric> createTotalUsage(const std::string& name = "Total Memory MB");
    };

    // Network Throughput Metric (Additional example) 
    // Values represent bytes per second
    class NetworkMetric : public TypedMetric<long> {
    private:
        long total_bytes_;
        std::string direction_; // "in", "out", or "both"

    public:
        explicit NetworkMetric(const std::string& name = "Network Bytes/sec", 
                             const std::string& direction = "both");

        // Track total bytes (convenience method)
        void recordValue(long bytes);

        // Reset total byte counter
        void reset() override;

        // Get total bytes transferred
        long getTotalBytes() const { return total_bytes_; }

        // Get direction of metric
        const std::string& getDirection() const { return direction_; }

        // Convert bytes to human-readable format
        std::string formatThroughput(long bytesPerSecond) const;

        // Static factory methods
        static std::unique_ptr<NetworkMetric> createInboundMetric(const std::string& name = "Network In");
        static std::unique_ptr<NetworkMetric> createOutboundMetric(const std::string& name = "Network Out");
        static std::unique_ptr<NetworkMetric> createTotalMetric(const std::string& name = "Network Total");
    };

    // Metric Factory for easy creation of all metric types
    class MetricFactory {
    public:
        // CPU Metrics
        static std::unique_ptr<CPUMetric> createCPUMetric(const std::string& name = "CPU");
        static std::unique_ptr<CPUMetric> createCPUMetricWithCores(const std::string& name, int cores);

        // HTTP Metrics  
        static std::unique_ptr<HTTPRequestMetric> createHTTPMetric(const std::string& name = "HTTP requests RPS");

        // Memory Metrics
        static std::unique_ptr<MemoryMetric> createMemoryMetric(const std::string& name = "Memory Usage MB");

        // Network Metrics
        static std::unique_ptr<NetworkMetric> createNetworkMetric(const std::string& name = "Network Bytes/sec");

        // Generic typed metrics
        template<typename T>
        static std::unique_ptr<TypedMetric<T>> createGenericMetric(const std::string& name);
    };

} // namespace MetricsSystem 