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
#include <pthread.h>
#include <mutex>
#include <jsoncpp/json/json.h>
#include <random>
#include <cmath>
#include <chrono>
#include <sys/resource.h>
#include <sys/time.h>

struct ComponentInfo {
    size_t numeroNos;
    size_t numeroArestas;
};

struct ThreadArgs {
    int thread_id;
    int num_threads;
    const std::vector<std::vector<int>>* adjList_undirected;
    double somaClustering;
    size_t totalTriangulos;
    size_t totalTrios;
};

struct EffectiveDiameterArgs {
    int thread_id;
    int num_threads;
    std::vector<int> nodes;
    const std::vector<std::vector<int>>* adjList;
    std::vector<size_t> distance_counts;
    size_t total_paths;
    int local_diameter;
};

void* contarTriangulosTrios(void* args) {
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(args);
    int thread_id = threadArgs->thread_id;
    int num_threads = threadArgs->num_threads;
    const std::vector<std::vector<int>> &adjList = *(threadArgs->adjList_undirected);
    size_t localTriangulos = 0;
    size_t localTrios = 0;

    size_t numeroNosTotal = adjList.size();
    for (size_t i = thread_id; i < numeroNosTotal; i += num_threads) {
        size_t grau = adjList[i].size();

        if (grau < 2) continue;

        size_t localTriosNode = grau * (grau - 1) / 2;
        localTrios += localTriosNode;

        for (size_t j = 0; j < adjList[i].size(); ++j) {
            for (size_t k = j + 1; k < adjList[i].size(); ++k) {
                int neighbor1 = adjList[i][j];
                int neighbor2 = adjList[i][k];
                if (std::binary_search(adjList[neighbor1].begin(), adjList[neighbor1].end(), neighbor2)) {
                    localTriangulos++;
                }
            }
        }
    }

    threadArgs->totalTriangulos = localTriangulos;
    threadArgs->totalTrios = localTrios;

    pthread_exit(nullptr);
}

void* calcularClustering(void* args) {
    ThreadArgs* threadArgs = static_cast<ThreadArgs*>(args);
    int thread_id = threadArgs->thread_id;
    int num_threads = threadArgs->num_threads;
    const std::vector<std::vector<int>> &adjList = *(threadArgs->adjList_undirected);
    double local_somaClustering = 0.0;

    size_t numeroNosTotal = adjList.size();
    for (size_t i = thread_id; i < numeroNosTotal; i += num_threads) {
        size_t grau = adjList[i].size();

        size_t totalPossiveis = grau * (grau - 1) / 2;
        size_t triangles = 0;
        double C_u = 0.0;

        if (totalPossiveis > 0) {
            for (size_t j = 0; j < adjList[i].size(); ++j) {
                for (size_t k = j + 1; k < adjList[i].size(); ++k) {
                    int neighbor1 = adjList[i][j];
                    int neighbor2 = adjList[i][k];
                    if (std::binary_search(adjList[neighbor1].begin(), adjList[neighbor1].end(), neighbor2)) {
                        triangles++;
                    }
                }
            }
            C_u = static_cast<double>(triangles) / static_cast<double>(totalPossiveis);
        } else {
            C_u = 0.0;
        }

        local_somaClustering += C_u;
    }

    threadArgs->somaClustering = local_somaClustering;

    pthread_exit(nullptr);
}

void* thread_calcularDistancias_optimized(void* args_void) {
    EffectiveDiameterArgs* eda = static_cast<EffectiveDiameterArgs*>(args_void);
    for (size_t i = eda->thread_id; i < eda->nodes.size(); i += eda->num_threads) {
        std::queue<int> fila;
        size_t n = eda->adjList->size();
        std::vector<int> distances(n, -1);
        int start = eda->nodes[i];
        distances[start] = 0;
        fila.push(start);

        while (!fila.empty()) {
            int atual = fila.front();
            fila.pop();

            for (const auto &vizinho : (*eda->adjList)[atual]) {
                if (distances[vizinho] == -1) {
                    distances[vizinho] = distances[atual] + 1;
                    fila.push(vizinho);
                }
            }
        }

        for (size_t j = 0; j < distances.size(); ++j) {
            if (distances[j] > 0 && distances[j] <= 1000) {
                eda->distance_counts[distances[j]]++;
            }
            if (distances[j] > eda->local_diameter) {
                eda->local_diameter = distances[j];
            }
            if (distances[j] > 0) {
                eda->total_paths++;
            }
        }
    }
    pthread_exit(nullptr);
}

