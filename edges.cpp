#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <filesystem>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;

// Função auxiliar para normalizar arestas no formato (menor, maior)
std::pair<int, int> normalizeEdge(int u, int v) {
    return (u < v) ? std::make_pair(u, v) : std::make_pair(v, u);
}

int main() {
    try {
        std::string directory = "./facebook"; // Caminho do dataset
        std::set<std::pair<int, int>> uniqueEdges; // Conjunto global para armazenar arestas únicas
        std::vector<std::string> problematicFiles; // Lista de arquivos problemáticos

        // Log para analisar dados processados
        std::ofstream logFile("edge_analysis.log");
        if (!logFile.is_open()) {
            std::cerr << "Erro ao abrir o arquivo de log.\n";
            return 1;
        }

        // Iterar por todos os arquivos no diretório
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.path().extension() == ".edges") {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    std::cerr << "Não foi possível abrir o arquivo: " << entry.path().string() << "\n";
                    problematicFiles.push_back(entry.path().string());
                    continue;
                }

                std::string line;
                int lineCount = 0;
                int edgeCount = 0;

                while (std::getline(file, line)) {
                    lineCount++;
                    std::istringstream iss(line);
                    int u, v;

                    if (!(iss >> u >> v)) {
                        std::cerr << "Linha inválida no arquivo " << entry.path().filename() << ": " << line << "\n";
                        continue; // Ignora linhas inválidas
                    }

                    uniqueEdges.insert(normalizeEdge(u, v));
                    edgeCount++;
                }

                file.close();

                // Registrar resumo no log
                logFile << "Arquivo: " << entry.path().filename()
                        << " - Linhas processadas: " << lineCount
                        << ", Arestas válidas: " << edgeCount << "\n";
            }
        }

        // Exibe o número total de arestas únicas
        std::cout << "Número total de arestas (Edges): " << uniqueEdges.size() << "\n";
        logFile << "Número total de arestas únicas: " << uniqueEdges.size() << "\n";

        // Exibe arquivos problemáticos, se houver
        if (!problematicFiles.empty()) {
            std::cerr << "Arquivos com problemas:\n";
            for (const auto& file : problematicFiles) {
                std::cerr << " - " << file << "\n";
            }
        }

        logFile.close();

    } catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
