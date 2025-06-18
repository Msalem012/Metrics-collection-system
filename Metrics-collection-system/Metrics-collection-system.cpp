// Metrics-collection-system.cpp : Complete metrics collection system demonstration
//

#include "MetricSystemManager.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace MetricsSystem;

// Main demo showing the complete metrics collection system
int main()
{
    std::cout << "=== Metrics Collection System - Complete Demo ===" << std::endl;
    std::cout << "This demo shows CPU and HTTP request metrics as specified." << std::endl << std::endl;

    try {
        // Create metrics system using RAII helper
        ScopedMetricSystem metrics("complete_demo_output.txt");

        // Register the required metrics from the specification
        metrics->registerCPUMetric("CPU");                    // Float values 0.0 to N (cores)
        metrics->registerHTTPMetric("HTTP requests RPS");     // Integer values 0 to INT_MAX

        std::cout << "Metrics system started. Recording sample data..." << std::endl;

        // Simulate the exact examples from the specification
        std::cout << "\nRecording specification examples:" << std::endl;

        // Example 1: 2025-06-01 15:00:01.653 "CPU" 0.97 "HTTP requests RPS" 42
        metrics->recordCPU(0.97);
        metrics->recordHTTPRequests(42);
        std::cout << "Recorded: CPU=0.97, HTTP requests=42" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Example 2: 2025-06-01 15:00:02.653 "CPU" 1.12 "HTTP requests RPS" 30  
        metrics->recordCPU(1.12);
        metrics->recordHTTPRequests(30);
        std::cout << "Recorded: CPU=1.12, HTTP requests=30" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Additional samples to show the system in action
        std::cout << "\nRecording additional samples..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            double cpu_value = 0.5 + (i * 0.3);  // Varying CPU load
            int http_requests = 25 + (i * 10);   // Varying request count

            metrics->recordCPU(cpu_value);
            metrics->recordHTTPRequests(http_requests);

            std::cout << "Sample " << (i + 1) << ": CPU=" << cpu_value 
                      << ", HTTP requests=" << http_requests << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Force final flush
        metrics->flush();

        std::cout << "\nDemo completed successfully!" << std::endl;
        std::cout << "Check 'complete_demo_output.txt' for the results." << std::endl;
        std::cout << "\nExpected output format:" << std::endl;
        std::cout << "timestamp \"metric_name\" value" << std::endl;
        std::cout << "Example: 2025-01-20 14:30:15.123 \"CPU\" 0.97" << std::endl;
        std::cout << "         2025-01-20 14:30:15.123 \"HTTP requests RPS\" 42" << std::endl;

        std::cout << "\nKey features demonstrated:" << std::endl;
        std::cout << "✓ Thread-safe metric recording" << std::endl;
        std::cout << "✓ Non-blocking operation" << std::endl;
        std::cout << "✓ Automatic reset after writing" << std::endl;
        std::cout << "✓ Proper timestamp formatting" << std::endl;
        std::cout << "✓ Extensible metric types" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Demo failed: " << e.what() << std::endl;
        return 1;
    }

    // ScopedMetricSystem automatically stops when going out of scope
    std::cout << "\nMetrics system automatically stopped." << std::endl;
    return 0;
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln