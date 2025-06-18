#include "../MetricSystemManager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <atomic>

using namespace MetricsSystem;

// Multi-threaded demo showing thread-safe metric recording
class MultiThreadedWorkloadSimulator {
private:
    std::shared_ptr<MetricSystemManager> metrics_system_;
    std::atomic<bool> should_stop_;
    std::vector<std::thread> worker_threads_;

    // CPU monitoring thread
    void cpuMonitorThread() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> cpu_dist(0.1, 1.8);
        
        while (!should_stop_) {
            double cpu_load = cpu_dist(gen);
            metrics_system_->recordCPU(cpu_load);
            
            std::cout << "[CPU Thread] Recorded CPU: " << cpu_load << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }

    // HTTP request simulation thread
    void httpRequestThread(int thread_id) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> request_dist(5, 25);
        
        while (!should_stop_) {
            int requests = request_dist(gen);
            metrics_system_->recordHTTPRequests(requests);
            
            std::cout << "[HTTP Thread " << thread_id << "] Recorded requests: " << requests << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
    }

    // Memory monitoring thread
    void memoryMonitorThread() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> mem_dist(100.0, 512.0);
        
        while (!should_stop_) {
            double memory_mb = mem_dist(gen);
            metrics_system_->recordMemoryUsage(memory_mb);
            
            std::cout << "[Memory Thread] Recorded memory: " << memory_mb << " MB" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }
    }

    // Network traffic simulation thread
    void networkTrafficThread() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<long> bytes_dist(1024, 10485760); // 1KB to 10MB
        
        while (!should_stop_) {
            long bytes_transferred = bytes_dist(gen);
            metrics_system_->recordNetworkBytes(bytes_transferred);
            
            std::cout << "[Network Thread] Recorded bytes: " << bytes_transferred << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(900));
        }
    }

public:
    explicit MultiThreadedWorkloadSimulator(std::shared_ptr<MetricSystemManager> system)
        : metrics_system_(system), should_stop_(false) {}

    void start() {
        std::cout << "Starting multi-threaded workload simulation..." << std::endl;
        
        // Start different types of worker threads
        worker_threads_.emplace_back(&MultiThreadedWorkloadSimulator::cpuMonitorThread, this);
        worker_threads_.emplace_back(&MultiThreadedWorkloadSimulator::httpRequestThread, this, 1);
        worker_threads_.emplace_back(&MultiThreadedWorkloadSimulator::httpRequestThread, this, 2);
        worker_threads_.emplace_back(&MultiThreadedWorkloadSimulator::memoryMonitorThread, this);
        worker_threads_.emplace_back(&MultiThreadedWorkloadSimulator::networkTrafficThread, this);
        
        std::cout << "Started " << worker_threads_.size() << " worker threads" << std::endl;
    }

    void stop() {
        std::cout << "Stopping worker threads..." << std::endl;
        should_stop_ = true;
        
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        worker_threads_.clear();
        std::cout << "All worker threads stopped" << std::endl;
    }
};

int main() {
    std::cout << "=== Multi-Threaded Metrics Collection Demo ===" << std::endl;

    try {
        // Create metrics system
        auto metricsSystem = std::shared_ptr<MetricSystemManager>(
            MetricSystemManager::create("multithreaded_metrics_output.txt").release()
        );

        // Register various metrics
        metricsSystem->registerCPUMetric("CPU");
        metricsSystem->registerHTTPMetric("HTTP requests RPS");
        metricsSystem->registerMemoryMetric("Memory Usage MB");
        metricsSystem->registerNetworkMetric("Network Bytes/sec");

        // Start metrics collection
        metricsSystem->start();

        // Create and start workload simulator
        MultiThreadedWorkloadSimulator simulator(metricsSystem);
        simulator.start();

        // Run simulation for 15 seconds
        std::cout << "\nRunning simulation for 15 seconds..." << std::endl;
        std::cout << "Multiple threads are recording metrics simultaneously..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(15));

        // Stop simulation
        simulator.stop();

        // Final flush and stop
        metricsSystem->flush();
        metricsSystem->stop();

        std::cout << "\nMulti-threaded demo completed!" << std::endl;
        std::cout << "Check 'multithreaded_metrics_output.txt' for results." << std::endl;
        std::cout << "This demonstrates thread-safe, non-blocking metric recording." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Multi-threaded demo failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 