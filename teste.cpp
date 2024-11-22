#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <set>
#include <stack>

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

// Funções para calcular o maior componente fortemente conectado (SCC)
void fillOrder(int v, const std::unordered_map<int, std::vector<int>>& adj, std::stack<int>& Stack, std::unordered_set<int>& visited) {
    visited.insert(v);
    if (adj.find(v) != adj.end()) {
        for (auto& neighbor : adj.at(v)) {
            if (!visited.count(neighbor)) {
                fillOrder(neighbor, adj, Stack, visited);
            }
        }
    }
    Stack.push(v);
}

void dfs(int v, const std::unordered_map<int, std::vector<int>>& adj, std::unordered_set<int>& visited, std::vector<int>& component) {
    visited.insert(v);
    component.push_back(v);
    if (adj.find(v) != adj.end()) {
        for (auto& neighbor : adj.at(v)) {
            if (!visited.count(neighbor)) {
                dfs(neighbor, adj, visited, component);
            }
        }
    }
}

std::unordered_map<int, std::vector<int>> getTranspose(const std::unordered_map<int, std::vector<int>>& adj) {
    std::unordered_map<int, std::vector<int>> transpose;
    for (const auto& [node, neighbors] : adj) {
        for (const auto& neighbor : neighbors) {
            transpose[neighbor].push_back(node);
        }
    }
    return transpose;
}

void findLargestSCC(const std::unordered_map<int, std::vector<int>>& adj, int& largestNodeCount, int& largestEdgeCount) {
    std::stack<int> Stack;
    std::unordered_set<int> visited;

    for (const auto& [node, _] : adj) {
        if (!visited.count(node)) {
            fillOrder(node, adj, Stack, visited);
        }
    }

    auto transpose = getTranspose(adj);

    visited.clear();
    std::vector<int> largestSCC;

    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();
        if (!visited.count(v)) {
            std::vector<int> component;
            dfs(v, transpose, visited, component);
            if (component.size() > largestSCC.size()) {
                largestSCC = component;
            }
        }
    }

    largestNodeCount = largestSCC.size();
    largestEdgeCount = 0;
    std::set<int> largestSCCSet(largestSCC.begin(), largestSCC.end());
    std::set<std::pair<int, int>> countedEdges;

    for (const auto& node : largestSCC) {
        if (adj.find(node) != adj.end()) {
            for (const auto& neighbor : adj.at(node)) {
                if (largestSCCSet.count(neighbor)) {
                    largestEdgeCount++;
                }
            }
        }
    }
    largestEdgeCount /= 2;
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
    std::unordered_map<int, std::vector<int>> adj; // Representação para SCC
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

        // Adiciona arestas para SCC
        adj[u].push_back(v);
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

    std::cout << "Número de nós no maior WCC: " << largestNodeCountWCC << "\n";
    std::cout << "Número de arestas no maior WCC: " << largestEdgeCountWCC << "\n";

    // Cálculo do maior componente fortemente conectado (SCC)
    int largestNodeCountSCC = 0;
    int largestEdgeCountSCC = 0;
    findLargestSCC(adj, largestNodeCountSCC, largestEdgeCountSCC);

    std::cout << "Número de nós no maior SCC: " << largestNodeCountSCC << "\n";
    std::cout << "Número de arestas no maior SCC: " << largestEdgeCountSCC << "\n";

    return 0;
}
