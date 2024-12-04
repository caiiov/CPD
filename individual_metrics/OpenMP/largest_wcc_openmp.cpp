#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <omp.h>
#include <mutex>

std::mutex mtx;  

void findLargestWCC(const std::unordered_map<int, std::unordered_set<int>>& graph, 
                    std::unordered_set<int>& visited, 
                    int startNode, 
                    int& nodeCount, 
                    int& edgeCount) {
    std::queue<int> q;
    q.push(startNode);
    
    std::unordered_set<int> localVisited;
    int localNodeCount = 0;
    int localEdgeCount = 0;

    while (!q.empty()) {
        int node = q.front();
        q.pop();

        if (localVisited.count(node)) continue;
        localVisited.insert(node);
        localNodeCount++;

        for (int neighbor : graph.at(node)) {
            localEdgeCount++;
            if (!localVisited.count(neighbor)) {
                q.push(neighbor);
            }
        }
    }

    std::lock_guard<std::mutex> lock(mtx);
    for (int node : localVisited) {
        visited.insert(node);
    }
    nodeCount = localNodeCount;
    edgeCount = localEdgeCount / 2;
}

int main() {
    omp_set_num_threads(omp_get_max_threads());  

    std::string filePath = "facebook_combined.txt";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << "\n";
        return 1;
    }
        double start_time = omp_get_wtime();

    std::unordered_map<int, std::unordered_set<int>> graph;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            graph[u].insert(v);
            graph[v].insert(u);
        }
    }
    file.close();

    std::unordered_set<int> visited;
    int largestNodeCount = 0;
    int largestEdgeCount = 0;

    #pragma omp parallel
    {
        std::vector<int> localNodeCounts;
        std::vector<int> localEdgeCounts;

        #pragma omp for
        for (size_t i = 0; i < graph.size(); ++i) {
            auto it = std::next(graph.begin(), i);
            int node = it->first;

            if (!visited.count(node)) {
                int nodeCount = 0, edgeCount = 0;
                findLargestWCC(graph, visited, node, nodeCount, edgeCount);

                #pragma omp critical
                {
                    if (nodeCount > largestNodeCount) {
                        largestNodeCount = nodeCount;
                        largestEdgeCount = edgeCount;
                    }
                }
            }
        }
    }
    
    double end_time = omp_get_wtime();

	std::cout << "Número de nós no maior WCC: " << largestNodeCount << "\n";
	std::cout << "Número de arestas no maior WCC: " << largestEdgeCount << "\n";
	std::cout << "Computation Time: " << end_time - start_time << " seconds" << std::endl;
    return 0;
}
