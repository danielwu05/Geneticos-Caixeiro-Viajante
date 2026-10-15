#include "mundo.hpp"
#include <iostream>
#include <iomanip>

int main() {
    // 1. Cria um mundo com 20 cidades em círculo, mutação 0.05 e população de 100
    Mundo* mundo = Mundo::Setup(20, DistribuicaoCidades::Circular, 0.05, 100);

    // 2. Escolhe uma combinação de operadores
    SelecaoTorneio selecao(3);
    CrossoverERX crossover;
    MutacaoInversao mutacao;

    std::cout << "--- Inicio da Simulacao ---\n";

    // 3. Roda 30 épocas
    for (int i = 0; i < 30; ++i) {
        mundo->AvancarEpoca(selecao, crossover, mutacao);
    }

    // 4. Exibe o histórico de evolução (Telemetria)
    std::cout << std::fixed << std::setprecision(4);
    for (const auto& era : mundo->Eras()) {
        std::cout << "Epoca " << std::setw(2) << era.indiceEpoca 
                    << " | Melhor: " << std::setw(8) << era.melhorDistancia 
                    << " | Media: "  << std::setw(8) << era.distanciaMedia 
                    << " | Pior: "   << std::setw(8) << era.piorDistancia 
                    << " | Op: "     << era.nomeSelecao << " + " << era.nomeCrossover << "\n";
    }

    delete mundo;
    return 0;
}
