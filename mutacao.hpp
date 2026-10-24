#ifndef MUTACAO_HPP
#define MUTACAO_HPP

#include "cidade.hpp"
#include <string>
#include <vector>

// Interface dos tipos de mutação. Mutar() decide, com probabilidade "taxa", se
// a rota sofre mutação; cada tipo concreto define apenas como a rota é alterada
// (Alterar).
class Mutacao {
public:
  virtual ~Mutacao() = default;
  void Mutar(std::vector<int> &genetica, double taxa) const;
  virtual std::string Nome() const = 0;

protected:
  virtual void Alterar(std::vector<int> &genetica) const = 0;

  // Sorteia duas posições distintas i < j da rota.
  static void SortearPosicoes(int n, int &i, int &j);
};

// Inversão: inverte a ordem do trecho entre duas posições. Altera apenas 2
// arestas da rota (movimento 2-opt).
class MutacaoInversao : public Mutacao {
public:
  std::string Nome() const override;

protected:
  void Alterar(std::vector<int> &genetica) const override;
};

// Troca: troca duas cidades de lugar. Altera até 4 arestas da rota.
class MutacaoTroca : public Mutacao {
public:
  std::string Nome() const override;

protected:
  void Alterar(std::vector<int> &genetica) const override;
};

// Inserção: retira uma cidade da rota e a insere em outra posição. Altera 3
// arestas da rota.
class MutacaoInsercao : public Mutacao {
public:
  std::string Nome() const override;

protected:
  void Alterar(std::vector<int> &genetica) const override;
};

#endif // MUTACAO_HPP