int main() {
    auto start_time = std::chrono::high_resolution_clock::now();

    struct rusage usage_start, usage_end;
    getrusage(RUSAGE_SELF, &usage_start);

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
    for (const auto &aresta : arestas_direcionadas) {
        int idxU = mapaNos[aresta.first];
        int idxV = mapaNos[aresta.second];
        if (scc_ids[idxU] == maiorSCC_id && scc_ids[idxV] == maiorSCC_id) {
            maiorSCC_arestas++;
        }
    }

    scc_infos[maiorSCC_id].numeroArestas = maiorSCC_arestas;

    double somaClustering = 0.0;
    size_t totalTriangulos = 0;
    size_t totalTrios = 0;

    const int NUM_THREADS = 8;
    pthread_t threads_ids[NUM_THREADS];
    ThreadArgs threadArgs[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        threadArgs[i].thread_id = i;
        threadArgs[i].num_threads = NUM_THREADS;
        threadArgs[i].adjList_undirected = &adjList_undirected;
        threadArgs[i].somaClustering = 0.0;
        threadArgs[i].totalTriangulos = 0;
        threadArgs[i].totalTrios = 0;
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads_ids[i], nullptr, calcularClustering, (void*)&threadArgs[i]) != 0) {
            std::cerr << "Erro ao criar thread " << i << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads_ids[i], nullptr);
        somaClustering += threadArgs[i].somaClustering;
    }

    double averageClusteringCoefficient = somaClustering / static_cast<double>(numeroNosTotal);

    for (int i = 0; i < NUM_THREADS; ++i) {
        threadArgs[i].totalTriangulos = 0;
        threadArgs[i].totalTrios = 0;
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&threads_ids[i], nullptr, contarTriangulosTrios, (void*)&threadArgs[i]) != 0) {
            std::cerr << "Erro ao criar thread " << i << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads_ids[i], nullptr);
        totalTriangulos += threadArgs[i].totalTriangulos;
        totalTrios += threadArgs[i].totalTrios;
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

    EffectiveDiameterArgs eda_args[NUM_THREADS];
    pthread_t diameter_threads_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i) {
        eda_args[i].thread_id = i;
        eda_args[i].num_threads = NUM_THREADS;
        eda_args[i].nodes = nodes_in_largest_wcc;
        eda_args[i].adjList = &adjList_undirected;
        eda_args[i].distance_counts.assign(MAX_DISTANCE + 1, 0);
        eda_args[i].total_paths = 0;
        eda_args[i].local_diameter = 0;
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        if (pthread_create(&diameter_threads_ids[i], nullptr, thread_calcularDistancias_optimized, (void*)&eda_args[i]) != 0) {
            std::cerr << "Erro ao criar thread de diâmetro " << i << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(diameter_threads_ids[i], nullptr);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        for (size_t d = 1; d <= MAX_DISTANCE; ++d) {
            distance_counts[d] += eda_args[i].distance_counts[d];
        }
        if (eda_args[i].local_diameter > diameter) {
            diameter = eda_args[i].local_diameter;
        }
        total_paths += eda_args[i].total_paths;
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

    getrusage(RUSAGE_SELF, &usage_end);

    double cpu_time_user = (usage_end.ru_utime.tv_sec - usage_start.ru_utime.tv_sec) +
                           (usage_end.ru_utime.tv_usec - usage_start.ru_utime.tv_usec) / 1e6;
    double cpu_time_system = (usage_end.ru_stime.tv_sec - usage_start.ru_stime.tv_sec) +
                             (usage_end.ru_stime.tv_usec - usage_start.ru_stime.tv_usec) / 1e6;
    double cpu_time_total = cpu_time_user + cpu_time_system;

    long max_rss = usage_end.ru_maxrss;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    Json::Value root;
    double fraction_nodes_wcc = static_cast<double>(maiorWCC.numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_wcc = static_cast<double>(maiorWCC.numeroArestas) / static_cast<double>(numeroArestasTotal);
    double fraction_nodes_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroArestas) / static_cast<double>(numeroArestasTotal);

    root["graph_metrics"]["nodes"] = static_cast<Json::UInt64>(numeroNosTotal);
    root["graph_metrics"]["edges"] = static_cast<Json::UInt64>(numeroArestasTotal);

    root["graph_metrics"]["largest_wcc"]["nodes"] = static_cast<Json::UInt64>(maiorWCC.numeroNos);
    root["graph_metrics"]["largest_wcc"]["fraction_of_total_nodes"] = fraction_nodes_wcc;
    root["graph_metrics"]["largest_wcc"]["edges"] = static_cast<Json::UInt64>(maiorWCC.numeroArestas);
    root["graph_metrics"]["largest_wcc"]["fraction_of_total_edges"] = fraction_edges_wcc;

    root["graph_metrics"]["largest_scc"]["nodes"] = static_cast<Json::UInt64>(scc_infos[maiorSCC_id].numeroNos);
    root["graph_metrics"]["largest_scc"]["fraction_of_total_nodes"] = fraction_nodes_scc;
    root["graph_metrics"]["largest_scc"]["edges"] = static_cast<Json::UInt64>(scc_infos[maiorSCC_id].numeroArestas);
    root["graph_metrics"]["largest_scc"]["fraction_of_total_edges"] = fraction_edges_scc;

    root["graph_metrics"]["average_clustering_coefficient"] = averageClusteringCoefficient;

    root["graph_metrics"]["triangles"] = static_cast<Json::UInt64>(totalTriangulos);
    root["graph_metrics"]["fraction_of_closed_triangles"] = fractionClosedTriangles;

    root["graph_metrics"]["diameter"] = diameter;
    root["graph_metrics"]["effective_diameter_90_percentile"] = effective_diameter;

    root["performance_metrics"]["execution_time_seconds"] = elapsed_seconds.count();
    root["performance_metrics"]["cpu_time_user_seconds"] = cpu_time_user;
    root["performance_metrics"]["cpu_time_system_seconds"] = cpu_time_system;
    root["performance_metrics"]["cpu_time_total_seconds"] = cpu_time_total;
    root["performance_metrics"]["memory_usage_kb"] = static_cast<Json::Int64>(max_rss);

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "  ";
    std::string jsonOutput = Json::writeString(writerBuilder, root);

    std::cout << jsonOutput << std::endl;

    return 0;
}
