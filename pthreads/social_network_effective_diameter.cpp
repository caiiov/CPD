#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include <random>
#include <numeric>

class SocialNetworkAnalyzer {
private:
    std::unordered_map<int, std::unordered_set<int>> graph;

    //Continuo usando um algoritmo de busca em largura
    std::vector<int> bfsDistances(int startNode, int maxPathLength) {
        std::vector<int> distances(graph.size(), std::numeric_limits<int>::max());
        std::vector<int> reachablePaths;
        std::queue<std::pair<int, int>> q; 
        
        distances[startNode] = 0;
        q.push({startNode, 0});

        while (!q.empty()) {
            auto [current, currentDist] = q.front();
            q.pop();

            if (currentDist > maxPathLength) break; //Se for paralelizar mude para continue, se nao mata o codigo

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

    std::vector<int> sampleNodes(int sampleSize) {
        std::vector<int> nodes;
        nodes.reserve(graph.size());
        for (const auto& entry : graph) {
            nodes.push_back(entry.first);
        }
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(nodes.begin(), nodes.end(), g);
        
        nodes.resize(std::min(sampleSize, static_cast<int>(nodes.size())));
        
        return nodes;
    }

public:
    void loadGraph(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        int from, to;
        while (file >> from >> to) {
            graph[from].insert(to);
            graph[to].insert(from); 
        }
    }

    double calculateEffectiveDiameter() {
        const int SAMPLE_SIZE = 2000;  // Tem como mudar isso aqui a vontade, para poder ver qual da o valor mais proximo e roda melhor
        const int MAX_PATH_LENGTH = 10;  // Mesma coisa
        const double PERCENTILE = 0.9;

        std::vector<int> allDistances;

        std::vector<int> sampledNodes = sampleNodes(SAMPLE_SIZE);
        for (int node : sampledNodes) {
            std::vector<int> nodeDistances = bfsDistances(node, MAX_PATH_LENGTH);
            allDistances.insert(
                allDistances.end(), 
                nodeDistances.begin(), 
                nodeDistances.end()
            );
        }

        if (allDistances.empty()) return 0.0;
        
        std::sort(allDistances.begin(), allDistances.end());
        
        int percentileIndex = static_cast<int>(allDistances.size() * PERCENTILE);
        
        if (percentileIndex >= allDistances.size()) {
            percentileIndex = allDistances.size() - 1;
        }
        
        return static_cast<double>(allDistances[percentileIndex]);
    }

    void printGraphStats() {
        std::cout << "Total Nodes: " << graph.size() << std::endl;
        
        int totalEdges = 0;
        for (const auto& entry : graph) {
            totalEdges += entry.second.size();
        }
        std::cout << "Total Edges: " << totalEdges / 2 << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <facebook_graph.txt>" << std::endl;
        return 1;
    }

    try {
        SocialNetworkAnalyzer analyzer;
        analyzer.loadGraph(argv[1]);
        
        analyzer.printGraphStats();
        
        double effectiveDiameter = analyzer.calculateEffectiveDiameter();
        std::cout << "90-Percentile Effective Diameter: " << effectiveDiameter << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
