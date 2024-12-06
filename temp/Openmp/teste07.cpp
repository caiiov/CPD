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
#include <nlohmann/json.hpp>
#include <random>
#include <cmath> 
#include <sys/resource.h> 
#include <sys/time.h> 
#include <unistd.h> 
#include <chrono> 
#include <omp.h> 
using json = nlohmann::json;

// Estrutura para armazenar informações de um componente
struct ComponentInfo {
    size_t numeroNos;
    size_t numeroArestas;
};

// Função para obter o uso de memória residente em KB
size_t getCurrentRSS() {
    std::ifstream status_file("/proc/self/status");
    std::string line;
    size_t rss = 0;
    while (std::getline(status_file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string key;
            size_t value;
            std::string unit;
            iss >> key >> value >> unit;
            rss = value; // em KB
            break;
        }
    }
    return rss;
}

// Função para obter o tempo de CPU (usuário + sistema) em segundos
double getCPUTime() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    double user_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double sys_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;
    return user_time + sys_time;
}

int main() {
    // Início da medição de tempo
    auto start_time = std::chrono::steady_clock::now();

    std::ifstream arquivo("web-Google.txt");
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo 'web-Google.txt'. Verifique se o arquivo existe e o caminho está correto." << std::endl;
        return 1;
    }

    std::unordered_set<long long> nos; // Usando long long para identificar nós numéricos.
    size_t numeroArestasTotal = 0;
    std::vector<std::pair<long long, long long>> arestas_direcionadas; // Lista de arestas direcionadas

    std::string linha;
    while (std::getline(arquivo, linha)) {
        // Ignorar linhas vazias e comentários
        if (linha.empty() || linha[0] == '#') continue;

        std::istringstream iss(linha);
        long long no1, no2;
        if (!(iss >> no1 >> no2)) {
            // Ignorar linhas com formato inválido
            continue;
        }

        // Ignorar loops (auto-arestas)
        if (no1 == no2) continue;

        nos.insert(no1);
        nos.insert(no2);
        arestas_direcionadas.emplace_back(no1, no2);
        numeroArestasTotal++;
    }

    arquivo.close();

    size_t numeroNosTotal = nos.size();

    // Mapeamento de nós para índices
    std::unordered_map<long long, int> mapaNos;
    int indice = 0;
    for (const auto &no : nos) {
        mapaNos[no] = indice++;
    }

    // Construção das listas de adjacência
    // Para WCC e Coeficiente de Agrupamento (não direcionado)
    std::vector<std::vector<int>> adjList_undirected(numeroNosTotal, std::vector<int>());
    // Para SCC (direcionado)
    std::vector<std::vector<int>> adjList_direcionado(numeroNosTotal, std::vector<int>());
    // Para Grafo Transposto (necessário para Kosaraju)
    std::vector<std::vector<int>> adjList_transposto(numeroNosTotal, std::vector<int>());

    // Preencher as listas de adjacência
    for (const auto &aresta : arestas_direcionadas) {
        int idxU = mapaNos[aresta.first];
        int idxV = mapaNos[aresta.second];
        // Para WCC e Coeficiente de Agrupamento
        adjList_undirected[idxU].push_back(idxV);
        adjList_undirected[idxV].push_back(idxU);
        // Para SCC
        adjList_direcionado[idxU].push_back(idxV);
        // Para Grafo Transposto
        adjList_transposto[idxV].push_back(idxU);
    }

    // -------------------
    // Remover Duplicatas e Auto-Arestas nas Listas de Adjacência Não Direcionadas
    // -------------------
    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < numeroNosTotal; ++i) {
        // Ordenar a lista de adjacência
        std::sort(adjList_undirected[i].begin(), adjList_undirected[i].end());

        // Remover duplicatas
        adjList_undirected[i].erase(std::unique(adjList_undirected[i].begin(), adjList_undirected[i].end()), adjList_undirected[i].end());

        // Remover auto-arestas (nós conectados a si mesmos)
        adjList_undirected[i].erase(std::remove(adjList_undirected[i].begin(), adjList_undirected[i].end(), i), adjList_undirected[i].end());
    }

    // -------------------
    // Cálculo do Maior WCC
    // -------------------
    std::vector<bool> visitado_wcc(numeroNosTotal, false);
    ComponentInfo maiorWCC = {0, 0};
    std::vector<int> maiorWCC_nodes; // Armazenar os nós da maior WCC

    // Função lambda para realizar DFS iterativo para WCC
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

        // Cada aresta é contada duas vezes em um grafo não direcionado
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

    // -------------------
    // Cálculo do Maior SCC usando Kosaraju Otimizado
    // -------------------

    // Passo 1: Ordem de finalização usando DFS no grafo direcionado (Iterativo)
    std::vector<bool> visitado_kosaraju(numeroNosTotal, false);
    std::vector<int> ordem_finalizacao;

    // Função lambda para realizar DFS iterativo no grafo direcionado
    auto dfs_iterativo_kosaraju = [&](int start_node, std::vector<int> &ordem) {
        std::stack<std::pair<int, bool>> pilha; // Pair: (node, is_processed)
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
            pilha.emplace(node, true); // Marcar para adicionar à ordem após explorar vizinhos

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

    // Passo 2: DFS no grafo transposto seguindo a ordem de finalização decrescente
    std::fill(visitado_kosaraju.begin(), visitado_kosaraju.end(), false);
    std::vector<int> scc_ids(numeroNosTotal, -1); // ID da SCC para cada nó
    int current_scc_id = 0;
    std::vector<ComponentInfo> scc_infos; // Informações de cada SCC

    // Função lambda para realizar DFS iterativo no grafo transposto e atribuir IDs de SCC
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

    // Iterar na ordem de finalização decrescente
    for (auto it = ordem_finalizacao.rbegin(); it != ordem_finalizacao.rend(); ++it) {
        int node = *it;
        if (!visitado_kosaraju[node]) {
            ComponentInfo scc_info = {0, 0};
            dfs_iterativo_transposto(node, current_scc_id, scc_info);
            scc_infos.push_back(scc_info);
            current_scc_id++;
        }
    }

    // Identificar o maior SCC
    size_t maiorSCC_id = 0;
    size_t maiorSCC_size = 0;
    for (size_t i = 0; i < scc_infos.size(); ++i) {
        if (scc_infos[i].numeroNos > maiorSCC_size) {
            maiorSCC_size = scc_infos[i].numeroNos;
            maiorSCC_id = i;
        }
    }

    // Contar as arestas no maior SCC
    // Abordagem otimizada: Iterar sobre todas as arestas uma única vez
    size_t maiorSCC_arestas = 0;
    #pragma omp parallel for reduction(+:maiorSCC_arestas) schedule(dynamic)
    for (size_t i = 0; i < arestas_direcionadas.size(); ++i) {
        int idxU = mapaNos[arestas_direcionadas[i].first];
        int idxV = mapaNos[arestas_direcionadas[i].second];
        if (scc_ids[idxU] == maiorSCC_id && scc_ids[idxV] == maiorSCC_id) {
            maiorSCC_arestas++;
        }
    }

    // Atualizar o número de arestas do maior SCC
    scc_infos[maiorSCC_id].numeroArestas = maiorSCC_arestas;

    // -------------------
    // Cálculo do Coeficiente de Agrupamento Médio utilizando OpenMP
    // -------------------
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
                    // Como as listas estão ordenadas, usamos binary_search
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

    // Cálculo do coeficiente de agrupamento médio
    double averageClusteringCoefficient = somaClustering / static_cast<double>(numeroNosTotal);

    // -------------------
    // Cálculo do Número Total de Triângulos Fechados e Trios Conectados utilizando OpenMP
    // -------------------
    size_t totalTriangulos = 0;
    size_t totalTrios = 0;

    // Contagem de triângulos e trios
    #pragma omp parallel for reduction(+:totalTriangulos, totalTrios) schedule(dynamic)
    for (size_t i = 0; i < adjList_undirected.size(); ++i) {
        size_t grau = adjList_undirected[i].size();

        if (grau < 2) continue;

        // Contar trios locais para o nó atual
        size_t localTriosNode = grau * (grau - 1) / 2;
        totalTrios += localTriosNode;

        // Contar triângulos fechados centrados no nó atual
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

    // Cada triângulo foi contado três vezes (uma vez para cada nó que o compõe)
    totalTriangulos /= 3;

    // Calcular a Fração de Triângulos Fechados
    double fractionClosedTriangles = 0.0;
    if (totalTrios > 0) {
        fractionClosedTriangles = static_cast<double>(totalTriangulos * 3) / static_cast<double>(totalTrios);
    }

    // -------------------
    // Cálculo do Diâmetro e do Diâmetro Efetivo (90º Percentil) - Utilizando OpenMP
    // -------------------
    std::vector<int> nodes_in_largest_wcc = maiorWCC_nodes;

    // Definir o número de nós para BFS (exemplo: 1000)
    const size_t NUM_BFS = 1000;

    // Selecionar aleatoriamente 1000 nós da maior WCC sem usar std::find
    std::vector<int> bfs_nodes;
    if (nodes_in_largest_wcc.size() > NUM_BFS) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(nodes_in_largest_wcc.begin(), nodes_in_largest_wcc.end(), gen);
        bfs_nodes.assign(nodes_in_largest_wcc.begin(), nodes_in_largest_wcc.begin() + NUM_BFS);
    } else {
        bfs_nodes = nodes_in_largest_wcc;
    }

    // Atualizar a lista de nós para BFS
    nodes_in_largest_wcc = bfs_nodes;

    // Variáveis para armazenar os comprimentos dos caminhos
    const int MAX_DISTANCE = 1000;
    std::vector<size_t> distance_counts(MAX_DISTANCE + 1, 0);
    size_t total_paths = 0;
    int diameter = 0;

    // Função lambda para realizar BFS e contar distâncias
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

        // Atualizar contadores de distância
        for (size_t j = 0; j < distances.size(); ++j) {
            if (distances[j] > 0 && distances[j] <= MAX_DISTANCE) { // Limitação de distância
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

    // Preparar variáveis para armazenar resultados parciais
    std::vector<std::vector<size_t>> all_distance_counts;
    std::vector<size_t> all_total_paths;
    std::vector<int> all_diameters;

    all_distance_counts.resize(omp_get_max_threads(), std::vector<size_t>(MAX_DISTANCE + 1, 0));
    all_total_paths.resize(omp_get_max_threads(), 0);
    all_diameters.resize(omp_get_max_threads(), 0);

    // Executar BFS paralelamente
    #pragma omp parallel
    {
        int thread_num = omp_get_thread_num();
        #pragma omp for schedule(dynamic)
        for (size_t i = 0; i < nodes_in_largest_wcc.size(); ++i) {
            bfs_and_count(nodes_in_largest_wcc[i], all_distance_counts[thread_num], all_total_paths[thread_num], all_diameters[thread_num]);
        }
    }

    // Agregar resultados das threads
    for (int t = 0; t < omp_get_max_threads(); ++t) {
        for (size_t d = 1; d <= MAX_DISTANCE; ++d) {
            distance_counts[d] += all_distance_counts[t][d];
        }
        if (all_diameters[t] > diameter) {
            diameter = all_diameters[t];
        }
        total_paths += all_total_paths[t];
    }

    // Calcular o percentil de 90% com correções
    size_t target = static_cast<size_t>(std::ceil(0.9 * static_cast<double>(total_paths)));
    size_t cumulative_count = 0;
    int effective_diameter = 0;

    for (size_t d = 1; d <= MAX_DISTANCE; ++d) {
        cumulative_count += distance_counts[d];
        if (cumulative_count >= target) {
            effective_diameter = static_cast<int>(d); // Converter de size_t para int
            break;
        }
    }

    // Verificar se o effective_diameter foi definido
    if (effective_diameter == 0) {
        effective_diameter = diameter;
    }

    // Fim da medição de tempo
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    double wall_clock_time = elapsed_seconds.count(); // em segundos

    // Obter tempo de CPU usado
    double cpu_time = getCPUTime(); // em segundos

    // Obter uso de memória
    size_t memory_usage_kb = getCurrentRSS(); // em KB

    // -------------------
    // Montagem do JSON utilizando JsonCpp
    // -------------------
    // Calcular fração de total de nós e arestas para WCC e SCC
    double fraction_nodes_wcc = static_cast<double>(maiorWCC.numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_wcc = static_cast<double>(maiorWCC.numeroArestas) / static_cast<double>(numeroArestasTotal);
    double fraction_nodes_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroNos) / static_cast<double>(numeroNosTotal);
    double fraction_edges_scc = static_cast<double>(scc_infos[maiorSCC_id].numeroArestas) / static_cast<double>(numeroArestasTotal);

    // Montar o JSON usando JsonCpp
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


    // -------------------
    // Impressão dos Resultados no Console
    // -------------------
    std::cout << "Dados análisado:" << std::endl;
    std::cout << jsonOutput << std::endl;
    std::cout << "===============================================================" << std::endl;
    std::cout << std::endl;

    // -------------------
    // Impressão dos Dados de CPU e Memória
    // -------------------
    //std::cout << "===================== Dados de Desempenho =====================" << std::endl;
    //std::cout << "Tempo Total de Execução (Wall-Clock Time): " << std::fixed << std::setprecision(6) << wall_clock_time << " segundos" << std::endl;
    //std::cout << "Tempo Total de CPU Ocupado: " << std::fixed << std::setprecision(6) << cpu_time << " segundos" << std::endl;
    //std::cout << "Consumo de Memória: " << memory_usage_kb / 1024.0 << " MB" << std::endl;
    //std::cout << "===============================================================" << std::endl;

    return 0;
}
