#include "mundo.hpp"
#include "selecao.hpp"
#include <filesystem>
#include <iostream>

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

      // Operadores opcionais passados via CLI (ex: --start ... OX Inversao
      // Torneio)
      std::string crossNome = (argc > 9) ? argv[9] : "OX";
      std::string mutNome = (argc > 10) ? argv[10] : "Inversão";
      std::string selNome = (argc > 11) ? argv[11] : "Torneio";

      std::unique_ptr<Crossover> cross;
      if (crossNome == "PMX")
        cross = std::make_unique<CrossoverPMX>();
      else if (crossNome == "ERX")
        cross = std::make_unique<CrossoverERX>();
      else if (crossNome == "Uniforme")
        cross = std::make_unique<CrossoverUniforme>();
      else
        cross = std::make_unique<CrossoverOX>();

      std::unique_ptr<Mutacao> mutOp;
      if (mutNome == "Troca")
        mutOp = std::make_unique<MutacaoTroca>();
      else if (mutNome == "Inserção")
        mutOp = std::make_unique<MutacaoInsercao>();
      else
        mutOp = std::make_unique<MutacaoInversao>();

      std::unique_ptr<Selecao> selOp;
      if (selNome == "Roleta")
        selOp = std::make_unique<SelecaoRoleta>();
      else if (selNome == "Ranking")
        selOp = std::make_unique<SelecaoRanking>();
      else if (selNome == "Top Ranking")
        selOp = std::make_unique<SelecaoTopRanking>(0.2);
      else if (selNome == "Amostragem Estocástica")
        selOp = std::make_unique<SelecaoAmostragemEstocastica>();
      else
        selOp = std::make_unique<SelecaoTorneio>(4);
      mundo.definirOperadores(std::move(selOp), std::move(cross),
                              std::move(mutOp));
      std::cout << "Experimento iniciado na época 0.\n";
      return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--step") {
      Mundo mundo(argv[2]);
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
