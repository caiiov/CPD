#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>

// Função para calcular o maior componente fracamente conectado (WCC)
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
    std::string filePath = "facebook_combined.txt"; // Caminho do arquivo no mesmo diretório
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << "\n";
        return 1;
    }

    std::set<int> nodes; // Conjunto para armazenar nós únicos
    std::unordered_map<int, std::unordered_set<int>> graph; // Representação do grafo
    std::string line;
    int lineCount = 0;

    // Processamento do arquivo e construção do grafo
    while (std::getline(file, line)) {
        lineCount++;
        std::istringstream iss(line);
        int u, v;

        if (!(iss >> u >> v)) {
            std::cerr << "Linha inválida no arquivo: " << line << "\n";
            continue; // Ignora linhas inválidas
        }

        // Adiciona os nós ao conjunto
        nodes.insert(u);
        nodes.insert(v);

        // Adiciona arestas ao grafo
        graph[u].insert(v);
        graph[v].insert(u); // Grafo não direcionado
    }
    file.close();

    // Exibe o número total de nós e arestas
    std::cout << "Número total de nós (Nodes): " << nodes.size() << "\n";
    std::cout << "Número total de arestas (Edges): " << lineCount << "\n";

    // Cálculo do maior componente fracamente conectado (WCC)
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

    // Exibe o resultado do maior WCC
    std::cout << "Número de nós no maior WCC: " << largestNodeCount << "\n";
    std::cout << "Número de arestas no maior WCC: " << largestEdgeCount << "\n";

    return 0;
}
