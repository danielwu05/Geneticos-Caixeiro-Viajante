#include "crossover.hpp"
#include <algorithm>
#include <cstdint>
#include <random>
#include <variant>
#include <vector>

namespace {
std::mt19937 &Gerador() {
  static thread_local std::mt19937 gerador(std::random_device{}());
  return gerador;
}

// Sorteia o trecho [a, b] (a <= b) herdado do pai.
void SortearTrecho(int n, int &a, int &b) {
  std::uniform_int_distribution<int> corte(0, n - 1);
  a = corte(Gerador());
  b = corte(Gerador());
  if (a > b)
    std::swap(a, b);
}
} // namespace

// OX

std::vector<int> CrossoverOX::Cruzar(const std::vector<int> &pai,
                                     const std::vector<int> &mae) const {
  const int n = static_cast<int>(pai.size());
  int a, b;
  SortearTrecho(n, a, b);

  std::vector<int> filho(n, -1);
  std::vector<std::uint8_t> usado(n, 0);

  for (int i = a; i <= b; ++i) {
    filho[i] = pai[i];
    usado[pai[i]] = 1;
  }

  int proxima = (b + 1) % n;
  for (int i = 0; i < n; ++i) {
    const int gene = mae[(b + 1 + i) % n];
    if (!usado[gene]) {
      filho[proxima] = gene;
      proxima = (proxima + 1) % n;
    }
  }
  return filho;
}

std::string CrossoverOX::Nome() const { return "OX"; }

// PMX

std::vector<int> CrossoverPMX::Cruzar(const std::vector<int> &pai,
                                      const std::vector<int> &mae) const {
  const int n = static_cast<int>(pai.size());
  int a, b;
  SortearTrecho(n, a, b);

  std::vector<int> posPai(n);
  for (int i = 0; i < n; ++i)
    posPai[pai[i]] = i;

  std::vector<int> noTrecho(n, 0);
  for (int i = 0; i < n; ++i)
    noTrecho[pai[i]] = 1;

  std::vector<int> filho = mae;
  for (int i = 0; i < n; ++i) {
    if (i >= a && i <= b) {
      filho[i] = pai[i];
      continue;
    }
    int gene = mae[i];
    while (noTrecho[gene]) {
      gene = mae[posPai[gene]];
    }
    filho[i] = gene;
  }
  return filho;
}

std::string CrossoverPMX::Nome() const { return "PMX"; }

// ERX

std::vector<int> CrossoverERX::Cruzar(const std::vector<int> &pai,
                                      const std::vector<int> &mae) const {
  const int n = static_cast<int>(pai.size());
  std::vector<std::vector<int>> vizinhos(n);

  auto adicionar = [&](const std::vector<int> &r) {
    for (int i = 0; i < n; ++i) {
      int u = r[i];
      int v1 = r[(i + 1) % n];
      int v2 = r[(i + n - 1) % n];
      if (std::find(vizinhos[u].begin(), vizinhos[u].end(), v1) ==
          vizinhos[u].end())
        vizinhos[u].push_back(v1);
      if (std::find(vizinhos[u].begin(), vizinhos[u].end(), v2) ==
          vizinhos[u].end())
        vizinhos[u].push_back(v2);
    }
  };
  adicionar(pai);
  adicionar(mae);

  std::vector<int> naoVisitados(n), pos(n);

  for (int i = 0; i < n; ++i) {
    naoVisitados[i] = i;
    pos[i] = i;
  }

  auto marcar = [&](int u) {
    int p = pos[u];
    int ultimo = naoVisitados.back();
    naoVisitados[p] = ultimo;
    pos[ultimo] = p;
    naoVisitados.pop_back();
  };

  std::vector<int> filho;

  filho.reserve(n);
  int atual = pai[0];

  while (true) {
    filho.push_back(atual);
    marcar(atual);
    if (naoVisitados.empty())
      break;

    for (int v : vizinhos[atual]) {
      auto &lst = vizinhos[v];
      auto it = std::find(lst.begin(), lst.end(), atual);
      if (it != lst.end())
        lst.erase(it);
    }

    if (vizinhos[atual].empty()) {
      std::uniform_int_distribution<int> sort(
          0, static_cast<int>(naoVisitados.size() - 1));
      atual = naoVisitados[sort(Gerador())];
    } else {
      int proximo = vizinhos[atual][0];
      for (int v : vizinhos[atual]) {
        if (vizinhos[v].size() < vizinhos[proximo].size()) {
          proximo = v;
        }
      }
      atual = proximo;
    }
  }
  return filho;
}

std::string CrossoverERX::Nome() const { return "ERX"; }

// Uniforme (Uniform Order-Based)

std::vector<int> CrossoverUniforme::Cruzar(const std::vector<int> &pai,
                                           const std::vector<int> &mae) const {
  const int n = static_cast<int>(pai.size());
  std::bernoulli_distribution moeda(0.5);
  std::vector<std::uint8_t> doPai(n, 0);
  std::vector<std::uint8_t> usada(n, 0);

  for (int i = 0; i < n; ++i) {
    if (moeda(Gerador())) {
      doPai[i] = 1;
      usada[pai[i]] = 1;
    }
  }

  std::vector<int> filho = pai;

  int posMae = 0;
  for (int i = 0; i < n; ++i) {
    if (doPai[i]) {
      continue;
    }
    while (usada[mae[posMae]]) {
      ++posMae;
    }
    filho[i] = mae[posMae++];
  }
  return filho;
}

std::string CrossoverUniforme::Nome() const { return "Uniforme"; }
