#include "MetricSystem.h"
#include "MetricUtilities.h"
#include "SpecificMetrics.h"
#include <algorithm>
#include <chrono>
#include <iostream>

namespace MetricsSystem {

    // MetricCollector Implementation
    MetricCollector::MetricCollector(std::unique_ptr<MetricWriter> writer)
        : writer_(std::move(writer)), running_(false) {
        if (!writer_) {
            throw std::invalid_argument("MetricWriter cannot be null");
        }
    }

    MetricCollector::~MetricCollector() {
        stop();
    }

    template<typename T>
    void MetricCollector::registerMetric(const std::string& name) {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        
        // Check if metric already exists
        auto it = std::find_if(metrics_.begin(), metrics_.end(),
            [&name](const std::unique_ptr<Metric>& metric) {
                return metric->getName() == name;
            });
        
        if (it != metrics_.end()) {
            throw std::invalid_argument("Metric already registered: " + name);
        }
        
        // Create and add new metric
        auto metric = std::make_unique<TypedMetric<T>>(name);
        metrics_.push_back(std::move(metric));
    }

    template<typename T>
    void MetricCollector::recordMetric(const std::string& name, T value) {
        if (!running_) {
            return; // Silently ignore if not running
        }

        // Find the metric (read lock)
        Metric* target_metric = nullptr;
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            auto it = std::find_if(metrics_.begin(), metrics_.end(),
                [&name](const std::unique_ptr<Metric>& metric) {
                    return metric->getName() == name;
                });
            
            if (it != metrics_.end()) {
                target_metric = it->get();
            }
        }

        if (!target_metric) {
            // Auto-register metric if it doesn't exist
            try {
                registerMetric<T>(name);
                // Try again after registration
                std::lock_guard<std::mutex> lock(metrics_mutex_);
                auto it = std::find_if(metrics_.begin(), metrics_.end(),
                    [&name](const std::unique_ptr<Metric>& metric) {
                        return metric->getName() == name;
                    });
                if (it != metrics_.end()) {
                    target_metric = it->get();
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to auto-register metric '" << name << "': " << e.what() << std::endl;
                return;
            }
        }

        if (target_metric) {
            // Record the value (this is the hot path - must be fast)
            try {
                auto typed_metric = dynamic_cast<TypedMetric<T>*>(target_metric);
                if (typed_metric) {
                    typed_metric->recordValue(value);
                }
            } catch (const std::exception& e) {
                std::cerr << "Failed to record metric '" << name << "': " << e.what() << std::endl;
            }
        }
    }

    void MetricCollector::start() {
        if (running_.exchange(true)) {
            return; // Already running
        }

        // Start background worker thread
        worker_thread_ = std::thread(&MetricCollector::processMetrics, this);
        std::cout << "MetricCollector started" << std::endl;
    }

    void MetricCollector::stop() {
        if (!running_.exchange(false)) {
            return; // Already stopped
        }

        // Signal worker thread to stop and wait for it
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }

        // Final flush before stopping
        collectCurrentMetrics();
        std::cout << "MetricCollector stopped" << std::endl;
    }

    void MetricCollector::flush() {
        if (!running_) {
            return;
        }

        collectCurrentMetrics();
    }

    void MetricCollector::processMetrics() {
        const auto flush_interval = std::chrono::seconds(1); // Flush every second
        
        while (running_) {
            auto start_time = std::chrono::steady_clock::now();
            
            // Collect and write current metrics
            collectCurrentMetrics();
            
            // Calculate sleep time to maintain consistent intervals
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto sleep_time = flush_interval - elapsed;
            
            if (sleep_time > std::chrono::milliseconds(0)) {
                std::this_thread::sleep_for(sleep_time);
            }
        }
    }

    void MetricCollector::collectCurrentMetrics() {
        std::vector<MetricEntry> entries;
        auto timestamp = TimestampUtils::getCurrentTime();

        // Collect all metric values
        {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            entries.reserve(metrics_.size());

            for (auto& metric : metrics_) {
                try {
                    auto accumulated_value = metric->getAccumulatedValue();
                    
                    // Only write if there's actual data
                    if (accumulated_value) {
                        entries.emplace_back(timestamp, metric->getName(), std::move(accumulated_value));
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error collecting metric '" << metric->getName() << "': " << e.what() << std::endl;
                }
            }
        }

        // Write to file (outside of lock)
        if (!entries.empty()) {
            try {
                writer_->writeMetrics(entries);
                
                // Reset metrics after successful write
                std::lock_guard<std::mutex> lock(metrics_mutex_);
                for (auto& metric : metrics_) {
                    try {
                        metric->reset();
                    } catch (const std::exception& e) {
                        std::cerr << "Error resetting metric '" << metric->getName() << "': " << e.what() << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error writing metrics: " << e.what() << std::endl;
            }
        }
    }

    // Explicit template instantiations for common types
    template void MetricCollector::registerMetric<int>(const std::string& name);
    template void MetricCollector::registerMetric<double>(const std::string& name);
    template void MetricCollector::registerMetric<float>(const std::string& name);
    template void MetricCollector::registerMetric<long>(const std::string& name);

    template void MetricCollector::recordMetric<int>(const std::string& name, int value);
    template void MetricCollector::recordMetric<double>(const std::string& name, double value);
    template void MetricCollector::recordMetric<float>(const std::string& name, float value);
    template void MetricCollector::recordMetric<long>(const std::string& name, long value);

} // namespace MetricsSystem 