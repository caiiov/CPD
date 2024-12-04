#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <omp.h>

using namespace std;

double clusteringCoefficient(int node, const vector<vector<int>>& graph) {
    if (node >= graph.size()) {
        cerr << "Erro: Nó fora do intervalo (" << node << ")." << endl;
        return 0.0;
    }

    const vector<int>& neighbors = graph[node];
    int degree = neighbors.size();

    if (degree < 2) return 0.0;

    int triangles = 0;
    #pragma omp parallel for reduction(+:triangles) shared(neighbors, graph)
    for (size_t i = 0; i < neighbors.size(); i++) {
        for (size_t j = i + 1; j < neighbors.size(); j++) {
            int neighbor1 = neighbors[i];
            int neighbor2 = neighbors[j];

            if (find(graph[neighbor1].begin(), graph[neighbor1].end(), neighbor2) != graph[neighbor1].end()) {
                triangles++;
            }
        }
    }

    return (2.0 * triangles) / (degree * (degree - 1));
}

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

        while (graph.size() <= max(u, v)) {
            graph.emplace_back();
        }

        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    file.close();
}

int main() {
    omp_set_num_threads(omp_get_max_threads());  // Use maximum available threads

    vector<vector<int>> graph;
    string filePath = "facebook_combined.txt";

    cout << "Lendo arquivo: " << filePath << endl;
    readGraphFile(filePath, graph);
	
    double start_time = omp_get_wtime();	
	
    double totalClustering = 0.0;
    int nodeCount = 0;

    #pragma omp parallel for reduction(+:totalClustering, nodeCount) shared(graph)
    for (size_t i = 0; i < graph.size(); i++) {
        if (!graph[i].empty()) {
            totalClustering += clusteringCoefficient(i, graph);
            nodeCount++;
        }
    }
	
    double end_time = omp_get_wtime();
	
    if (nodeCount > 0) {
        double averageClustering = totalClustering / nodeCount;
        cout << "Coeficiente de agrupamento médio: " << averageClustering << endl;
        cout << "Computation Time: " << end_time - start_time << " seconds" << endl;
    } else {
        cout << "Nenhum nó encontrado no grafo." << endl;
    }

    return 0;
}
