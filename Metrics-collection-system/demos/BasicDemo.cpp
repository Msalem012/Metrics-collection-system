#include "../MetricSystemManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

using namespace MetricsSystem;

// Basic demo showing CPU and HTTP metrics as specified in requirements
int main() {
    std::cout << "=== Basic Metrics Collection Demo ===" << std::endl;

    try {
        // Create metrics system with output file
        auto metricsSystem = MetricSystemManager::create("basic_metrics_output.txt");

        // Register the required metrics from specifications
        metricsSystem->registerCPUMetric("CPU");                    // CPU utilization (double, 0 to N cores)
        metricsSystem->registerHTTPMetric("HTTP requests RPS");     // HTTP requests per second (int, 0 to INT_MAX)

        // Start the system
        metricsSystem->start();
        std::cout << "Metrics collection system started..." << std::endl;

        // Simulate application workload for 10 seconds
        std::cout << "Simulating application workload..." << std::endl;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> cpu_dist(0.0, 2.0);  // 0 to 2.0 (for 2-core system)
        std::uniform_int_distribution<int> http_dist(20, 60);       // 20-60 requests per second

        for (int i = 0; i < 10; ++i) {
            // Generate sample CPU utilization (0.0 = no load, 2.0 = 100% on 2 cores)
            double cpu_utilization = cpu_dist(gen);
            
            // Generate sample HTTP request count
            int http_requests = http_dist(gen);

            // Record metrics (non-blocking calls)
            metricsSystem->recordCPU(cpu_utilization);
            metricsSystem->recordHTTPRequests(http_requests);

            std::cout << "Recorded: CPU=" << cpu_utilization 
                      << ", HTTP requests=" << http_requests << std::endl;

            // Wait 1 second before next measurement
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Force immediate flush of any remaining data
        std::cout << "Flushing final metrics..." << std::endl;
        metricsSystem->flush();

        // Stop the system
        std::cout << "Stopping metrics system..." << std::endl;
        metricsSystem->stop();

        std::cout << "\nDemo completed! Check 'basic_metrics_output.txt' for results." << std::endl;
        std::cout << "Expected format: timestamp \"metric_name\" value" << std::endl;
        std::cout << "Example: 2025-06-01 15:00:01.653 \"CPU\" 0.97 \"HTTP requests RPS\" 42" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Demo failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 