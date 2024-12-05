#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Função para contar o número de triângulos em um grafo
int countTriangles(const vector<vector<int>>& adjMatrix, int n) {
    int triangleCount = 0;
    
    // Iterar sobre todos os trios de nós
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                // Verificar se os três nós estão conectados entre si
                if (adjMatrix[i][j] == 1 && adjMatrix[j][k] == 1 && adjMatrix[k][i] == 1) {
                    triangleCount++;
                }
            }
        }
    }
    
    return triangleCount;
}

int main() {
    ifstream inputFile("facebook_combined.txt"); // Nome do arquivo com as arestas
    if (!inputFile.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    vector<pair<int, int>> edges;
    int maxNode = 0;
    
    // Ler todas as arestas do arquivo e encontrar o maior nó
    int u, v;
    while (inputFile >> u >> v) {
        edges.push_back({u, v});
        maxNode = max(maxNode, max(u, v)); // Encontrar o maior nó
    }

    inputFile.close();
    
    // O número de nós é o maior nó + 1 (pois os índices começam em 0)
    int n = maxNode + 1;

    // Criar a matriz de adjacência
    vector<vector<int>> adjMatrix(n, vector<int>(n, 0));
    
    // Preencher a matriz de adjacência com base nas arestas
    for (const auto& edge : edges) {
        int u = edge.first;
        int v = edge.second;
        adjMatrix[u][v] = 1;
        adjMatrix[v][u] = 1; // Grafo não direcionado
    }

    // Contar e exibir o número de triângulos
    int numTriangles = countTriangles(adjMatrix, n);
    cout << "Número de triângulos no grafo: " << numTriangles << endl;

    return 0;
}

