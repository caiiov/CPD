#include <iostream>
#include <fstream>
#include <set>
#include <sstream>

int main() {
    std::string filePath = "facebook_combined.txt"; // Caminho do arquivo no mesmo diretório
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filePath << "\n";
        return 1;
    }

    std::set<int> nodes; // Conjunto para armazenar nós únicos
    std::string line;
    int lineCount = 0;

    while (std::getline(file, line)) {
        lineCount++;
        std::istringstream iss(line);
        int u, v;

        if (!(iss >> u >> v)) {
            std::cerr << "Linha inválida no arquivo: " << line << "\n";
            continue; // Ignora linhas inválidas
        }

        // Adiciona os nós ao conjunto
        nodes.insert(u);
        nodes.insert(v);
    }

    file.close();

    // Exibe o resultado
    std::cout << "Número total de nós (Nodes): " << nodes.size() << "\n";
    std::cout << "Linhas processadas: " << lineCount << "\n";

    return 0;
}
