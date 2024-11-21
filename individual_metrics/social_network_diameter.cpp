#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <climits>
#include <algorithm>
#include <string>

class SocialNetwork {
private:
    // Adjacency list representation of the graph
    std::unordered_map<int, std::vector<int>> graph;
    int num_nodes;

    // Breadth-First Search to find shortest paths from a given source
    std::vector<int> breadthFirstSearch(int source) {
        std::vector<int> distances(num_nodes + 1, INT_MAX);
        std::queue<int> queue;
        
        distances[source] = 0;
        queue.push(source);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();

            // Explore all neighbors
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

    // Load graph from text file with social network connections
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
                continue; // Skip invalid lines
            }

            // Bidirectional connection
            graph[user1].push_back(user2);
            graph[user2].push_back(user1);

            // Track unique nodes
            unique_nodes[user1] = true;
            unique_nodes[user2] = true;
        }

        num_nodes = unique_nodes.size();
        return true;
    }

    // Calculate network diameter using intelligent sampling and BFS
    int calculateDiameter() {
        if (graph.empty()) {
            std::cerr << "Error: Empty graph" << std::endl;
            return -1;
        }

        int max_diameter = 0;
        int effective_samples = 0;

        // Intelligent node selection strategy
        std::vector<int> seed_nodes;
        for (const auto& entry : graph) {
            // Prioritize nodes with high connectivity
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

        // Diameter calculation using efficient sampling
        for (int source : seed_nodes) {
            // Find distances from current source
            std::vector<int> distances = breadthFirstSearch(source);
            
            int max_distance = 0;
            for (int dist : distances) {
                if (dist != INT_MAX) {
                    max_distance = std::max(max_distance, dist);
                }
            }
            
            // Update diameter
            max_diameter = std::max(max_diameter, max_distance);
            effective_samples++;

            // Early termination if we have a good estimate
            if (effective_samples >= 5 && max_diameter > 0) {
                break;
            }
        }

        return max_diameter;
    }

    // Network statistics
    int getNodeCount() const { return num_nodes; }
    int getEdgeCount() const {
        int total_edges = 0;
        for (const auto& node : graph) {
            total_edges += node.second.size();
        }
        return total_edges / 2; // Each edge counted twice
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <social_network_file>" << std::endl;
        return 1;
    }

    SocialNetwork network;
    
    // Load graph from file
    if (!network.loadGraphFromFile(argv[1])) {
        return 1;
    }

    // Display network insights
    std::cout << "Network Analysis Report:" << std::endl;
    std::cout << "---------------------" << std::endl;
    std::cout << "Total Nodes: " << network.getNodeCount() << std::endl;
    std::cout << "Total Connections: " << network.getEdgeCount() << std::endl;

    // Calculate and display network diameter
    int diameter = network.calculateDiameter();
    
    if (diameter != -1) {
        std::cout << "Network Diameter: " << diameter << std::endl;
    }

    return 0;
}
