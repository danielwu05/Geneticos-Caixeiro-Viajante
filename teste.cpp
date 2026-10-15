#include "mundo.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <sys/resource.h>

// Função auxiliar nativa para obter o pico de memória RAM (RSS) em Megabytes
double ObterPicoMemoriaMB() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss / 1024.0; // No Linux, ru_maxrss é medido em KB
}

int main() {
    // Parâmetros da simulação
    const int numCidades = 750;
    const int tamanhoPop = 200;
    const int totalEpocas = 3000;
    const double taxaMutacaoInicial = 0.1;

    std::cout << "====================================================\n";
    std::cout << "   INICIALIZANDO BENCHMARK DO ALGORITMO GENÉTICO    \n";
    std::cout << "====================================================\n";

    // 1. Cronômetro da execução completa
    auto inicio = std::chrono::steady_clock::now();

    // 2. Setup do Mundo
    Mundo* mundo = Mundo::Setup(numCidades, DistribuicaoCidades::Uniforme, taxaMutacaoInicial, tamanhoPop);

    // 3. Escolha dos operadores genéticos
    SelecaoTorneio selecao(3);
    CrossoverUniforme crossover;
    MutacaoInsercao mutacao;

    std::cout << "Configuração:\n";
    std::cout << "  - Cidades: " << numCidades << "\n";
    std::cout << "  - População: " << tamanhoPop << "\n";
    std::cout << "  - Seleção: " << selecao.Nome() << "\n";
    std::cout << "  - Crossover: " << crossover.Nome() << "\n";
    std::cout << "  - Mutação: " << mutacao.Nome() << "\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Evoluindo " << totalEpocas << " épocas...\n\n";

    // 4. Loop de evolução das épocas
    for (int i = 0; i < totalEpocas; ++i) {
        mundo->AvancarEpoca(selecao, crossover, mutacao);
    }

    auto fim = std::chrono::steady_clock::now();
    std::chrono::duration<double> duracaoSegundos = fim - inicio;

    // 5. Exibe amostras da telemetria (início, meio e fim)
    const auto& eras = mundo->Eras();
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "--- Telemetria Selecionada ---\n";
    for (size_t i = 0; i < eras.size(); i += (totalEpocas / 10)) {
        std::cout << "Época " << std::setw(3) << eras[i].indiceEpoca 
                  << " | Melhor: " << std::setw(8) << eras[i].melhorDistancia 
                  << " | Média: "  << std::setw(8) << eras[i].distanciaMedia 
                  << " | Pior: "   << std::setw(8) << eras[i].piorDistancia << "\n";
    }
    // Garante que a última época seja impressa
    if ((eras.size() - 1) % (totalEpocas / 10) != 0) {
        const auto& ultima = eras.back();
        std::cout << "Época " << std::setw(3) << ultima.indiceEpoca 
                  << " | Melhor: " << std::setw(8) << ultima.melhorDistancia 
                  << " | Média: "  << std::setw(8) << ultima.distanciaMedia 
                  << " | Pior: "   << std::setw(8) << ultima.piorDistancia << "\n";
    }

    // 6. Cálculo do Ótimo Teórico D*(n) no círculo unitário
    // D*(n) = 2 * n * sin(pi / n)
    double dEstrela = 2.0 * numCidades * std::sin(M_PI / numCidades);
    double melhorDistInicial = eras.front().melhorDistancia;
    double melhorDistFinal = eras.back().melhorDistancia;
    double reducao = (1.0 - (melhorDistFinal / melhorDistInicial)) * 100.0;
    double erroDStar = ((melhorDistFinal - dEstrela) / dEstrela) * 100.0;

    // 7. Relatório Final de Métricas (Performance, Memória e Tempo)
    std::cout << "\n====================================================\n";
    std::cout << "                 RELATÓRIO FINAL                    \n";
    std::cout << "====================================================\n";
    std::cout << "Distância Inicial (Época 0):    " << melhorDistInicial << "\n";
    std::cout << "Distância Final (Época " << totalEpocas << "):  " << melhorDistFinal << "\n";
    std::cout << "Redução total da rota:          " << reducao << " %\n";
    std::cout << "Ótimo Teórico D*(100):          " << dEstrela << "\n";
    std::cout << "Distância acima do ótimo:       " << erroDStar << " %\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Tempo Total de Execução:        " << duracaoSegundos.count() << " segundos\n";
    std::cout << "Tempo Médio por Época:          " << (duracaoSegundos.count() * 1000.0 / totalEpocas) << " ms/época\n";
    std::cout << "Pico de Uso de Memória (RSS):   " << ObterPicoMemoriaMB() << " MB\n";
    std::cout << "====================================================\n";

    delete mundo;
    return 0;
}
