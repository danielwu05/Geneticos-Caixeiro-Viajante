#include "mundo.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

constexpr double PI = 3.14159265358979323846;

void escreverRota(std::ostream &saida, const std::vector<int> &rota) {
  for (std::size_t i = 0; i < rota.size(); ++i) {
    if (i > 0) {
      saida << ' ';
    }
    saida << rota[i];
  }
}

std::mt19937 &obterGeradorThread() {
  static thread_local std::mt19937 geradorLocal(std::random_device{}());
  return geradorLocal;
}

} // namespace

// Cria cidades, população inicial e arquivos do experimento.
Mundo::Mundo(int quantidadeCidades, int tamanhoPopulacao, double taxaMutacao,
             int maxEpocas, int limiteSemMelhora, const std::string &cenario,
             const std::string &diretorio)
    : taxaMutacao(taxaMutacao), tamanhoPopulacao(tamanhoPopulacao),
      maxEpocas(maxEpocas), limiteSemMelhora(limiteSemMelhora),
      diretorio(diretorio), gerador(std::random_device{}()) {
  if (quantidadeCidades < 3 || tamanhoPopulacao < 2 || taxaMutacao < 0.0 ||
      taxaMutacao > 1.0) {
    throw std::invalid_argument(
        "Parâmetros inválidos para iniciar o algoritmo.");
  }

  std::filesystem::create_directories(diretorio);
  criarCidades(quantidadeCidades, cenario);
  calcularMatrizDistancias();
  criarPopulacaoInicial(quantidadeCidades);
  encontrarMelhorRotaInicial();

  salvarResultados();
  salvarEstado();
}

// Restaura os dados salvos para continuar um experimento.
Mundo::Mundo(const std::string &diretorio)
    : diretorio(diretorio), gerador(std::random_device{}()) {
  std::ifstream arquivo(std::filesystem::path(diretorio) / "estado.dat");
  if (!arquivo) {
    throw std::runtime_error(
        "Não encontrei estado.dat; inicie um experimento primeiro.");
  }

  int quantidadeCidades = 0;
  arquivo >> epoca >> epocasSemMelhora >> melhorDistancia >> taxaMutacao >>
      tamanhoPopulacao >> maxEpocas >> limiteSemMelhora >> quantidadeCidades;

  if (!arquivo || quantidadeCidades < 3) {
    throw std::runtime_error(
        "O arquivo de estado do experimento está inválido.");
  }

  for (int i = 0; i < quantidadeCidades; ++i) {
    int indice = 0;
    double x = 0.0;
    double y = 0.0;
    arquivo >> indice >> x >> y;
    cidades.emplace_back(indice, x, y);
  }
  calcularMatrizDistancias();

  int quantidadeIndividuos = 0;
  arquivo >> quantidadeIndividuos;
  for (int i = 0; i < quantidadeIndividuos; ++i) {
    std::vector<int> rota(quantidadeCidades);
    for (int &cidade : rota) {
      arquivo >> cidade;
    }
    individuos.push_back(std::move(rota));
  }

  int tamanhoMelhorRota = 0;
  arquivo >> tamanhoMelhorRota;
  melhorRota.resize(tamanhoMelhorRota);
  for (int &cidade : melhorRota) {
    arquivo >> cidade;
  }

  if (!arquivo) {
    throw std::runtime_error(
        "O arquivo de estado do experimento está truncado.");
  }

  // Estados antigos podem não conter o tempo acumulado.
  if (!(arquivo >> tempoAcumulado)) {
    tempoAcumulado = 0.0;
  }
}

void Mundo::criarCidades(int quantidadeCidades, const std::string &cenario) {
  std::uniform_real_distribution<double> coordenada(0.0, 100.0);

  for (int indice = 0; indice < quantidadeCidades; ++indice) {
    double x = 0.0;
    double y = 0.0;

    if (cenario == "circular") {
      const double angulo = 2.0 * PI * indice / quantidadeCidades;
      x = 50.0 + 40.0 * std::cos(angulo);
      y = 50.0 + 40.0 * std::sin(angulo);
    } else {
      x = coordenada(gerador);
      y = coordenada(gerador);
    }

    cidades.emplace_back(indice, x, y);
  }
}

void Mundo::calcularMatrizDistancias() {
  const std::size_t n = cidades.size();
  matrizDistancias.assign(n * n, 0.0);

  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = i + 1; j < n; ++j) {
      const double dx = cidades[i].X() - cidades[j].X();
      const double dy = cidades[i].Y() - cidades[j].Y();
      const double dist = std::sqrt(dx * dx + dy * dy);
      matrizDistancias[i * n + j] = dist;
      matrizDistancias[j * n + i] = dist;
    }
  }
}

