#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <queue>
#include <sstream>
#include <cstdlib>
#include <fstream>

std::mutex queue_mutex;
std::mutex output_mutex;
std::queue<std::string> tasks;

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

        // Execute the command
        std::ostringstream output_stream;
        output_stream << command << " > temp_output.log 2>&1";
        int ret_code = std::system(output_stream.str().c_str());

        // Collect output
        std::ifstream output_file("temp_output.log");
        std::ostringstream file_content;
        file_content << output_file.rdbuf();

        // Notify task completion
        {
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "[Thread " << std::this_thread::get_id() << "] Concluída tarefa: " << command << std::endl;
            std::cout << "Saída:\n" << file_content.str() << std::endl;
        }
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

    std::cout << "Todas as tarefas foram concluídas!" << std::endl;
    return 0;
}
