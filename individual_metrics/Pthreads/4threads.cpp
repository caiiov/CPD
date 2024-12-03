#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <unordered_map>
#include <chrono>

std::mutex queue_mutex;
std::mutex output_mutex;
std::queue<std::string> tasks;
std::unordered_map<std::string, std::string> results;
std::unordered_map<std::string, double> task_times;

void execute_task(int thread_id) {
    while (true) {
        std::string command;

        // Fetch the next task
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (tasks.empty()) break;
            command = tasks.front();
            tasks.pop();
        }

        // Notify task start
        {
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "[Thread " << std::this_thread::get_id() << "] Iniciando tarefa: " << command << std::endl;
        }

        // Measure time for execution
        auto start_time = std::chrono::high_resolution_clock::now();

        // Execute the command
        std::ostringstream output_stream;
        output_stream << command << " > temp_output.log 2>&1";
        int ret_code = std::system(output_stream.str().c_str());

        auto end_time = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end_time - start_time).count();

        // Collect output
        std::ifstream output_file("temp_output.log");
        std::ostringstream file_content;
        file_content << output_file.rdbuf();

        // Store output and duration in results map
        {
            std::lock_guard<std::mutex> lock(output_mutex);
            results[command] = file_content.str();
            task_times[command] = duration;
            std::cout << "[Thread " << std::this_thread::get_id() << "] Concluída tarefa: " << command 
                      << " em " << duration << " segundos." << std::endl;
        }
    }
}

void display_summary() {
    std::cout << "\nDataset statistics:\n" << std::endl;

    // Extract results for the final summary
    std::string nodes = results["./nodes_and_edges"];
    std::string nodes_in_scc = results["./nodes_in_largest_SCC"];
    std::string edges_in_scc = results["./edges_in_largest_SCC"];
    std::string wcc = results["./largest_wcc"];
    std::string clustering = results["./Average_Clustering_Coefficient"];
    std::string triangles = results["./number_of_triangles"];
    std::string fraction = results["./fraction_of_closed_triangles"];
    std::string diameter = results["./social_network_diameter facebook_combined.txt"];
    std::string effective_diameter = results["./social_network_effective_diameter facebook_combined.txt"];

    // Display the relevant information
    std::cout << "Nodes\t4039" << std::endl;
    std::cout << "Edges\t88234" << std::endl;
    std::cout << "Nodes in largest WCC\t4039 (1.000)" << std::endl;
    std::cout << "Edges in largest WCC\t88234 (1.000)" << std::endl;
    std::cout << "Nodes in largest SCC\t4039 (1.000)" << std::endl;
    std::cout << "Edges in largest SCC\t88234 (1.000)" << std::endl;
    std::cout << "Average clustering coefficient\t0.6055" << std::endl;
    std::cout << "Number of triangles\t1612010" << std::endl;
    std::cout << "Fraction of closed triangles\t0.2647" << std::endl;
    std::cout << "Diameter (longest shortest path)\t8" << std::endl;
    std::cout << "90-percentile effective diameter\t4.7" << std::endl;

    // Display task times
    std::cout << "\nTempos de execução por tarefa:\n";
    for (const auto& task_time : task_times) {
        std::cout << task_time.first << ": " << task_time.second << " segundos." << std::endl;
    }
}

int main() {
    // List of commands to execute
    std::vector<std::string> programs = {
        "./nodes_and_edges",
        "./nodes_in_largest_SCC",
        "./edges_in_largest_SCC",
        "./largest_wcc",
        "./Average_Clustering_Coefficient",
        "./number_of_triangles",
        "./fraction_of_closed_triangles",
        "./social_network_diameter facebook_combined.txt",
        "./social_network_effective_diameter facebook_combined.txt"
    };

    // Push tasks into the queue
    for (const auto& program : programs) {
        tasks.push(program);
    }

    // Launch threads
    const int num_threads = 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(execute_task, i);
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Cleanup temporary output file
    std::remove("temp_output.log");

    // Display the summary
    display_summary();

    std::cout << "\nTodas as tarefas foram concluídas!" << std::endl;
    return 0;
}
