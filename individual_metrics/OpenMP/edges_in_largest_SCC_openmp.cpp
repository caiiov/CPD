#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <omp.h>

std::pair<int, int> normalizeEdge(int u, int v) {
    return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
}

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

int main() {
	omp_set_num_threads(omp_get_max_threads()); 
    try {
        std::ifstream file("facebook_combined.txt");
        if (!file.is_open()) {
            std::cerr << "Não foi possível abrir o arquivo facebook_combined.txt\n";
            return 1;
        }
          double start_time = omp_get_wtime();

        std::unordered_map<int, std::vector<int>> adj;
        std::unordered_set<int> allNodes;
        std::set<std::pair<int, int>> uniqueEdges;
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            int u, v;
            if (!(iss >> u >> v)) {
                std::cerr << "Linha inválida no arquivo: " << line << "\n";
                continue;
            }

            // Construir grafo não direcionado
            adj[u].push_back(v);
            adj[v].push_back(u);
            uniqueEdges.insert(normalizeEdge(u, v));
            allNodes.insert(u);
            allNodes.insert(v);
        }
        file.close();

        // Garantir que todos os nós estejam no grafo
        for (int node : allNodes) {
            if (adj.find(node) == adj.end()) {
                adj[node] = {};
            }
        }

        // Ordenação topológica
        std::stack<int> Stack;
        std::unordered_map<int, bool> visited;

        for (auto& pair : adj) {
            if (!visited[pair.first]) {
                fillOrder(pair.first, adj, Stack, visited);
            }
        }

        // Transpor o grafo
        std::unordered_map<int, std::vector<int>> transpose = getTranspose(adj);

        // Encontrar SCCs
        visited.clear();
        std::vector<std::vector<int>> SCCs;

        while (!Stack.empty()) {
            int v = Stack.top();
            Stack.pop();
            if (!visited[v]) {
                std::vector<int> component;
                dfs(v, transpose, visited, component);
                SCCs.push_back(component);
            }
        }

        // Maior SCC
        std::vector<int> largestSCC;
        for (const auto& component : SCCs) {
            if (component.size() > largestSCC.size()) {
                largestSCC = component;
            }
        }

        // Contar arestas no maior SCC
        int edgeCount = 0;
        std::set<int> largestSCCSet(largestSCC.begin(), largestSCC.end());
        std::set<std::pair<int, int>> countedEdges;

               #pragma omp parallel for reduction(+:edgeCount)
                for (const auto& node : largestSCC) {
                    for (const auto& neighbor : adj[node]) {
                        if (largestSCCSet.count(neighbor)) {
                            #pragma omp critical
                            {
                                if (!countedEdges.count(normalizeEdge(node, neighbor))) {
                                    edgeCount++;
                                    countedEdges.insert(normalizeEdge(node, neighbor));
                                }
                            }
                        }
                    }
                }

        double edgeRatio = static_cast<double>(edgeCount) / uniqueEdges.size();
	 
	 double end_time = omp_get_wtime();
	    
        // Exibir resultado
        std::cout << "Total de arestas no maior SCC: " << edgeCount << " (" << edgeRatio << ")\n";
        std::cout << "Computation Time: " << end_time - start_time << " seconds" << std::endl;

    } catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
