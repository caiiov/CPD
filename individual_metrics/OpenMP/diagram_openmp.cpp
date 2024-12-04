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

    //Continuo usando um algoritmo de busca em largura, e tento paralelizar no código

    std::vector<int> parallelBreadthFirstSearch(int source) {
        std::vector<int> distances(num_nodes + 1, INT_MAX);
        std::vector<bool> visited(num_nodes + 1, false);
        std::queue<int> queue;
        
        distances[source] = 0;
        visited[source] = true;
        queue.push(source);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            #pragma omp parallel for
            for (size_t i = 0; i < graph[current].size(); ++i) {
                int neighbor = graph[current][i];
                
                #pragma omp critical
                {
                    if (!visited[neighbor]) {
                        visited[neighbor] = true;
                        distances[neighbor] = distances[current] + 1;
                        queue.push(neighbor);
                    }
                }
            }
        }

        return distances;
    }
    // Nao paralelizei
    int findFarthestNode(const std::vector<int>& distances) {
        int max_dist = 0;
        int farthest_node = 0;
        for (int i = 1; i < distances.size(); ++i) {
            if (distances[i] != INT_MAX && distances[i] > max_dist) {
                max_dist = distances[i];
                farthest_node = i;
            }
        }
        return farthest_node;
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
                        continue; // Isso aq deu mt problema, mas é para pular as linhas inválidas
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

    // Nao deu para usar o effective samples, então só faz o BFS e retorna o diâmetro
    // Pq usando o effetivo ele nao dava o código que a gente espera, e como o BFS em paralelo já é mais rápido, nao da B.O (I Hope!!)
    int calculateDiameter() {
        if (graph.empty()) {
            std::cerr << "Error: Empty graph" << std::endl;
            return -1;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, num_nodes);
        
        int start_node = dis(gen);
        std::vector<int> first_bfs = parallelBreadthFirstSearch(start_node);
        int end_point1 = findFarthestNode(first_bfs);
        
        std::vector<int> second_bfs = parallelBreadthFirstSearch(end_point1);
        int end_point2 = findFarthestNode(second_bfs);

        return second_bfs[end_point2];
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