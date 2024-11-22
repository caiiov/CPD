#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <climits>
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

// Função para contar o número de triângulos no grafo
int countTriangles(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    int triangleCount = 0;

    for (const auto& [node, neighbors] : graph) {
        for (auto it1 = neighbors.begin(); it1 != neighbors.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != neighbors.end(); ++it2) {
                if (graph.at(*it1).count(*it2)) {
                    triangleCount++;
                }
            }
        }
    }

    return triangleCount / 3; // Cada triângulo é contado três vezes
}

// Função para contar o número de triângulos fechados
int countClosedTriangles(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    int triangleCount = 0;

    for (const auto& [node, neighbors] : graph) {
        for (auto it1 = neighbors.begin(); it1 != neighbors.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != neighbors.end(); ++it2) {
                if (graph.at(*it1).count(*it2)) {
                    // Verifica se o triângulo é fechado
                    if (graph.at(node).count(*it1) && graph.at(node).count(*it2)) {
                        triangleCount++;
                    }
                }
            }
        }
    }

    return triangleCount / 3; // Cada triângulo fechado é contado três vezes
}

// Função para contar o número de triângulos possíveis (não fechados)
int countPossibleTriangles(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    int possibleTriangleCount = 0;

    for (const auto& [node, neighbors] : graph) {
        for (auto it1 = neighbors.begin(); it1 != neighbors.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != neighbors.end(); ++it2) {
                if (graph.at(*it1).count(*it2) || graph.at(node).count(*it1) || graph.at(node).count(*it2)) {
                    possibleTriangleCount++;
                }
            }
        }
    }

    return possibleTriangleCount / 3; // Cada triângulo possível é contado três vezes
}

// Função para calcular a fração de triângulos fechados
double fractionOfClosedTriangles(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    int numClosedTriangles = countClosedTriangles(graph);
    int numPossibleTriangles = countPossibleTriangles(graph);

    if (numPossibleTriangles == 0) return 0.0;
    return static_cast<double>(numClosedTriangles) / numPossibleTriangles;
}

// Função para calcular o diâmetro do grafo
int calculateDiameter(const std::unordered_map<int, std::unordered_set<int>>& graph) {
    int diameter = 0;

    // Função BFS para encontrar a maior distância entre dois nós
    auto bfs = [&graph](int start) {
        std::unordered_map<int, int> distances;
        std::queue<int> q;
        q.push(start);
        distances[start] = 0;

        while (!q.empty()) {
            int node = q.front();
            q.pop();

            for (int neighbor : graph.at(node)) {
                if (distances.find(neighbor) == distances.end()) {
                    distances[neighbor] = distances[node] + 1;
                    q.push(neighbor);
                }
            }
        }
        return distances;
    };

    // Para cada nó, calcule a distância máxima (diâmetro)
    for (const auto& [node, _] : graph) {
        auto distances = bfs(node);
        int maxDistance = 0;
        for (const auto& [_, dist] : distances) {
            maxDistance = std::max(maxDistance, dist);
        }
        diameter = std::max(diameter, maxDistance);
    }

    return diameter;
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

    // Contagem do número de triângulos
    int triangleCount = countTriangles(graph);

    // Cálculo da fração de triângulos fechados
    double closedTriangleFraction = fractionOfClosedTriangles(graph);

    // Cálculo do diâmetro do grafo
    int diameter = calculateDiameter(graph);

    // Exibição dos resultados
    std::cout << "Maior componente fracamente conectado (WCC) - Nós: " << largestNodeCountWCC << ", Arestas: " << largestEdgeCountWCC << "\n";
    std::cout << "Maior componente fortemente conectado (SCC) - Nós: " << largestNodeCountSCC << ", Arestas: " << largestEdgeCountSCC << "\n";
    std::cout << "Coeficiente de agrupamento médio: " << averageClustering << "\n";
    std::cout << "Número total de triângulos: " << triangleCount << "\n";
    std::cout << "Fração de triângulos fechados: " << closedTriangleFraction << "\n";
    std::cout << "Diâmetro do grafo: " << diameter << "\n";

    return 0;
}
