#include "mutacao.hpp"
#include <algorithm>
#include <random>

namespace {
std::mt19937 &Gerador() {
  static thread_local std::mt19937 gerador(std::random_device{}());
  return gerador;
}
} // namespace

void Mutacao::Mutar(std::vector<int> &genetica, double taxa) const {
  std::bernoulli_distribution deveMutar(taxa);
  if (genetica.size() > 1 && deveMutar(Gerador()))
    Alterar(genetica);
}

void Mutacao::SortearPosicoes(int n, int &i, int &j) {
  i = std::uniform_int_distribution<int>(0, n - 1)(Gerador());
  j = std::uniform_int_distribution<int>(0, n - 2)(Gerador());
  if (j >= i)
    j++;
  if (i > j)
    std::swap(i, j);
}

// Inversão

void MutacaoInversao::Alterar(std::vector<int> &genetica) const {
  int n = static_cast<int>(genetica.size());
  int i, j;
  do {
    SortearPosicoes(n, i, j);
  } while (j - i >= n - 2);
  std::reverse(genetica.begin() + i, genetica.begin() + j + 1);
}

std::string MutacaoInversao::Nome() const { return "Inversao"; }

// Troca

void MutacaoTroca::Alterar(std::vector<int> &genetica) const {
  int i, j;
  SortearPosicoes(static_cast<int>(genetica.size()), i, j);
  std::swap(genetica[i], genetica[j]);
}

std::string MutacaoTroca::Nome() const { return "Troca"; }

// Inserção

void MutacaoInsercao::Alterar(std::vector<int> &genetica) const {
  int n = static_cast<int>(genetica.size());
  int i, j;
  do {
    SortearPosicoes(n, i, j);
  } while (i == 0 && j == n - 1);
  if (std::bernoulli_distribution(0.5)(Gerador())) {
    std::rotate(genetica.begin() + i, genetica.begin() + i + 1,
                genetica.begin() + j + 1);
  } else {
    std::rotate(genetica.begin() + i, genetica.begin() + j,
                genetica.begin() + j + 1);
  }
}

std::string MutacaoInsercao::Nome() const { return "Insercao"; }
