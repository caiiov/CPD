#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Extrai o nó ego do nome do arquivo (e.g., "107.edges" -> 107)
int extractEgoNode(const std::string& filename) {
    return std::stoi(filename.substr(0, filename.find('.')));
}

// Adiciona nós de um arquivo ao conjunto
void addNodesFromEdgesFile(const std::string& filepath, std::unordered_set<int>& nodes) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filepath);
    }

    int u, v;
    while (file >> u >> v) {
        nodes.insert(u); // Nó de origem
        nodes.insert(v); // Nó de destino
    }
}

// Adiciona nós do arquivo .feat ao conjunto
void addNodesFromFeatFile(const std::string& filepath, std::unordered_set<int>& nodes) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o arquivo: " + filepath);
    }

    int node;
    while (file >> node) {
        nodes.insert(node); // Nó listado no arquivo
    }
}

int main() {
    try {
        // Diretório contendo os arquivos do dataset
        std::string directory = "./facebook"; // Substitua pelo caminho correto
        std::unordered_set<int> nodes; // Conjunto para armazenar nós únicos

        // Processa todos os arquivos do diretório
        for (const auto& entry : fs::directory_iterator(directory)) {
            std::string extension = entry.path().extension();
            std::string filename = entry.path().filename();

            if (extension == ".edges") {
                // Adiciona o nó ego do arquivo
                int egoNode = extractEgoNode(filename);
                nodes.insert(egoNode);

                // Adiciona nós das arestas
                addNodesFromEdgesFile(entry.path().string(), nodes);
            } else if (extension == ".feat") {
                // Adiciona nós do arquivo .feat
                addNodesFromFeatFile(entry.path().string(), nodes);
            }
        }

        // Número total de nós
        std::cout << "Número total de nós (Nodes): " << nodes.size() << "\n";

    } catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
