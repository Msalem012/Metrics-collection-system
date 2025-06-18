#pragma once

#include "MetricSystem.h"
#include "SpecificMetrics.h"
#include <memory>
#include <string>

namespace MetricsSystem {

    // High-level manager for the entire metrics system
    // Provides a simple interface for common operations
    class MetricSystemManager {
    private:
        std::unique_ptr<MetricCollector> collector_;
        std::string output_file_;
        bool is_running_;

    public:
        explicit MetricSystemManager(const std::string& output_file = "metrics.txt");
        ~MetricSystemManager();

        // Non-copyable, non-movable for safety
        MetricSystemManager(const MetricSystemManager&) = delete;
        MetricSystemManager& operator=(const MetricSystemManager&) = delete;
        MetricSystemManager(MetricSystemManager&&) = delete;
        MetricSystemManager& operator=(MetricSystemManager&&) = delete;

        // System lifecycle
        void start();
        void stop();
        bool isRunning() const { return is_running_; }

        // Metric registration (call before start())
        template<typename T>
        void registerMetric(const std::string& name);
        
        // Convenient metric registration methods
        void registerCPUMetric(const std::string& name = "CPU");
        void registerHTTPMetric(const std::string& name = "HTTP requests RPS");
        void registerMemoryMetric(const std::string& name = "Memory Usage MB");
        void registerNetworkMetric(const std::string& name = "Network Bytes/sec");

        // Metric recording (non-blocking, thread-safe)
        template<typename T>
        void recordMetric(const std::string& name, T value);

        // Convenient recording methods
        void recordCPU(double utilization, const std::string& name = "CPU");
        void recordHTTPRequests(int requests, const std::string& name = "HTTP requests RPS");
        void recordMemoryUsage(double memoryMB, const std::string& name = "Memory Usage MB");
        void recordNetworkBytes(long bytes, const std::string& name = "Network Bytes/sec");

        // System operations
        void flush(); // Force immediate write
        const std::string& getOutputFile() const { return output_file_; }

        // Factory method for easy setup
        static std::unique_ptr<MetricSystemManager> create(const std::string& output_file = "metrics.txt");
    };

    // RAII helper for automatic system management
    class ScopedMetricSystem {
    private:
        std::unique_ptr<MetricSystemManager> manager_;

    public:
        explicit ScopedMetricSystem(const std::string& output_file = "metrics.txt");
        ~ScopedMetricSystem();

        // Access to the manager
        MetricSystemManager* operator->() { return manager_.get(); }
        MetricSystemManager& operator*() { return *manager_; }
        MetricSystemManager* get() { return manager_.get(); }
    };

} // namespace MetricsSystem 