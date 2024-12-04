#include <iostream>
#include <fstream>
#include <set>
#include <vector>  
#include <utility> 
#include <sstream>
#include <omp.h>

int main() {
    std::string filePath = "facebook_combined.txt";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << "\n";
        return 1;
    }
    
    double start_time = omp_get_wtime();

    // prepara para usar o openmp
    std::vector<std::pair<int, int>> edges;
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int u, v;
        if (iss >> u >> v) {
            edges.push_back({u, v});
        }
    }
    file.close();

    std::set<int> nodes;
    
    #pragma omp parallel
    {
        std::set<int> local_nodes;
        
        #pragma omp for nowait schedule(static)
        for (int i = 0; i < edges.size(); ++i) {
            local_nodes.insert(edges[i].first);
            local_nodes.insert(edges[i].second);
        }
        
        #pragma omp critical
        {
            nodes.insert(local_nodes.begin(), local_nodes.end());
        }
    }

    double end_time = omp_get_wtime();
    
    std::cout << "Número total de nós (Nodes): " << nodes.size() << "\n";
    std::cout << "Linhas processadas: " << edges.size() << "\n";
    std::cout << "Computation Time: " << end_time - start_time << " seconds" << std::endl;

    return 0;
}
