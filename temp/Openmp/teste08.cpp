#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>
#include <stack>
#include <queue>
#include <iomanip> 
#include "json.hpp"
#include <random>
#include <cmath> 
#include <sys/resource.h> 
#include <sys/time.h> 
#include <unistd.h> 
#include <chrono> 
#include <omp.h> 
using json = nlohmann::json;

struct ComponentInfo {
    size_t numeroNos;
    size_t numeroArestas;
};


int main() {

    std::ifstream arquivo("web-Google.txt");
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo 'web-Google.txt'. Verifique se o arquivo existe e o caminho está correto." << std::endl;
        return 1;
    }

    std::unordered_set<long long> nos; 
    size_t numeroArestasTotal = 0;
    std::vector<std::pair<long long, long long>> arestas_direcionadas; 

    std::string linha;
    while (std::getline(arquivo, linha)) {
        
        if (linha.empty() || linha[0] == '#') continue;

        std::istringstream iss(linha);
        long long no1, no2;
        if (!(iss >> no1 >> no2)) {
            
            continue;
        }

        
        if (no1 == no2) continue;

        nos.insert(no1);
        nos.insert(no2);
        arestas_direcionadas.emplace_back(no1, no2);
        numeroArestasTotal++;
    }

    arquivo.close();

    size_t numeroNosTotal = nos.size();

 
    std::unordered_map<long long, int> mapaNos;
    int indice = 0;
    for (const auto &no : nos) {
        mapaNos[no] = indice++;
    }

    std::vector<std::vector<int>> adjList_undirected(numeroNosTotal, std::vector<int>());
    std::vector<std::vector<int>> adjList_direcionado(numeroNosTotal, std::vector<int>());
    std::vector<std::vector<int>> adjList_transposto(numeroNosTotal, std::vector<int>());

    for (const auto &aresta : arestas_direcionadas) {
        int idxU = mapaNos[aresta.first];
        int idxV = mapaNos[aresta.second];
        adjList_undirected[idxU].push_back(idxV);
        adjList_undirected[idxV].push_back(idxU);
        adjList_direcionado[idxU].push_back(idxV);
        adjList_transposto[idxV].push_back(idxU);
    }

    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < numeroNosTotal; ++i) {
        std::sort(adjList_undirected[i].begin(), adjList_undirected[i].end());


        adjList_undirected[i].erase(std::unique(adjList_undirected[i].begin(), adjList_undirected[i].end()), adjList_undirected[i].end());

        adjList_undirected[i].erase(std::remove(adjList_undirected[i].begin(), adjList_undirected[i].end(), i), adjList_undirected[i].end());
    }

    std::vector<bool> visitado_wcc(numeroNosTotal, false);
    ComponentInfo maiorWCC = {0, 0};
    std::vector<int> maiorWCC_nodes; 

    auto dfs_iterativo_wcc = [&](int start_node, std::vector<int> &component_nodes) -> ComponentInfo {
        ComponentInfo wcc = {0, 0};
        std::stack<int> pilha;
        pilha.push(start_node);
        visitado_wcc[start_node] = true;
        component_nodes.push_back(start_node);

        while (!pilha.empty()) {
            int node = pilha.top();
            pilha.pop();
            wcc.numeroNos++;

            for (const auto &neighbor : adjList_undirected[node]) {
                wcc.numeroArestas++;
                if (!visitado_wcc[neighbor]) {
                    visitado_wcc[neighbor] = true;
                    pilha.push(neighbor);
                    component_nodes.push_back(neighbor);
                }
            }
        }

        wcc.numeroArestas /= 2;

        return wcc;
    };

    for (int i = 0; i < numeroNosTotal; ++i) {
        if (!visitado_wcc[i]) {
            std::vector<int> current_component_nodes;
            ComponentInfo currentWCC = dfs_iterativo_wcc(i, current_component_nodes);
            if (currentWCC.numeroNos > maiorWCC.numeroNos) {
                maiorWCC = currentWCC;
                maiorWCC_nodes = current_component_nodes;
            }
        }
    }

 
    std::vector<bool> visitado_kosaraju(numeroNosTotal, false);
    std::vector<int> ordem_finalizacao;

 
    auto dfs_iterativo_kosaraju = [&](int start_node, std::vector<int> &ordem) {
        std::stack<std::pair<int, bool>> pilha; 
        pilha.emplace(start_node, false);

        while (!pilha.empty()) {
            auto [node, is_processed] = pilha.top();
            pilha.pop();

            if (is_processed) {
                ordem.push_back(node);
                continue;
            }

            if (visitado_kosaraju[node]) {
                continue;
            }

            visitado_kosaraju[node] = true;
            pilha.emplace(node, true);

            for (const auto &neighbor : adjList_direcionado[node]) {
                if (!visitado_kosaraju[neighbor]) {
                    pilha.emplace(neighbor, false);
                }
            }
        }
    };

    for (int i = 0; i < numeroNosTotal; ++i) {
        if (!visitado_kosaraju[i]) {
            dfs_iterativo_kosaraju(i, ordem_finalizacao);
        }
    }

    
    std::fill(visitado_kosaraju.begin(), visitado_kosaraju.end(), false);
    std::vector<int> scc_ids(numeroNosTotal, -1); 
    int current_scc_id = 0;
    std::vector<ComponentInfo> scc_infos; 

    auto dfs_iterativo_transposto = [&](int start_node, int scc_id, ComponentInfo &scc_info) {
        std::stack<int> pilha;
        pilha.push(start_node);
        visitado_kosaraju[start_node] = true;
        scc_info.numeroNos = 0;

        while (!pilha.empty()) {
            int node = pilha.top();
            pilha.pop();
            scc_info.numeroNos++;
            scc_ids[node] = scc_id;

            for (const auto &neighbor : adjList_transposto[node]) {
                if (!visitado_kosaraju[neighbor]) {
                    visitado_kosaraju[neighbor] = true;
                    pilha.push(neighbor);
                }
            }
        }
    };

    for (auto it = ordem_finalizacao.rbegin(); it != ordem_finalizacao.rend(); ++it) {
        int node = *it;
        if (!visitado_kosaraju[node]) {
            ComponentInfo scc_info = {0, 0};
            dfs_iterativo_transposto(node, current_scc_id, scc_info);
            scc_infos.push_back(scc_info);
            current_scc_id++;
        }
    }

    size_t maiorSCC_id = 0;
    size_t maiorSCC_size = 0;
    for (size_t i = 0; i < scc_infos.size(); ++i) {
        if (scc_infos[i].numeroNos > maiorSCC_size) {
            maiorSCC_size = scc_infos[i].numeroNos;
            maiorSCC_id = i;
        }
    }

    size_t maiorSCC_arestas = 0;
    #pragma omp parallel for reduction(+:maiorSCC_arestas) schedule(dynamic)
    for (size_t i = 0; i < arestas_direcionadas.size(); ++i) {
        int idxU = mapaNos[arestas_direcionadas[i].first];
        int idxV = mapaNos[arestas_direcionadas[i].second];
        if (scc_ids[idxU] == maiorSCC_id && scc_ids[idxV] == maiorSCC_id) {
            maiorSCC_arestas++;
        }
    }

    scc_infos[maiorSCC_id].numeroArestas = maiorSCC_arestas;

    double somaClustering = 0.0;

    #pragma omp parallel for reduction(+:somaClustering) schedule(dynamic)
    for (size_t i = 0; i < adjList_undirected.size(); ++i) {
        size_t grau = adjList_undirected[i].size();

        size_t totalPossiveis = grau * (grau - 1) / 2;
        size_t triangles = 0;
        double C_u = 0.0;

        if (totalPossiveis > 0) {
            for (size_t j = 0; j < adjList_undirected[i].size(); ++j) {
                for (size_t k = j + 1; k < adjList_undirected[i].size(); ++k) {
                    int neighbor1 = adjList_undirected[i][j];
                    int neighbor2 = adjList_undirected[i][k];
                    if (std::binary_search(adjList_undirected[neighbor1].begin(), adjList_undirected[neighbor1].end(), neighbor2)) {
                        triangles++;
                    }
                }
            }
            C_u = static_cast<double>(triangles) / static_cast<double>(totalPossiveis);
        } else {
            C_u = 0.0;
        }

        somaClustering += C_u;
    }

    double averageClusteringCoefficient = somaClustering / static_cast<double>(numeroNosTotal);

    size_t totalTriangulos = 0;
    size_t totalTrios = 0;

    #pragma omp parallel for reduction(+:totalTriangulos, totalTrios) schedule(dynamic)
    for (size_t i = 0; i < adjList_undirected.size(); ++i) {
        size_t grau = adjList_undirected[i].size();

        if (grau < 2) continue;

        size_t localTriosNode = grau * (grau - 1) / 2;
        totalTrios += localTriosNode;

        size_t localTriangulos = 0;
        for (size_t j = 0; j < adjList_undirected[i].size(); ++j) {
            for (size_t k = j + 1; k < adjList_undirected[i].size(); ++k) {
                int neighbor1 = adjList_undirected[i][j];
                int neighbor2 = adjList_undirected[i][k];
                if (std::binary_search(adjList_undirected[neighbor1].begin(), adjList_undirected[neighbor1].end(), neighbor2)) {
                    localTriangulos++;
                }
            }
        }
        totalTriangulos += localTriangulos;
    }

    totalTriangulos /= 3;


    double fractionClosedTriangles = 0.0;
    if (totalTrios > 0) {
        fractionClosedTriangles = static_cast<double>(totalTriangulos * 3) / static_cast<double>(totalTrios);
    }

    std::vector<int> nodes_in_largest_wcc = maiorWCC_nodes;

  
    const size_t NUM_BFS = 1000;

    std::vector<int> bfs_nodes;
    if (nodes_in_largest_wcc.size() > NUM_BFS) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(nodes_in_largest_wcc.begin(), nodes_in_largest_wcc.end(), gen);
        bfs_nodes.assign(nodes_in_largest_wcc.begin(), nodes_in_largest_wcc.begin() + NUM_BFS);
    } else {
        bfs_nodes = nodes_in_largest_wcc;
    }


    nodes_in_largest_wcc = bfs_nodes;

    const int MAX_DISTANCE = 1000;
    std::vector<size_t> distance_counts(MAX_DISTANCE + 1, 0);
    size_t total_paths = 0;
    int diameter = 0;

    auto bfs_and_count = [&](int start_node, std::vector<size_t> &local_distance_counts, size_t &local_total_paths, int &local_diameter) {
        std::queue<int> fila;
        size_t n = adjList_undirected.size();
        std::vector<int> distances(n, -1);
        distances[start_node] = 0;
        fila.push(start_node);

        while (!fila.empty()) {
            int atual = fila.front();
            fila.pop();

            for (const auto &vizinho : adjList_undirected[atual]) {
                if (distances[vizinho] == -1) {
                    distances[vizinho] = distances[atual] + 1;
                    fila.push(vizinho);
                }
            }
        }


        for (size_t j = 0; j < distances.size(); ++j) {
            if (distances[j] > 0 && distances[j] <= MAX_DISTANCE) {
                local_distance_counts[distances[j]]++;
            }
            if (distances[j] > local_diameter) {
                local_diameter = distances[j];
            }
            if (distances[j] > 0) {
                local_total_paths++;
            }
        }
    };


    std::vector<std::vector<size_t>> all_distance_counts;
    std::vector<size_t> all_total_paths;
    std::vector<int> all_diameters;

    all_distance_counts.resize(omp_get_max_threads(), std::vector<size_t>(MAX_DISTANCE + 1, 0));
    all_total_paths.resize(omp_get_max_threads(), 0);
    all_diameters.resize(omp_get_max_threads(), 0);


    #pragma omp parallel
    {
        int thread_num = omp_get_thread_num();
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < nodes_in_largest_wcc.size(); ++i) {
            bfs_and_count(nodes_in_largest_wcc[i], all_distance_counts[thread_num], all_total_paths[thread_num], all_diameters[thread_num]);
        }
    }


    for (int t = 0; t < omp_get_max_threads(); ++t) {
        for (size_t d = 1; d <= MAX_DISTANCE; ++d) {
            distance_counts[d] += all_distance_counts[t][d];
        }
        if (all_diameters[t] > diameter) {
            diameter = all_diameters[t];
        }
        total_paths += all_total_paths[t];
    }

 
    size_t target = static_cast<size_t>(std::ceil(0.9 * static_cast<double>(total_paths)));
    size_t cumulative_count = 0;
    int effective_diameter = 0;

    for (size_t d = 1; d <= MAX_DISTANCE; ++d) {
        cumulative_count += distance_counts[d];
        if (cumulative_count >= target) {
            effective_diameter = static_cast<int>(d);
            break;
        }
    }


    if (effective_diameter == 0) {
        effective_diameter = diameter;
    }



  
    double fraction_nodes_wcc = static_cast<double>(maiorWCC.numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_wcc = static_cast<double>(maiorWCC.numeroArestas) / static_cast<double>(numeroArestasTotal);
    double fraction_nodes_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroArestas) / static_cast<double>(numeroArestasTotal);


    json root;
    root["dataset_statistics"]["nos"] = numeroNosTotal;
    root["dataset_statistics"]["arestas"] = numeroArestasTotal;

    // Maior WCC
    root["dataset_statistics"]["mais_largo_wcc"]["nos"] = maiorWCC.numeroNos;
    root["dataset_statistics"]["mais_largo_wcc"]["fracao_total_de_nos"] = fraction_nodes_wcc;
    root["dataset_statistics"]["mais_largo_wcc"]["arestas"] = maiorWCC.numeroArestas;
    root["dataset_statistics"]["mais_largo_wcc"]["fracao_total_de_arestas"] = fraction_edges_wcc;

    // Maior SCC
    root["dataset_statistics"]["mais_largo_scc"]["nos"] = scc_infos[maiorSCC_id].numeroNos;
    root["dataset_statistics"]["mais_largo_scc"]["fracao_total_de_nos"] = fraction_nodes_scc;
    root["dataset_statistics"]["mais_largo_scc"]["arestas"] = scc_infos[maiorSCC_id].numeroArestas;
    root["dataset_statistics"]["mais_largo_scc"]["fracao_total_de_arestas"] = fraction_edges_scc;

    // Coeficiente de Agrupamento Médio
    root["dataset_statistics"]["coeficiente_de_agrupamento_medio"] = averageClusteringCoefficient;

    // Triângulos e Trios
    root["dataset_statistics"]["triangulos"] = totalTriangulos;
    root["dataset_statistics"]["fracao_de_triangulos-fechados"] = fractionClosedTriangles;

    // Diâmetro e Diâmetro Efetivo
    root["dataset_statistics"]["diametro"] = diameter;
    root["dataset_statistics"]["diametro_efetivo_90%"] = effective_diameter;

    // Escrever o JSON como string formatada
    std::string jsonOutput = root.dump(2); // '2' indica indentação para leitura humana



    std::cout << "Dados análisado:" << std::endl;
    std::cout << jsonOutput << std::endl;
    std::cout << "===============================================================" << std::endl;
    std::cout << std::endl;

std::ofstream jsonFile("dados.json"); 
if (jsonFile.is_open()) {
    jsonFile << jsonOutput; 
    jsonFile.close();       
    std::cout << "Arquivo 'dados.json' gerado com sucesso!" << std::endl;
} else {
    std::cerr << "Erro ao criar o arquivo 'dados.json'." << std::endl;
}

    return 0;
}
