#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <filesystem>
#include <vector>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace fs = std::filesystem;

void fillOrder(int v, std::unordered_map<int, std::vector<int>>& adj, std::stack<int>& Stack, std::unordered_map<int, bool>& visited) {
    visited[v] = true;
    for (auto& neighbor : adj[v]) {
        if (!visited[neighbor]) {
            fillOrder(neighbor, adj, Stack, visited);
        }
    }
    Stack.push(v);
}

void dfs(int v, std::unordered_map<int, std::vector<int>>& adj, std::unordered_map<int, bool>& visited, std::vector<int>& component) {
    visited[v] = true;
    component.push_back(v);
    for (auto& neighbor : adj[v]) {
        if (!visited[neighbor]) {
            dfs(neighbor, adj, visited, component);
        }
    }
}

std::unordered_map<int, std::vector<int>> getTranspose(std::unordered_map<int, std::vector<int>>& adj) {
    std::unordered_map<int, std::vector<int>> transpose;
    for (auto& pair : adj) {
        for (auto& neighbor : pair.second) {
            transpose[neighbor].push_back(pair.first);
        }
    }
    return transpose;
}

std::pair<int, int> normalizeEdge(int u, int v) {
    return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
}

int main() {
    try {
        std::string directory = "./facebook";
        std::unordered_map<int, std::vector<int>> adj;
        std::set<std::pair<int, int>> uniqueEdges;
        std::unordered_set<int> allNodes; // Todos os nós
        std::vector<std::vector<int>> SCCs;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".edges" || entry.path().filename() == "facebook_combined.txt") {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    std::cerr << "Não foi possível abrir o arquivo: " << entry.path().string() << "\n";
                    continue;
                }

                std::string line;
                while (std::getline(file, line)) {
                    std::istringstream iss(line);
                    int u, v;
                    if (!(iss >> u >> v)) {
                        std::cerr << "Linha inválida no arquivo " << entry.path().filename() << ": " << line << "\n";
                        continue;
                    }

                    adj[u].push_back(v);
                    adj[v].push_back(u);
                    uniqueEdges.insert(normalizeEdge(u, v));
                    allNodes.insert(u);
                    allNodes.insert(v);
                }

                file.close();
            }
        }

        // Garante que todos os nós estão no grafo, mesmo que estejam isolados
        for (int node : allNodes) {
            if (adj.find(node) == adj.end()) {
                adj[node] = {}; // Adiciona nó isolado
            }
        }

        // Passo 1: Ordenação topológica
        std::stack<int> Stack;
        std::unordered_map<int, bool> visited;

        for (auto& pair : adj) {
            if (!visited[pair.first]) {
                fillOrder(pair.first, adj, Stack, visited);
            }
        }

        // Passo 2: Transpor o grafo
        std::unordered_map<int, std::vector<int>> transpose = getTranspose(adj);

        // Passo 3: Encontrar SCCs no grafo transposto
        visited.clear();
        while (!Stack.empty()) {
            int v = Stack.top();
            Stack.pop();
            if (!visited[v]) {
                std::vector<int> component;
                dfs(v, transpose, visited, component);
                SCCs.push_back(component);
            }
        }

        // Identificar o maior SCC
        std::vector<int> largestSCC;
        for (const auto& component : SCCs) {
            if (component.size() > largestSCC.size()) {
                largestSCC = component;
            }
        }

        // Contar as arestas no maior SCC
        int edgeCount = 0;
        std::set<int> largestSCCSet(largestSCC.begin(), largestSCC.end());
        std::set<std::pair<int, int>> countedEdges;

        for (const auto& node : largestSCC) {
            for (const auto& neighbor : adj[node]) {
                if (largestSCCSet.count(neighbor) && !countedEdges.count(normalizeEdge(node, neighbor))) {
                    edgeCount++;
                    countedEdges.insert(normalizeEdge(node, neighbor));
                }
            }
        }

        // Calculando a proporção
        double edgeRatio = static_cast<double>(edgeCount) / uniqueEdges.size();

        // Exibir resultado
        std::cout << "Total de arestas no maior SCC: " << edgeCount << " (" << edgeRatio << ")\n";

    } catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
