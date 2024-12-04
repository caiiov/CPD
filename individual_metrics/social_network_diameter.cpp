#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <climits>
#include <algorithm>
#include <string>
#include <chrono>

class SocialNetwork {
private:
    // Lista de adjacência 
    std::unordered_map<int, std::vector<int>> graph;
    int num_nodes;

    // Calcula a distância de todos os nós a partir de um nó de origem
    std::vector<int> breadthFirstSearch(int source) {
        std::vector<int> distances(num_nodes + 1, INT_MAX);
        std::queue<int> queue;
        
        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            // Para garantir que vai explorar todos os nós
            for (int neighbor : graph[current]) {
                if (distances[neighbor] == INT_MAX) {
                    distances[neighbor] = distances[current] + 1;
                    queue.push(neighbor);
                }
            }
        }

        return distances;
    }

public:
    SocialNetwork() : num_nodes(0) {}

    bool loadGraphFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }

        std::string line;
        std::unordered_map<int, bool> unique_nodes;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            int user1, user2;
            
            if (!(iss >> user1 >> user2)) {
                continue; // Isso aq deu mt problema, mas é para pular as linhas inválidas (Nao use Break)
            }

            graph[user1].push_back(user2);
            graph[user2].push_back(user1);

            unique_nodes[user1] = true;
            unique_nodes[user2] = true;
        }

        num_nodes = unique_nodes.size();
        return true;
    }

    // Usando Sampling e BFS 
    int calculateDiameter() {
        if (graph.empty()) {
            std::cerr << "Error: Empty graph" << std::endl;
            return -1;
        }

        int max_diameter = 0;
        int effective_samples = 0;

        // Seleciona os nós com maior conectividade e prioriza eles
        std::vector<int> seed_nodes;
        for (const auto& entry : graph) {
            if (seed_nodes.empty() || 
                entry.second.size() > graph[seed_nodes[0]].size()) {
                if (seed_nodes.empty()) {
                    seed_nodes.push_back(entry.first);
                } else {
                    seed_nodes[0] = entry.first;
                }
            }
            
            if (seed_nodes.size() < 10) {
                seed_nodes.push_back(entry.first);
            }
        }

        // Aqui calcula de verdade o diâmetro
        for (int source : seed_nodes) {
            std::vector<int> distances = breadthFirstSearch(source);
            
            int max_distance = 0;
            for (int dist : distances) {
                if (dist != INT_MAX) {
                    max_distance = std::max(max_distance, dist);
                }
            }
            
            max_diameter = std::max(max_diameter, max_distance);
            effective_samples++;

            // Se encontrar uma amostra boa já para o código aqui
            // Isso está aqui para rodar o do google melhor, mas ainda assim ta dando core bump
            if (effective_samples >= 5 && max_diameter > 0) {
                break;
            }
        }

        return max_diameter;
    }

    int getNodeCount() const { return num_nodes; }
    int getEdgeCount() const {
        int total_edges = 0;
        for (const auto& node : graph) {
            total_edges += node.second.size();
        }
        return total_edges / 2;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <social_network_file>" << std::endl;
        return 1;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    SocialNetwork network;
    
    if (!network.loadGraphFromFile(argv[1])) {
        return 1;
    }

    std::cout << "Network Analysis Report:" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "Total Nodes: " << network.getNodeCount() << std::endl;
    std::cout << "Total Connections: " << network.getEdgeCount() << std::endl;

    int diameter = network.calculateDiameter();
    
    auto end_time = std::chrono::high_resolution_clock::now();


    if (diameter != -1) {
        std::cout << "Network Diameter: " << diameter << std::endl;
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        std::cout << "Computation Time: " << duration.count() << " seconds" << std::endl;
    }

    return 0;
}
