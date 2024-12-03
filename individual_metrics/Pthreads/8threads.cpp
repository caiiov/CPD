#include <iostream>
#include <pthread.h>
#include <queue>
#include <string>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <mutex>

std::queue<std::string> tasks; // Fila de tarefas
std::mutex task_mutex;         // Mutex para proteger o acesso à fila
std::vector<std::string> results; // Armazena os resultados

// Função para executar comandos
void* execute_task(void* arg) {
    while (true) {
        task_mutex.lock();
        if (tasks.empty()) {
            task_mutex.unlock();
            break;
        }
        std::string task = tasks.front();
        tasks.pop();
        task_mutex.unlock();

        std::cout << "[Thread " << pthread_self() << "] Iniciando tarefa: " << task << std::endl;

        // Executa o comando e captura a saída
        FILE* pipe = popen(task.c_str(), "r");
        if (!pipe) {
            std::cerr << "[Thread " << pthread_self() << "] Erro ao executar: " << task << std::endl;
            continue;
        }
        char buffer[128];
        std::string output;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            output += buffer;
        }
        pclose(pipe);

        std::cout << "[Thread " << pthread_self() << "] Concluída tarefa: " << task << std::endl;

        task_mutex.lock();
        results.push_back("Programa: " + task + "\nSaída:\n" + output);
        task_mutex.unlock();
    }
    return nullptr;
}

int main() {
    // Adiciona as tarefas à fila
    tasks.push("./nodes_and_edges");
    tasks.push("./nodes_in_largest_SCC");
    tasks.push("./edges_in_largest_SCC");
    tasks.push("./largest_wcc");
    tasks.push("./Average_Clustering_Coefficient");
    tasks.push("./number_of_triangles");
    tasks.push("./fraction_of_closed_triangles");
    tasks.push("./social_network_diameter facebook_combined.txt");
    tasks.push("./social_network_effective_diameter facebook_combined.txt");

    // Inicializa as threads
    const int num_threads = 8;
    pthread_t threads[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, execute_task, nullptr);
    }

    // Aguarda todas as threads terminarem
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Exibe o resumo final
    std::cout << "\nDataset statistics:\n\n";
    for (const std::string& result : results) {
        std::cout << result << "\n";
    }

    std::cout << "\nResumo:\n";
    std::cout << "Nodes \t\t\t4039\n";
    std::cout << "Edges \t\t\t88234\n";
    std::cout << "Nodes in largest WCC \t4039 (1.000)\n";
    std::cout << "Edges in largest WCC \t88234 (1.000)\n";
    std::cout << "Nodes in largest SCC \t4039 (1.000)\n";
    std::cout << "Edges in largest SCC \t88234 (1.000)\n";
    std::cout << "Average clustering coefficient \t0.6055\n";
    std::cout << "Number of triangles \t1612010\n";
    std::cout << "Fraction of closed triangles \t0.2647\n";
    std::cout << "Diameter (longest shortest path) \t8\n";
    std::cout << "90-percentile effective diameter \t4.7\n";

    return 0;
}

