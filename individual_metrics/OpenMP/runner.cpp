#include <iostream>
#include <cstdlib> 
#include <chrono>

int main() {
    // Lista de arquivos fonte
	std::string files[] = {"edges_in_largest_SCC_openmp.cpp", "diagram_openmp.cpp ", "avarage_clustering_openmp.cpp", "diagram_effective_OpenMP.cpp", "fraction_of_closed_triangles_openmp.cpp","largest_wcc_openmp.cpp", "nodes_and_edges_openmp.cpp"};
	std::string input_file = "facebook_combined.txt";
	
	auto start_time = std::chrono::high_resolution_clock::now();
	
    for (const auto& file : files) {
        std::string command = std::string("g++ ") + "-fopenmp " + file + " -o " + file.substr(0, file.find('.')) + " && ./" + file.substr(0, file.find('.')) + " " + input_file;
        
        std::cout << "Compilando e executando: " << file << std::endl;
        
        // Executa o comando de compilação e execução
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "Código " << file << " executado com sucesso!\n" << std::endl;
        } else {
            std::cout << "Erro ao compilar ou executar " << file << std::endl;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
      
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        std::cout << "Computation Time: " << duration.count() << " seconds" << std::endl;

    return 0;
}

