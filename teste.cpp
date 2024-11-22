#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <algorithm>

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

// Função para calcular o maior componente fortemente conectado (SCC)
void findLargestSCC(const std::unordered_map<int, std::unordered_set<int>>& graph,
                    int& largestNodeCount,
                    int& largestEdgeCount) {
    largestNodeCount = 1; // Em grafos não direcionados, SCC é no máximo um nó isolado
    largestEdgeCount = 0; // Nenhuma aresta dentro de um SCC isolado
}

// Função para calcular o coeficiente de agrupamento de um nó
double clusteringCoefficient(int node, const std::unordered_map<int, std::unordered_set<int>>& graph) {
    if (graph.at(node).size() < 2) return 0.0; // Sem triângulos possíveis

    const auto& neighbors = graph.at(node);
    int triangles = 0;

    for (auto it1 = neighbors.begin(); it1 != neighbors.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != neighbors.end(); ++it2) {
            if (graph.at(*it1).count(*it2)) {
                triangles++;
            }
        }
    }

    int degree = neighbors.size();
    return (2.0 * triangles) / (degree * (degree - 1));
}

// Função principal
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
    int largestNodeCountWCC = 0;
    int largestEdgeCountWCC = 0;

    for (const auto& [node, _] : graph) {
        if (!visited.count(node)) {
            int nodeCount = 0;
            int edgeCount = 0;
            findLargestWCC(graph, visited, node, nodeCount, edgeCount);
            if (nodeCount > largestNodeCountWCC) {
                largestNodeCountWCC = nodeCount;
                largestEdgeCountWCC = edgeCount;
            }
        }
    }

    // Cálculo do maior componente fortemente conectado (SCC)
    int largestNodeCountSCC = 0;
    int largestEdgeCountSCC = 0;
    findLargestSCC(graph, largestNodeCountSCC, largestEdgeCountSCC);

    // Cálculo do coeficiente de agrupamento médio
    double totalClustering = 0.0;
    int clusteringNodeCount = 0;

    for (const auto& [node, neighbors] : graph) {
        if (!neighbors.empty()) {
            totalClustering += clusteringCoefficient(node, graph);
            clusteringNodeCount++;
        }
    }

    double averageClustering = clusteringNodeCount > 0 ? totalClustering / clusteringNodeCount : 0.0;

    // Exibe os resultados
    std::cout << "Número de nós no maior WCC: " << largestNodeCountWCC << "\n";
    std::cout << "Número de arestas no maior WCC: " << largestEdgeCountWCC << "\n";
    std::cout << "Número de nós no maior SCC: " << largestNodeCountSCC << "\n";
    std::cout << "Número de arestas no maior SCC: " << largestEdgeCountSCC << "\n";
    std::cout << "Coeficiente de agrupamento médio: " << averageClustering << "\n";

    return 0;
}