void Mundo::criarPopulacaoInicial(int quantidadeCidades) {
  std::vector<int> rota(quantidadeCidades);
  std::iota(rota.begin(), rota.end(), 0);

  for (int i = 0; i < tamanhoPopulacao; ++i) {
    std::shuffle(rota.begin(), rota.end(), gerador);
    individuos.push_back(rota);
  }
}

void Mundo::encontrarMelhorRotaInicial() {
  melhorRota = individuos.front();
  melhorDistancia = distancia(melhorRota);

  for (const auto &individuo : individuos) {
    const double distanciaIndividuo = distancia(individuo);
    if (distanciaIndividuo < melhorDistancia) {
      melhorDistancia = distanciaIndividuo;
      melhorRota = individuo;
    }
  }
}

double Mundo::distancia(const std::vector<int> &rota) const {
  double distanciaTotal = 0.0;
  const std::size_t n = rota.size();

  for (std::size_t i = 0; i < n - 1; ++i) {
    distanciaTotal += matrizDistancias[rota[i] * n + rota[i + 1]];
  }

  distanciaTotal += matrizDistancias[rota[n - 1] * n + rota[0]];

  return distanciaTotal;
}

double Mundo::aptidao(const std::vector<int> &rota) const {
  return 1.0 / (1.0 + distancia(rota));
}

std::size_t Mundo::torneio(const std::vector<double> &distancias) const {
  auto &rng = obterGeradorThread();
  std::uniform_int_distribution<std::size_t> sortearIndividuo(
      0, individuos.size() - 1);

  std::size_t melhorIndice = sortearIndividuo(rng);

  for (int i = 1; i < 4; ++i) {
    const std::size_t candidato = sortearIndividuo(rng);

    if (distancias[candidato] < distancias[melhorIndice]) {
      melhorIndice = candidato;
    }
  }

  return melhorIndice;
}

std::vector<int> Mundo::cruzar(const std::vector<int> &paiA,
                               const std::vector<int> &paiB) const {
  auto &rng = obterGeradorThread();
  const int n = static_cast<int>(paiA.size());

  std::uniform_int_distribution<int> sortearCorte(0, n - 1);

  int inicioTrecho = sortearCorte(rng);
  int fimTrecho = sortearCorte(rng);
  if (inicioTrecho > fimTrecho) {
    std::swap(inicioTrecho, fimTrecho);
  }

  std::vector<int> filho(n, -1);
  std::vector<uint8_t> usado(n, 0);

  for (int i = inicioTrecho; i <= fimTrecho; ++i) {
    filho[i] = paiA[i];
    usado[paiA[i]] = 1;
  }

  int proximaPosicao = (fimTrecho + 1) % n;

  for (int i = 0; i < n; ++i) {
    const int indicePaiB = (fimTrecho + 1 + i) % n;
    const int gene = paiB[indicePaiB];

    if (!usado[gene]) {
      filho[proximaPosicao] = gene;
      proximaPosicao = (proximaPosicao + 1) % n;
    }
  }

  return filho;
}

void Mundo::mutar(std::vector<int> &rota) const {
  auto &rng = obterGeradorThread();
  std::bernoulli_distribution deveMutar(taxaMutacao);
  if (rota.size() <= 1 || !deveMutar(rng)) {
    return;
  }

  std::uniform_int_distribution<std::size_t> sortearPosicao(0, rota.size() - 1);
  const std::size_t posicaoA = sortearPosicao(rng);
  const std::size_t posicaoB = sortearPosicao(rng);
  std::swap(rota[posicaoA], rota[posicaoB]);
}
void Mundo::avancar(int quantidadeEpocas) {
  const auto inicioExecucao = std::chrono::steady_clock::now();
  const double tempoBase = tempoAcumulado;

  std::vector<double> distancias(individuos.size());

  const std::size_t tamPop = individuos.size();

#pragma omp parallel for
  for (std::size_t i = 0; i < tamPop; ++i) {
    distancias[i] = distancia(individuos[i]);
  }
  for (int passo = 0; passo < quantidadeEpocas && !concluido(); ++passo) {

    distancias.resize(tamPop);

    const auto itMelhor =
        std::min_element(distancias.begin(), distancias.end());

    const std::size_t indiceMelhor =
        static_cast<std::size_t>(std::distance(distancias.begin(), itMelhor));

    std::vector<std::vector<int>> novaPopulacao(tamanhoPopulacao);
    std::vector<double> novasDistancias(tamanhoPopulacao);

    novaPopulacao[0] = individuos[indiceMelhor];
    novasDistancias[0] = distancias[indiceMelhor];

#pragma omp parallel for
    for (int i = 1; i < tamanhoPopulacao; ++i) {
      const std::size_t indiceA = torneio(distancias);
      const std::size_t indiceB = torneio(distancias);
      std::vector<int> filho = cruzar(individuos[indiceA], individuos[indiceB]);
      mutar(filho);
      novasDistancias[i] = distancia(filho);
      novaPopulacao[i] = std::move(filho);
    }

    individuos = std::move(novaPopulacao);
    distancias = std::move(novasDistancias);
    ++epoca;
    const double tempoAtual =
        tempoBase + std::chrono::duration<double>(
                        std::chrono::steady_clock::now() - inicioExecucao)
                        .count();
    registrarEpoca(tempoAtual, distancias);
  }

  tempoAcumulado =
      tempoBase + std::chrono::duration<double>(
                      std::chrono::steady_clock::now() - inicioExecucao)
                      .count();
  salvarEstado();
}

