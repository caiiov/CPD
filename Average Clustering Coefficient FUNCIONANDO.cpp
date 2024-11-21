#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm> // Para std::max_element

using namespace std;

// Função para calcular o coeficiente de agrupamento de um nó
double clusteringCoefficient(int node, const vector<vector<int>>& graph) {
    if (node >= graph.size()) {
        cerr << "Erro: Nó fora do intervalo (" << node << ")." << endl;
        return 0.0;
    }

    const vector<int>& neighbors = graph[node];
    int degree = neighbors.size();

    if (degree < 2) return 0.0; // Sem triângulos possíveis

    int triangles = 0;
    for (size_t i = 0; i < neighbors.size(); i++) {
        for (size_t j = i + 1; j < neighbors.size(); j++) {
            int neighbor1 = neighbors[i];
            int neighbor2 = neighbors[j];

            // Verifica se neighbor1 e neighbor2 estão conectados
            if (find(graph[neighbor1].begin(), graph[neighbor1].end(), neighbor2) != graph[neighbor1].end()) {
                triangles++;
            }
        }
    }

    return (2.0 * triangles) / (degree * (degree - 1));
}

// Função para ler o arquivo "facebook_combined.txt" e construir o grafo
void readGraphFile(const string& filePath, vector<vector<int>>& graph) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << filePath << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        int u, v;
        ss >> u >> v;

        // Ajusta o tamanho do grafo dinamicamente
        while (graph.size() <= max(u, v)) {
            graph.emplace_back();
        }

        // Adiciona as arestas
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    file.close();
}

// Função principal
int main() {
    vector<vector<int>> graph;

    string filePath = "facebook_combined.txt";

    // Lê o arquivo "facebook_combined.txt"
    cout << "Lendo arquivo: " << filePath << endl;
    readGraphFile(filePath, graph);

    // Calcula o coeficiente de agrupamento médio
    double totalClustering = 0.0;
    int nodeCount = 0;

    for (size_t i = 0; i < graph.size(); i++) {
        if (!graph[i].empty()) {
            totalClustering += clusteringCoefficient(i, graph);
            nodeCount++;
        }
    }

    if (nodeCount > 0) {
        double averageClustering = totalClustering / nodeCount;
        cout << "Coeficiente de agrupamento médio: " << averageClustering << endl;
    } else {
        cout << "Nenhum nó encontrado no grafo." << endl;
    }

    return 0;
}

