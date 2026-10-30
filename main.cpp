#include "mundo.hpp"
#include "selecao.hpp"
#include <filesystem>
#include <iostream>

namespace {
std::unique_ptr<Crossover> criarCrossover(const std::string &nome) {
  if (nome == "PMX")
    return std::make_unique<CrossoverPMX>();
  if (nome == "ERX")
    return std::make_unique<CrossoverERX>();
  if (nome == "Uniforme")
    return std::make_unique<CrossoverUniforme>();
  return std::make_unique<CrossoverOX>();
}

std::unique_ptr<Mutacao> criarMutacao(const std::string &nome) {
  if (nome == "Troca")
    return std::make_unique<MutacaoTroca>();
  if (nome == "Inserção" || nome == "Insercao")
    return std::make_unique<MutacaoInsercao>();
  return std::make_unique<MutacaoInversao>();
}

std::unique_ptr<Selecao> criarSelecao(const std::string &nome) {
  if (nome == "Roleta")
    return std::make_unique<SelecaoRoleta>();
  if (nome == "Ranking")
    return std::make_unique<SelecaoRanking>();
  if (nome == "Top Ranking" || nome == "TopRanking")
    return std::make_unique<SelecaoTopRanking>(0.2);
  if (nome == "Amostragem Estocástica" || nome == "AmostragemEstocastica" ||
      nome == "SUS")
    return std::make_unique<SelecaoAmostragemEstocastica>();
  return std::make_unique<SelecaoTorneio>(4);
}
} // namespace

int main(int argc, char **argv) {
  try {
    if (argc >= 9 && std::string(argv[1]) == "--start") {
      int cidades = std::stoi(argv[2]);
      int pop = std::stoi(argv[3]);
      double mut = std::stod(argv[4]);
      int maxEp = std::stoi(argv[5]);
      int semMelh = std::stoi(argv[6]);
      std::string cenario = argv[7];
      std::string pasta = argv[8];

      std::filesystem::create_directories(pasta);
      std::filesystem::remove(std::filesystem::path(pasta) / "estado.dat");
      std::filesystem::remove(std::filesystem::path(pasta) / "historico.csv");
      Mundo mundo(cidades, pop, mut, maxEp, semMelh, cenario, pasta);

      std::string crossNome = (argc > 9) ? argv[9] : "OX";
      std::string mutNome = (argc > 10) ? argv[10] : "Inversão";
      std::string selNome = (argc > 11) ? argv[11] : "Torneio";

      mundo.definirOperadores(criarSelecao(selNome), criarCrossover(crossNome),
                              criarMutacao(mutNome));
      std::cout << "Experimento iniciado na época 0.\n";
      return 0;
    }
    if (argc >= 4 && std::string(argv[1]) == "--step") {
      Mundo mundo(argv[2]);

      // Permite manter os operadores escolhidos se fornecidos via CLI
      std::string crossNome = (argc > 4) ? argv[4] : "OX";
      std::string mutNome = (argc > 5) ? argv[5] : "Inversão";
      std::string selNome = (argc > 6) ? argv[6] : "Torneio";

      mundo.definirOperadores(criarSelecao(selNome), criarCrossover(crossNome),
                              criarMutacao(mutNome));

      mundo.avancar(std::stoi(argv[3]));
      std::cout << "Época atual: " << mundo.epocaAtual()
                << (mundo.concluido() ? " (experimento concluído)" : "")
                << '\n';
      return 0;
    }
    return 2;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
