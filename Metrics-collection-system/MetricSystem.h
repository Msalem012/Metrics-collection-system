#pragma once

#define _CRT_SECURE_NO_WARNINGS  // Disable Windows security warnings

#include <string>
#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <fstream>

namespace MetricsSystem {

    // Forward declarations
    class MetricValue;
    class Metric;
    class MetricCollector;
    class MetricWriter;

    // Timestamp type for consistent time handling
    using TimePoint = std::chrono::system_clock::time_point;

    // Base class for metric values that can hold different data types
    class MetricValue {
    public:
        virtual ~MetricValue() = default;
        virtual std::string toString() const = 0;
        virtual std::unique_ptr<MetricValue> clone() const = 0;
        virtual void reset() = 0;
        virtual void accumulate(const MetricValue& other) = 0;
    };

    // Template implementation for specific data types
    template<typename T>
    class TypedMetricValue : public MetricValue {
    private:
        T value_;
        size_t count_;

    public:
        TypedMetricValue(T value = T{}) : value_(value), count_(value != T{} ? 1 : 0) {}

        std::string toString() const override;
        std::unique_ptr<MetricValue> clone() const override;
        void reset() override;
        void accumulate(const MetricValue& other) override;
        
        T getValue() const { return count_ > 0 ? value_ / static_cast<T>(count_) : T{}; }
        void addValue(T val) { value_ += val; count_++; }
    };

    // Metric entry that will be written to file
    struct MetricEntry {
        TimePoint timestamp;
        std::string name;
        std::unique_ptr<MetricValue> value;

        MetricEntry(TimePoint ts, const std::string& n, std::unique_ptr<MetricValue> v)
            : timestamp(ts), name(n), value(std::move(v)) {}
    };

    // Base interface for all metric types
    class Metric {
    public:
        virtual ~Metric() = default;
        virtual std::string getName() const = 0;
        virtual void recordValue(std::unique_ptr<MetricValue> value) = 0;
        virtual std::unique_ptr<MetricValue> getAccumulatedValue() const = 0;
        virtual void reset() = 0;
    };

    // Template implementation for specific metric types
    template<typename T>
    class TypedMetric : public Metric {
    private:
        std::string name_;
        mutable std::mutex mutex_;
        T accumulated_value_;
        size_t count_;

    public:
        explicit TypedMetric(const std::string& name) 
            : name_(name), accumulated_value_(T{}), count_(0) {}

        std::string getName() const override { return name_; }
        void recordValue(std::unique_ptr<MetricValue> value) override;
        std::unique_ptr<MetricValue> getAccumulatedValue() const override;
        void reset() override;

        // Convenience method for recording typed values
        void recordValue(T value);
    };

    // Thread-safe metric collector - main interface for recording metrics
    class MetricCollector {
    private:
        std::vector<std::unique_ptr<Metric>> metrics_;
        std::queue<MetricEntry> pending_entries_;
        std::mutex metrics_mutex_;
        std::mutex queue_mutex_;
        std::atomic<bool> running_;
        std::thread worker_thread_;
        std::unique_ptr<MetricWriter> writer_;

        // Internal processing methods
        void processMetrics();
        void collectCurrentMetrics();

    public:
        explicit MetricCollector(std::unique_ptr<MetricWriter> writer);
        ~MetricCollector();

        // Register new metrics
        template<typename T>
        void registerMetric(const std::string& name);

        // Record metric values (non-blocking)
        template<typename T>
        void recordMetric(const std::string& name, T value);

        // Control methods
        void start();
        void stop();
        void flush(); // Force write current metrics
    };

    // Handles writing metrics to file with proper formatting
    class MetricWriter {
    private:
        std::string output_file_;
        std::ofstream file_stream_;
        std::mutex write_mutex_;

        std::string formatTimestamp(const TimePoint& tp) const;

    public:
        explicit MetricWriter(const std::string& filename);
        ~MetricWriter();

        void writeMetrics(const std::vector<MetricEntry>& entries);
        void close();
    };

    // Factory class for easy system setup
    class MetricSystemFactory {
    public:
        static std::unique_ptr<MetricCollector> createSystem(const std::string& output_file);
    };

} // namespace MetricsSystem 