void Mundo::registrarEpoca(double tempoAtual,
                           const std::vector<double> &distancias) {
  double somaDistancias = 0.0;
  std::size_t indiceMelhorGeracao = 0;
  double melhorDistanciaGeracao = distancias[0];
  bool encontrouMelhora = false;

  const double melhorDistanciaAnterior = melhorDistancia;

  for (std::size_t i = 0; i < individuos.size(); ++i) {
    const double distanciaIndividuo = distancias[i];
    somaDistancias += distanciaIndividuo;

    if (distanciaIndividuo < melhorDistanciaGeracao) {
      melhorDistanciaGeracao = distanciaIndividuo;
      indiceMelhorGeracao = i;
    }

    if (distanciaIndividuo + 1e-9 < melhorDistancia) {
      melhorDistancia = distanciaIndividuo;
      melhorRota = individuos[i];
      encontrouMelhora = true;
    }
  }

  epocasSemMelhora = encontrouMelhora ? 0 : epocasSemMelhora + 1;

  const double variacaoMelhor = melhorDistancia - melhorDistanciaAnterior;
  const double mediaAtual = somaDistancias / individuos.size();
  static double mediaAnterior = mediaAtual;
  const double variacaoMedia = mediaAtual - mediaAnterior;

  mediaAnterior = mediaAtual;

  std::ofstream historico(std::filesystem::path(diretorio) / "historico.csv",
                          std::ios::app);
  historico << epoca << ',' << std::setprecision(12) << melhorDistancia << ','
            << mediaAtual << ',' << variacaoMelhor << ',' << variacaoMedia
            << ",\"";
  escreverRota(historico, melhorRota);
  historico << "\"," << melhorDistanciaGeracao << ",\"";
  escreverRota(historico, individuos[indiceMelhorGeracao]);
  historico << "\"," << tempoAtual << '\n';

  tempoAcumulado = tempoAtual;
}

bool Mundo::concluido() const {
  return epoca >= maxEpocas || epocasSemMelhora >= limiteSemMelhora;
}

int Mundo::epocaAtual() const { return epoca; }

void Mundo::salvarResultados() const {
  std::ofstream arquivoCidades(std::filesystem::path(diretorio) /
                               "cidades.csv");
  arquivoCidades << "indice,x,y\n" << std::setprecision(12);
  for (const auto &cidade : cidades) {
    arquivoCidades << cidade.Indice() << ',' << cidade.X() << ',' << cidade.Y()
                   << '\n';
  }

  std::ofstream historico(std::filesystem::path(diretorio) / "historico.csv");
  historico << "epoca,melhor_distancia,distancia_media,distancia_delta,"
               "distancia_delta_media,melhor_rota,"
               "melhor_geracao,rota_geracao,tempo_segundos\n";

  double somaDistancias = 0.0;
  for (const auto &individuo : individuos) {
    somaDistancias += distancia(individuo);
  }

  historico << "0," << std::setprecision(12) << melhorDistancia << ','
            << somaDistancias / individuos.size() << ",0.0,0.0,\"";
  escreverRota(historico, melhorRota);
  historico << "\"," << melhorDistancia << ",\"";
  escreverRota(historico, melhorRota);
  historico << "\"," << tempoAcumulado << '\n';
}

void Mundo::salvarEstado() const {
  std::ofstream arquivo(std::filesystem::path(diretorio) / "estado.dat");

  arquivo << epoca << ' ' << epocasSemMelhora << ' ' << std::setprecision(17)
          << melhorDistancia << ' ' << taxaMutacao << ' ' << tamanhoPopulacao
          << ' ' << maxEpocas << ' ' << limiteSemMelhora << ' '
          << cidades.size() << '\n';

  for (const auto &cidade : cidades) {
    arquivo << cidade.Indice() << ' ' << cidade.X() << ' ' << cidade.Y()
            << '\n';
  }

  arquivo << individuos.size() << '\n';
  for (const auto &rota : individuos) {
    escreverRota(arquivo, rota);
    arquivo << '\n';
  }

  arquivo << melhorRota.size();
  for (int cidade : melhorRota) {
    arquivo << ' ' << cidade;
  }
  arquivo << '\n' << tempoAcumulado << '\n';
}
