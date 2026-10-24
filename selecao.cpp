#include "selecao.hpp"
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <random>
#include <vector>

namespace {
std::mt19937 &Gerador() {
  static thread_local std::mt19937 gerador(std::random_device{}());
  return gerador;
}
} // namespace

std::vector<size_t>
Selecao::SelecionarVarios(const std::vector<double> &distancias,
                          int quantidade) const {
  std::vector<std::size_t> selecionados;
  selecionados.reserve(quantidade);
  for (int k = 0; k < quantidade; ++k) {
    selecionados.push_back(Selecionar(distancias));
  }
  return selecionados;
}

SelecaoTorneio::SelecaoTorneio(int tamanho) : tamanho(tamanho) {}

const std::size_t
SelecaoTorneio::Selecionar(const std::vector<double> &distancias) const {
  std::uniform_int_distribution<std::size_t> sort(0, distancias.size() - 1);
  std::size_t melhor = sort(Gerador());
  for (int k = 1; k < tamanho; ++k) {
    std::size_t cand = sort(Gerador());
    if (distancias[cand] < distancias[melhor])
      melhor = cand;
  }
  return melhor;
}
std::string SelecaoTorneio::Nome() const { return "Torneio"; }

const std::size_t
SelecaoRoleta::Selecionar(const std::vector<double> &distancias) const {
  double total = 0.0;
  for (double d : distancias)
    total += (1.0 / (1.0 + d));

  double alvo = std::uniform_real_distribution<double>(0.0, total)(Gerador());
  double acum = 0.0;
  for (std::size_t i = 0; i < distancias.size(); ++i) {
    acum += (1.0 / (1.0 + distancias[i]));
    if (alvo <= acum)
      return i;
  }
  return distancias.size() - 1;
}
std::string SelecaoRoleta::Nome() const { return "Roleta"; }

const std::size_t
SelecaoRanking::Selecionar(const std::vector<double> &distancias) const {
  const std::size_t n = distancias.size();
  std::vector<std::size_t> ordem(n);
  std::iota(ordem.begin(), ordem.end(), 0);
  std::sort(ordem.begin(), ordem.end(), [&](std::size_t a, std::size_t b) {
    return distancias[a] > distancias[b]; // Piores primeiro (peso menor)
  });

  double total = n * (n + 1) / 2.0;
  double alvo = std::uniform_real_distribution<double>(0.0, total)(Gerador());
  double acum = 0.0;
  for (std::size_t k = 0; k < n; ++k) {
    acum += (k + 1);
    if (alvo <= acum)
      return ordem[k];
  }
  return ordem.back();
}

std::string SelecaoRanking::Nome() const { return "Ranking"; }

SelecaoTopRanking::SelecaoTopRanking(double fracao) : fracao(fracao) {}

const std::size_t
SelecaoTopRanking::Selecionar(const std::vector<double> &distancias) const {
  const std::size_t n = distancias.size();
  std::size_t qtdTopo =
      std::max(std::size_t{1}, static_cast<std::size_t>(fracao * n));

  std::vector<std::size_t> ordem(n);
  std::iota(ordem.begin(), ordem.end(), 0);

  std::nth_element(ordem.begin(), ordem.begin() + (qtdTopo - 1), ordem.end(),
                   [&](std::size_t a, std::size_t b) {
                     return distancias[a] <
                            distancias[b]; // Menor distância = melhor
                   });

  std::uniform_int_distribution<std::size_t> sorteio(0, qtdTopo - 1);
  return ordem[sorteio(Gerador())];
}
std::string SelecaoTopRanking::Nome() const {
  return "TopRanking(" + std::to_string(std::lround(fracao * 100)) + "%)";
}

// --- 5. Amostragem Estocástica Universal (SUS) ---
const std::size_t SelecaoAmostragemEstocastica::Selecionar(
    const std::vector<double> &distancias) const {
  return SelecionarVarios(distancias, 1)[0];
}

std::vector<std::size_t> SelecaoAmostragemEstocastica::SelecionarVarios(
    const std::vector<double> &distancias, int quantidade) const {

  double totalAptidao = 0.0;
  for (double d : distancias) {
    totalAptidao += (1.0 / (1.0 + d));
  }

  double passo = totalAptidao / quantidade;
  double ponteiro =
      std::uniform_real_distribution<double>(0.0, passo)(Gerador());

  std::vector<std::size_t> selecionados;
  selecionados.reserve(quantidade);
  double acumulado = 0.0;

  for (std::size_t i = 0; i < distancias.size(); ++i) {
    acumulado += (1.0 / (1.0 + distancias[i]));
    while (static_cast<int>(selecionados.size()) < quantidade &&
           ponteiro <= acumulado) {
      selecionados.push_back(i);
      ponteiro += passo;
    }
  }

  while (static_cast<int>(selecionados.size()) < quantidade) {
    selecionados.push_back(distancias.size() - 1);
  }

  std::shuffle(selecionados.begin(), selecionados.end(), Gerador());
  return selecionados;
}
std::string SelecaoAmostragemEstocastica::Nome() const {
  return "AmostragemEstocastica";
}
