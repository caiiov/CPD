#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <omp.h>

using namespace std;

int countTriangles(const vector<vector<int>>& adjMatrix, int n) {
    int triangleCount = 0;
    
    #pragma omp parallel for reduction(+:triangleCount) shared(adjMatrix) 
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                if (adjMatrix[i][j] == 1 && adjMatrix[j][k] == 1 && adjMatrix[k][i] == 1) {
                    triangleCount++;
                }
            }
        }
    }
    
    return triangleCount;
}

int countPossibleTriangles(const vector<vector<int>>& adjMatrix, int n) {
    int possibleTriangleCount = 0;
    
    #pragma omp parallel for reduction(+:possibleTriangleCount) shared(adjMatrix)
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                int edges = adjMatrix[i][j] + adjMatrix[j][k] + adjMatrix[k][i];
                if (edges >= 2) {
                    possibleTriangleCount++;
                }
            }
        }
    }
    
    return possibleTriangleCount;
}

int main() {
    omp_set_num_threads(omp_get_max_threads());  

    ifstream inputFile("facebook_combined.txt");
    if (!inputFile.is_open()) {
        cerr << "Erro ao abrir o arquivo!" << endl;
        return 1;
    }

    double start_time = omp_get_wtime();

    vector<pair<int, int>> edges;
    int maxNode = 0;
    
    int u, v;
    while (inputFile >> u >> v) {
        edges.push_back({u, v});
        maxNode = max(maxNode, max(u, v));
    }

    inputFile.close();
    
    int n = maxNode + 1;

    vector<vector<int>> adjMatrix(n, vector<int>(n, 0));
    
    #pragma omp parallel for
    for (size_t idx = 0; idx < edges.size(); ++idx) {
        int u = edges[idx].first;
        int v = edges[idx].second;
        #pragma omp critical
        {
            adjMatrix[u][v] = 1;
            adjMatrix[v][u] = 1;
        }
    }

    int numTriangles = countTriangles(adjMatrix, n);
    int numPossibleTriangles = countPossibleTriangles(adjMatrix, n);

    double closedTriangleFraction = 0.0;
    if (numPossibleTriangles > 0) {
        closedTriangleFraction = static_cast<double>(numTriangles) / numPossibleTriangles;
    }
    
    double end_time = omp_get_wtime();

    cout << "Fração de triângulos fechados: " << closedTriangleFraction << endl;
    cout << "Computation Time: " << end_time - start_time << " seconds" << endl;

    return 0;
}
