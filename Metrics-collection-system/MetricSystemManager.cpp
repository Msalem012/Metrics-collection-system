#include "MetricSystemManager.h"
#include "MetricUtilities.h"
#include <iostream>
#include <stdexcept>

namespace MetricsSystem {

    // MetricSystemManager Implementation
    MetricSystemManager::MetricSystemManager(const std::string& output_file) 
        : output_file_(output_file), is_running_(false) {
        
        // Create the metric collection system
        collector_ = MetricSystemFactory::createSystem(output_file);
        
        if (!collector_) {
            throw std::runtime_error("Failed to create metric collector");
        }
    }

    MetricSystemManager::~MetricSystemManager() {
        stop();
    }

    void MetricSystemManager::start() {
        if (is_running_) {
            return; // Already running
        }

        try {
            collector_->start();
            is_running_ = true;
            std::cout << "Metrics collection system started" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to start metrics system: " << e.what() << std::endl;
            throw;
        }
    }

    void MetricSystemManager::stop() {
        if (!is_running_) {
            return; // Already stopped
        }

        try {
            collector_->stop();
            is_running_ = false;
            std::cout << "Metrics collection system stopped" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error stopping metrics system: " << e.what() << std::endl;
        }
    }

    template<typename T>
    void MetricSystemManager::registerMetric(const std::string& name) {
        if (!collector_) {
            throw std::runtime_error("Metric collector not initialized");
        }

        try {
            collector_->registerMetric<T>(name);
            std::cout << "Registered metric: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to register metric '" << name << "': " << e.what() << std::endl;
            throw;
        }
    }

    void MetricSystemManager::registerCPUMetric(const std::string& name) {
        registerMetric<double>(name);
    }

    void MetricSystemManager::registerHTTPMetric(const std::string& name) {
        registerMetric<int>(name);
    }

    void MetricSystemManager::registerMemoryMetric(const std::string& name) {
        registerMetric<double>(name);
    }

    void MetricSystemManager::registerNetworkMetric(const std::string& name) {
        registerMetric<long>(name);
    }

    template<typename T>
    void MetricSystemManager::recordMetric(const std::string& name, T value) {
        if (!collector_) {
            return; // Silently ignore if not initialized
        }

        if (!is_running_) {
            return; // Silently ignore if not running
        }

        try {
            collector_->recordMetric<T>(name, value);
        } catch (const std::exception& e) {
            std::cerr << "Failed to record metric '" << name << "': " << e.what() << std::endl;
        }
    }

    void MetricSystemManager::recordCPU(double utilization, const std::string& name) {
        recordMetric<double>(name, utilization);
    }

    void MetricSystemManager::recordHTTPRequests(int requests, const std::string& name) {
        recordMetric<int>(name, requests);
    }

    void MetricSystemManager::recordMemoryUsage(double memoryMB, const std::string& name) {
        recordMetric<double>(name, memoryMB);
    }

    void MetricSystemManager::recordNetworkBytes(long bytes, const std::string& name) {
        recordMetric<long>(name, bytes);
    }

    void MetricSystemManager::flush() {
        if (collector_ && is_running_) {
            collector_->flush();
        }
    }

    std::unique_ptr<MetricSystemManager> MetricSystemManager::create(const std::string& output_file) {
        return std::make_unique<MetricSystemManager>(output_file);
    }

    // ScopedMetricSystem Implementation
    ScopedMetricSystem::ScopedMetricSystem(const std::string& output_file) 
        : manager_(MetricSystemManager::create(output_file)) {
        manager_->start();
    }

    ScopedMetricSystem::~ScopedMetricSystem() {
        if (manager_) {
            manager_->stop();
        }
    }

    // Explicit template instantiations for common types
    template void MetricSystemManager::registerMetric<int>(const std::string& name);
    template void MetricSystemManager::registerMetric<double>(const std::string& name);
    template void MetricSystemManager::registerMetric<float>(const std::string& name);
    template void MetricSystemManager::registerMetric<long>(const std::string& name);

    template void MetricSystemManager::recordMetric<int>(const std::string& name, int value);
    template void MetricSystemManager::recordMetric<double>(const std::string& name, double value);
    template void MetricSystemManager::recordMetric<float>(const std::string& name, float value);
    template void MetricSystemManager::recordMetric<long>(const std::string& name, long value);

} // namespace MetricsSystem 