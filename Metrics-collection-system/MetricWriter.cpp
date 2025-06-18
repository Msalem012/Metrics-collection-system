#include "MetricSystem.h"
#include "MetricUtilities.h"
#include <iostream>
#include <stdexcept>

namespace MetricsSystem {

    // MetricWriter Implementation
    MetricWriter::MetricWriter(const std::string& filename) : output_file_(filename) {
        if (filename.empty()) {
            throw std::invalid_argument("Output filename cannot be empty");
        }

        // Open file for writing (append mode to preserve existing data)
        file_stream_.open(filename, std::ios::out | std::ios::app);
        
        if (!file_stream_.is_open()) {
            throw std::runtime_error("Failed to open output file: " + filename);
        }

        std::cout << "MetricWriter initialized with file: " << filename << std::endl;
    }

    MetricWriter::~MetricWriter() {
        close();
    }

    void MetricWriter::writeMetrics(const std::vector<MetricEntry>& entries) {
        if (entries.empty()) {
            return;
        }

        std::lock_guard<std::mutex> lock(write_mutex_);

        if (!file_stream_.is_open()) {
            throw std::runtime_error("Output file is not open");
        }

        // Group entries by timestamp to write them together
        // Format: timestamp "metric1" value1 "metric2" value2 ...
        
        // For now, write each metric on its own line as specified in the requirements
        for (const auto& entry : entries) {
            try {
                // Format: 2025-06-01 15:00:01.653 "CPU" 0.97 "HTTP requests RPS" 42
                std::string timestamp_str = formatTimestamp(entry.timestamp);
                std::string metric_name = MetricNameValidator::formatNameForOutput(entry.name);
                std::string value_str = entry.value->toString();

                file_stream_ << timestamp_str << " " << metric_name << " " << value_str;

                // Add newline after each metric entry
                file_stream_ << std::endl;

            } catch (const std::exception& e) {
                std::cerr << "Error formatting metric entry '" << entry.name << "': " << e.what() << std::endl;
                continue; // Skip this entry but continue with others
            }
        }

        // Ensure data is written to disk immediately
        file_stream_.flush();
    }

    void MetricWriter::close() {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        if (file_stream_.is_open()) {
            file_stream_.close();
            std::cout << "MetricWriter closed" << std::endl;
        }
    }

    std::string MetricWriter::formatTimestamp(const TimePoint& tp) const {
        return TimestampUtils::formatTimestamp(tp);
    }

    // MetricSystemFactory Implementation
    std::unique_ptr<MetricCollector> MetricSystemFactory::createSystem(const std::string& output_file) {
        auto writer = std::make_unique<MetricWriter>(output_file);
        return std::make_unique<MetricCollector>(std::move(writer));
    }

} // namespace MetricsSystem 