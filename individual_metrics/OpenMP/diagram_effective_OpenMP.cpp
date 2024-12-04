#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <limits>
#include <algorithm>
#include <string>
#include <omp.h>
#include <random>
#include <numeric>

class ParallelSocialNetworkDiameter {
private:
    std::unordered_map<int, std::unordered_set<int>> graph;
    int num_nodes;

    //Continuo usando um algoritmo de busca em largura
    std::vector<int> bfsEffectiveDiameterDistances(int startNode, int maxPathLength) {
        std::vector<int> distances(num_nodes + 1, std::numeric_limits<int>::max());
        std::vector<int> reachablePaths;
        std::queue<std::pair<int, int>> q;  
        
        distances[startNode] = 0;
        q.push({startNode, 0});

        while (!q.empty()) {
            auto [current, currentDist] = q.front();
            q.pop();

            if (currentDist > maxPathLength) break; //Nao usou paralelizacao aqui, entao nao morreu

            for (int neighbor : graph[current]) {
                if (distances[neighbor] == std::numeric_limits<int>::max()) {
                    int newDist = currentDist + 1;
                    distances[neighbor] = newDist;
                    
                    if (newDist > 0 && newDist <= maxPathLength) {
                        reachablePaths.push_back(newDist);
                    }
                    
                    q.push({neighbor, newDist});
                }
            }
        }

        return reachablePaths;
    }

    // Aqui ta usando sample nodes, pra otimizar, mas nao vai dar 100% do resultado

    std::vector<int> selectSeedNodes(int sample_size) {
        std::vector<std::pair<int, int>> node_degrees;
        
        for (const auto& entry : graph) {
            node_degrees.emplace_back(entry.first, entry.second.size());
        }

        std::sort(node_degrees.begin(), node_degrees.end(), 
            [](const auto& a, const auto& b) { 
                return a.second > b.second; 
            }
        );

        std::vector<int> seed_nodes;
        std::random_device rd;
        std::mt19937 gen(rd());

        int top_degree_count = sample_size / 3;
        int middle_degree_count = sample_size / 3;
        int random_count = sample_size - top_degree_count - middle_degree_count;

        for (int i = 0; i < top_degree_count && i < node_degrees.size(); ++i) {
            seed_nodes.push_back(node_degrees[i].first);
        }

        int middle_start = std::max(0, static_cast<int>(node_degrees.size() / 3));
        int middle_end = std::min(static_cast<int>(node_degrees.size() * 2 / 3), 
                                   static_cast<int>(node_degrees.size()));
        for (int i = middle_start; 
             i < middle_end && seed_nodes.size() < top_degree_count + middle_degree_count; 
             ++i) {
            seed_nodes.push_back(node_degrees[i].first);
        }

        std::unordered_set<int> used_nodes(seed_nodes.begin(), seed_nodes.end());
        while (seed_nodes.size() < sample_size) {
            int random_node = std::next(graph.begin(), 
                std::uniform_int_distribution<>(0, graph.size() - 1)(gen))->first;
            
            if (used_nodes.find(random_node) == used_nodes.end()) {
                seed_nodes.push_back(random_node);
                used_nodes.insert(random_node);
            }
        }

        return seed_nodes;
    }

public:
    ParallelSocialNetworkDiameter() : num_nodes(0) {
        omp_set_num_threads(omp_get_max_threads());
    }
    // tem segredo nao, só rodar o arquivo de texto
    bool loadGraphFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file " << filename << std::endl;
            return false;
        }

        std::string line;
        std::unordered_map<int, bool> unique_nodes;

        #pragma omp parallel
        {
            std::unordered_map<int, std::unordered_set<int>> local_graph;
            std::unordered_map<int, bool> local_unique_nodes;

            #pragma omp critical
            {
                while (std::getline(file, line)) {
                    std::istringstream iss(line);
                    int user1, user2;
                    
                    if (!(iss >> user1 >> user2)) {
                        continue; 
                    }

                    local_graph[user1].insert(user2);
                    local_graph[user2].insert(user1);

                    local_unique_nodes[user1] = true;
                    local_unique_nodes[user2] = true;
                }
            }

            #pragma omp critical
            {
                for (const auto& entry : local_graph) {
                    graph[entry.first].insert(
                        entry.second.begin(), 
                        entry.second.end()
                    );
                }

                unique_nodes.insert(local_unique_nodes.begin(), local_unique_nodes.end());
            }
        }

        num_nodes = unique_nodes.size();
        return true;
    }
    // Basicamente é o diametro onde 90% dos pares de nós estão a uma distancia menor que o valor retornado
    double calculateEffectiveDiameter(double percentile = 0.9, int max_path_length = 10) {
        if (graph.empty()) {
            std::cerr << "Error: Empty graph" << std::endl;
            return -1;
        }

        const int SAMPLE_SIZE = 2000;
        std::vector<int> seed_nodes = selectSeedNodes(SAMPLE_SIZE);
        std::vector<int> allDistances;

        #pragma omp parallel
        {
            std::vector<int> local_distances;

            #pragma omp for
            for (size_t i = 0; i < seed_nodes.size(); ++i) {
                int source = seed_nodes[i];
                
                std::vector<int> distances = bfsEffectiveDiameterDistances(source, max_path_length);
                
                #pragma omp critical
                {
                    local_distances.insert(
                        local_distances.end(), 
                        distances.begin(), 
                        distances.end()
                    );
                }
            }

            #pragma omp critical
            {
                allDistances.insert(
                    allDistances.end(), 
                    local_distances.begin(), 
                    local_distances.end()
                );
            }
        }

        if (allDistances.empty()) return 0.0;
        
        std::sort(allDistances.begin(), allDistances.end());
        
        int percentileIndex = static_cast<int>(allDistances.size() * percentile);
        if (percentileIndex >= allDistances.size()) {
            percentileIndex = allDistances.size() - 1;
        }
        
        return static_cast<double>(allDistances[percentileIndex]);
    }

    int getNodeCount() const { return num_nodes; }
    
    int getEdgeCount() const {
        int total_edges = 0;
        
        #pragma omp parallel reduction(+:total_edges)
        {
            for (const auto& node : graph) {
                total_edges += node.second.size();
            }
        }

        return total_edges / 2; 
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <social_network_file>" << std::endl;
        return 1;
    }

    double start_time = omp_get_wtime();

    ParallelSocialNetworkDiameter network;
    
    if (!network.loadGraphFromFile(argv[1])) {
        return 1;
    }

    std::cout << "Network Analysis Report:" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "Total Nodes: " << network.getNodeCount() << std::endl;
    std::cout << "Total Connections: " << network.getEdgeCount() << std::endl;

    double effective_diameter = network.calculateEffectiveDiameter(0.9, 10);
    
    double end_time = omp_get_wtime();

    if (effective_diameter != -1) {
        std::cout << "90-Percentile Effective Diameter: " << effective_diameter << std::endl;
        std::cout << "Computation Time: " << end_time - start_time << " seconds" << std::endl;
    }

    return 0;
}