#ifndef MUNDO_HPP
#define MUNDO_HPP

#include "cidade.hpp"
#include "crossover.hpp"
#include "mutacao.hpp"
#include "selecao.hpp"

#include <memory>
#include <random>
#include <string>
#include <vector>

class Mundo {
private:
  std::string cenario;
  std::vector<Cidade> cidades;
  std::vector<double> matrizDistancias;
  std::vector<std::vector<int>> individuos;
  int epoca = 0;
  int epocasSemMelhora = 0;
  double melhorDistancia = 0.0;
  double mediaAnterior = 0.0;
  std::vector<int> melhorRota;
  double tempoAcumulado = 0.0;
  double taxaMutacao = 0.02;
  int tamanhoPopulacao = 100;
  int maxEpocas = 500;
  int limiteSemMelhora = 100;
  std::string diretorio;
  std::mt19937 gerador;

  // Operadores configuráveis polimórficos
  std::unique_ptr<Selecao> operadorSelecao;
  std::unique_ptr<Crossover> operadorCrossover;
  std::unique_ptr<Mutacao> operadorMutacao;

  void criarCidades(int quantidadeCidades, const std::string &cenario);
  void calcularMatrizDistancias();
  void criarPopulacaoInicial(int quantidadeCidades);
  void encontrarMelhorRotaInicial();
  void registrarEpoca(double tempoAtual, const std::vector<double> &distancias);
  void salvarEstado() const;
  void salvarResultados();

public:
  Mundo(int quantidadeCidades, int tamanhoPopulacao, double taxaMutacao,
        int maxEpocas, int limiteSemMelhora, const std::string &cenario,
        const std::string &diretorio);
  explicit Mundo(const std::string &diretorio);

  double distancia(const std::vector<int> &rota) const;
  double Distancia(int i, int j) const;

  void definirOperadores(std::unique_ptr<Selecao> sel,
                         std::unique_ptr<Crossover> cross,
                         std::unique_ptr<Mutacao> mut);

  void avancar(int quantidadeEpocas = 1);
  bool concluido() const;
  int epocaAtual() const;
};

#endif // MUNDO_HPP
