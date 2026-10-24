#include "mundo.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
    try {
        if (argc == 9 && std::string(argv[1]) == "--start") {
            const int cidades = std::stoi(argv[2]);
            const int populacao = std::stoi(argv[3]);
            const double mutacao = std::stod(argv[4]);
            const int maxEpocas = std::stoi(argv[5]);
            const int semMelhora = std::stoi(argv[6]);
            const std::string cenario = argv[7];
            const std::string pasta = argv[8];
            std::filesystem::create_directories(pasta);
            std::filesystem::remove(std::filesystem::path(pasta) / "estado.dat");
            Mundo mundo(cidades, populacao, mutacao, maxEpocas, semMelhora, cenario, pasta);
            std::cout << "Experimento iniciado na época 0.\n";
            return 0;
        }
        if (argc == 4 && std::string(argv[1]) == "--step") {
            Mundo mundo(argv[2]);
            mundo.avancar(std::stoi(argv[3]));
            std::cout << "Época atual: " << mundo.epocaAtual()
                      << (mundo.concluido() ? " (experimento concluído)" : "") << '\n';
            return 0;
        }
        std::cerr << "Uso: caixeiro --start cidades populacao mutacao max_epocas sem_melhora cenario pasta\n"
                     "     caixeiro --step pasta quantidade_epocas\n";
        return 2;
    } catch (const std::exception& erro) {
        std::cerr << erro.what() << '\n';
        return 1;
    }
}
