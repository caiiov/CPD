#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>

// Função para encontrar o maior componente conectado (BFS)
void findLargestWCC(const std::unordered_map<int, std::unordered_set<int>>& graph, 
                    std::unordered_set<int>& visited, 
                    int startNode, 
                    int& nodeCount, 
                    int& edgeCount) {
    std::queue<int> q;
    q.push(startNode);
    visited.insert(startNode);

    int localNodeCount = 0;
    int localEdgeCount = 0;

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        localNodeCount++;

        for (int neighbor : graph.at(node)) {
            localEdgeCount++;
            if (!visited.count(neighbor)) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    nodeCount = localNodeCount;
    edgeCount = localEdgeCount / 2; // Cada aresta é contada duas vezes
}

int main() {
    std::string filePath = "facebook_combined.txt"; // Caminho do arquivo
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << "\n";
        return 1;
    }

    std::unordered_map<int, std::unordered_set<int>> graph;
    std::string line;

    // Construção do grafo a partir do arquivo
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            graph[u].insert(v);
            graph[v].insert(u); // Grafo não direcionado
        }
    }
    file.close();

    // Encontrar o maior componente fracamente conectado
    std::unordered_set<int> visited;
    int largestNodeCount = 0;
    int largestEdgeCount = 0;

    for (const auto& [node, _] : graph) {
        if (!visited.count(node)) {
            int nodeCount = 0;
            int edgeCount = 0;
            findLargestWCC(graph, visited, node, nodeCount, edgeCount);
            if (nodeCount > largestNodeCount) {
                largestNodeCount = nodeCount;
                largestEdgeCount = edgeCount;
            }
        }
    }

    // Exibir resultados
    std::cout << "Número de nós no maior WCC: " << largestNodeCount << "\n";
    std::cout << "Número de arestas no maior WCC: " << largestEdgeCount << "\n";

    return 0;
}
