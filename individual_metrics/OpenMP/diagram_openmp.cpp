#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <climits>
#include <algorithm>
#include <string>
#include <omp.h>
#include <random>
#include <unordered_set>

class ParallelSocialNetwork {
private:
    std::unordered_map<int, std::vector<int>> graph;
    int num_nodes;

    std::vector<int> parallelBreadthFirstSearch(int source) {
        std::vector<int> distances(num_nodes + 1, INT_MAX);
        std::queue<int> queue;
        
        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            #pragma omp parallel for shared(distances, queue)
            for (size_t i = 0; i < graph[current].size(); ++i) {
                int neighbor = graph[current][i];
                
                #pragma omp critical
                {
                    if (distances[neighbor] == INT_MAX) {
                        distances[neighbor] = distances[current] + 1;
                        queue.push(neighbor);
                    }
                }
            }
        }

        return distances;
    }

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

        int top_count = std::min(sample_size / 2, static_cast<int>(node_degrees.size()));
        for (int i = 0; i < top_count; ++i) {
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
    ParallelSocialNetwork() : num_nodes(0) {
        omp_set_num_threads(omp_get_max_threads());
    }

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
            std::unordered_map<int, std::vector<int>> local_graph;
            std::unordered_map<int, bool> local_unique_nodes;

            #pragma omp critical
            {
                while (std::getline(file, line)) {
                    std::istringstream iss(line);
                    int user1, user2;
                    
                    if (!(iss >> user1 >> user2)) {
                        continue; // Skip invalid lines
                    }

                    local_graph[user1].push_back(user2);
                    local_graph[user2].push_back(user1);

                    local_unique_nodes[user1] = true;
                    local_unique_nodes[user2] = true;
                }
            }

            #pragma omp critical
            {
                for (const auto& entry : local_graph) {
                    graph[entry.first].insert(
                        graph[entry.first].end(), 
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

    int calculateDiameter(int sample_count = 10) {
        if (graph.empty()) {
            std::cerr << "Error: Empty graph" << std::endl;
            return -1;
        }

        int max_diameter = 0;
        
        std::vector<int> seed_nodes = selectSeedNodes(sample_count);

        #pragma omp parallel
        {
            int local_max_diameter = 0;

            #pragma omp for
            for (size_t i = 0; i < seed_nodes.size(); ++i) {
                int source = seed_nodes[i];
                
                std::vector<int> distances = parallelBreadthFirstSearch(source);
                
                int max_distance = 0;
                for (int dist : distances) {
                    if (dist != INT_MAX) {
                        max_distance = std::max(max_distance, dist);
                    }
                }
                
                #pragma omp critical
                {
                    local_max_diameter = std::max(local_max_diameter, max_distance);
                }
            }

            #pragma omp critical
            {
                max_diameter = std::max(max_diameter, local_max_diameter);
            }
        }

        return max_diameter;
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

    ParallelSocialNetwork network;
    
    if (!network.loadGraphFromFile(argv[1])) {
        return 1;
    }

    std::cout << "Network Analysis Report:" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "Total Nodes: " << network.getNodeCount() << std::endl;
    std::cout << "Total Connections: " << network.getEdgeCount() << std::endl;

    int diameter = network.calculateDiameter();
    
    double end_time = omp_get_wtime();

    if (diameter != -1) {
        std::cout << "Network Diameter: " << diameter << std::endl;
        std::cout << "Computation Time: " << end_time - start_time << " seconds" << std::endl;
    }

    return 0;
}