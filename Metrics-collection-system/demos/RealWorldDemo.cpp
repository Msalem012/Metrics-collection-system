#include "../MetricSystemManager.h"
#include "../SpecificMetrics.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>

using namespace MetricsSystem;

// Simulated web server with realistic load patterns
class WebServerSimulator {
private:
    std::shared_ptr<MetricSystemManager> metrics_system_;
    std::atomic<bool> running_;
    std::atomic<int> active_connections_;
    std::atomic<int> total_requests_;
    
    // Request queue simulation
    std::queue<int> request_queue_;
    std::mutex queue_mutex_;

    // Simulate varying CPU load based on request processing
    void cpuLoadSimulation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        while (running_) {
            // Base CPU load + load from active connections
            double base_load = 0.1; // 10% base system load
            double connection_load = active_connections_.load() * 0.05; // 5% per connection
            double random_spike = 0.0;
            
            // Occasional CPU spikes (10% chance)
            std::uniform_real_distribution<double> spike_chance(0.0, 1.0);
            if (spike_chance(gen) < 0.1) {
                std::uniform_real_distribution<double> spike_intensity(0.3, 0.8);
                random_spike = spike_intensity(gen);
                std::cout << "[SERVER] CPU spike detected!" << std::endl;
            }
            
            double total_cpu = std::min(2.0, base_load + connection_load + random_spike);
            metrics_system_->recordCPU(total_cpu);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    // Simulate incoming HTTP requests with realistic patterns
    void requestGenerationSimulation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        // Time-based request patterns (simulating daily traffic)
        int time_step = 0;
        
        while (running_) {
            // Simulate daily traffic pattern (peak hours, off-peak, etc.)
            double time_factor = 1.0 + 0.5 * sin(time_step * 0.1); // Sine wave pattern
            
            // Base requests per second with time variation
            std::uniform_int_distribution<int> base_requests(10, 30);
            int requests_this_second = static_cast<int>(base_requests(gen) * time_factor);
            
            // Occasional traffic bursts (5% chance)
            std::uniform_real_distribution<double> burst_chance(0.0, 1.0);
            if (burst_chance(gen) < 0.05) {
                std::uniform_int_distribution<int> burst_requests(50, 100);
                requests_this_second += burst_requests(gen);
                std::cout << "[SERVER] Traffic burst! " << requests_this_second << " requests" << std::endl;
            }
            
            // Record HTTP requests
            metrics_system_->recordHTTPRequests(requests_this_second);
            total_requests_ += requests_this_second;
            
            // Simulate connection handling
            active_connections_ = std::min(100, requests_this_second / 2);
            
            time_step++;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    // Simulate memory usage patterns
    void memoryUsageSimulation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> mem_variation(-10.0, 15.0);
        
        double base_memory = 150.0; // Base memory usage in MB
        double current_memory = base_memory;
        
        while (running_) {
            // Memory usage correlates with active connections
            double connection_memory = active_connections_.load() * 2.0; // 2MB per connection
            
            // Gradual memory changes with occasional spikes
            current_memory += mem_variation(gen);
            current_memory = std::max(100.0, std::min(800.0, current_memory)); // Keep in reasonable bounds
            
            double total_memory = current_memory + connection_memory;
            metrics_system_->recordMemoryUsage(total_memory);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }

    // Simulate network throughput
    void networkTrafficSimulation() {
        std::random_device rd;
        std::mt19937 gen(rd());
        
        while (running_) {
            // Network traffic correlates with HTTP requests
            int recent_requests = active_connections_.load() * 10; // Approximation
            
            // Bytes per request (response size)
            std::uniform_int_distribution<int> response_size(1024, 8192); // 1-8KB responses
            long bytes_per_second = recent_requests * response_size(gen);
            
            metrics_system_->recordNetworkBytes(bytes_per_second);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
    }

public:
    explicit WebServerSimulator(std::shared_ptr<MetricSystemManager> system)
        : metrics_system_(system), running_(false), active_connections_(0), total_requests_(0) {}

    void start() {
        std::cout << "Starting web server simulation..." << std::endl;
        running_ = true;
        
        // Start simulation threads
        std::thread cpu_thread(&WebServerSimulator::cpuLoadSimulation, this);
        std::thread request_thread(&WebServerSimulator::requestGenerationSimulation, this);
        std::thread memory_thread(&WebServerSimulator::memoryUsageSimulation, this);
        std::thread network_thread(&WebServerSimulator::networkTrafficSimulation, this);
        
        // Run for 20 seconds
        std::this_thread::sleep_for(std::chrono::seconds(20));
        
        std::cout << "\nStopping simulation..." << std::endl;
        running_ = false;
        
        // Wait for threads to finish
        cpu_thread.join();
        request_thread.join();
        memory_thread.join();
        network_thread.join();
        
        std::cout << "Simulation completed. Total requests processed: " << total_requests_.load() << std::endl;
    }
};

int main() {
    std::cout << "=== Real-World Web Server Metrics Demo ===" << std::endl;
    std::cout << "Simulating a web server with realistic load patterns..." << std::endl;

    try {
        // Create metrics system
        auto metricsSystem = std::shared_ptr<MetricSystemManager>(
            MetricSystemManager::create("webserver_metrics_output.txt").release()
        );

        // Register web server metrics
        metricsSystem->registerCPUMetric("CPU");
        metricsSystem->registerHTTPMetric("HTTP requests RPS");
        metricsSystem->registerMemoryMetric("Memory Usage MB");
        metricsSystem->registerNetworkMetric("Network Bytes/sec");

        std::cout << "Registered metrics: CPU, HTTP requests, Memory, Network" << std::endl;

        // Start metrics collection
        metricsSystem->start();

        // Run web server simulation
        WebServerSimulator webServer(metricsSystem);
        webServer.start();

        // Final cleanup
        metricsSystem->flush();
        metricsSystem->stop();

        std::cout << "\nReal-world demo completed!" << std::endl;
        std::cout << "Check 'webserver_metrics_output.txt' for realistic metrics data." << std::endl;
        std::cout << "\nThis demo shows:" << std::endl;
        std::cout << "- Correlated metrics (CPU load vs connections)" << std::endl;
        std::cout << "- Time-based patterns (traffic variations)" << std::endl;
        std::cout << "- Realistic value ranges and spikes" << std::endl;
        std::cout << "- Production-like usage scenarios" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Real-world demo failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 