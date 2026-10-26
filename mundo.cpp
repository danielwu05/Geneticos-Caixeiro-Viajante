#include "mundo.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr double PI = 3.14159265358979323846;

void escreverRota(std::ostream &saida, const std::vector<int> &rota) {
  for (std::size_t i = 0; i < rota.size(); ++i) {
    if (i > 0)
      saida << ' ';
    saida << rota[i];
  }
}
} // namespace

Mundo::Mundo(int quantidadeCidades, int tamanhoPopulacao, double taxaMutacao,
             int maxEpocas, int limiteSemMelhora, const std::string &cenario,
             const std::string &diretorio)
    : cenario(cenario), taxaMutacao(taxaMutacao),
      tamanhoPopulacao(tamanhoPopulacao), maxEpocas(maxEpocas),
      limiteSemMelhora(limiteSemMelhora), diretorio(diretorio),
      gerador(std::random_device{}()),
      operadorSelecao(std::make_unique<SelecaoTorneio>(4)),
      operadorCrossover(std::make_unique<CrossoverOX>()),
      operadorMutacao(std::make_unique<MutacaoInversao>()) {

  if (quantidadeCidades < 3 || tamanhoPopulacao < 2 || taxaMutacao < 0.0 ||
      taxaMutacao > 1.0) {
    throw std::invalid_argument(
        "Parâmetros inválidos para iniciar o algoritmo.");
  }

  std::filesystem::create_directories(diretorio);
  const auto caminhoCidades = std::filesystem::path(diretorio) / "cidades.csv";
  bool reaproveitar = false;

  if (std::filesystem::exists(caminhoCidades)) {
    std::ifstream arq(caminhoCidades);
    std::string primeiraLinha;
    std::getline(arq, primeiraLinha); // Lê "#cenario:circular" ou "indice,x,y"

    bool mesmoCenario = true;
    if (primeiraLinha.rfind("#cenario:", 0) == 0) {
      std::string cenarioSalvo = primeiraLinha.substr(9);
      mesmoCenario = (cenarioSalvo == cenario);
      std::getline(arq, primeiraLinha); // consome "indice,x,y"
    }

    if (mesmoCenario) {
      std::vector<Cidade> cidadesLidas;
      int idx;
      char sep1, sep2;
      double x, y;
      while (arq >> idx >> sep1 >> x >> sep2 >> y) {
        cidadesLidas.emplace_back(idx, x, y);
      }

      if (static_cast<int>(cidadesLidas.size()) == quantidadeCidades) {
        cidades = std::move(cidadesLidas);
        reaproveitar = true;
      }
    }
  }

  if (!reaproveitar) {
    cidades.clear();
    criarCidades(quantidadeCidades, cenario);
  }

  calcularMatrizDistancias();
  criarPopulacaoInicial(static_cast<int>(cidades.size()));
  encontrarMelhorRotaInicial();

  salvarResultados();
  salvarEstado();
}

Mundo::Mundo(const std::string &diretorio)
    : diretorio(diretorio), gerador(std::random_device{}()),
      operadorSelecao(std::make_unique<SelecaoTorneio>(4)),
      operadorCrossover(std::make_unique<CrossoverOX>()),
      operadorMutacao(std::make_unique<MutacaoInversao>()) {

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

  cidades.reserve(quantidadeCidades);
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
  individuos.reserve(quantidadeIndividuos);
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

  if (!arquivo || !(arquivo >> tempoAcumulado)) {
    tempoAcumulado = 0.0;
  }
}

void Mundo::definirOperadores(std::unique_ptr<Selecao> sel,
                              std::unique_ptr<Crossover> cross,
                              std::unique_ptr<Mutacao> mut) {
  if (sel)
    operadorSelecao = std::move(sel);
  if (cross)
    operadorCrossover = std::move(cross);
  if (mut)
    operadorMutacao = std::move(mut);
}

void Mundo::criarCidades(int quantidadeCidades, const std::string &cenario) {
  std::uniform_real_distribution<double> coordenada(0.0, 100.0);
  cidades.reserve(quantidadeCidades);

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

  individuos.reserve(tamanhoPopulacao);
  for (int i = 0; i < tamanhoPopulacao; ++i) {
    std::shuffle(rota.begin(), rota.end(), gerador);
    individuos.push_back(rota);
  }
}

void Mundo::encontrarMelhorRotaInicial() {
  melhorRota = individuos.front();
  melhorDistancia = distancia(melhorRota);

  for (const auto &ind : individuos) {
    const double d = distancia(ind);
    if (d < melhorDistancia) {
      melhorDistancia = d;
      melhorRota = ind;
    }
  }
}

double Mundo::distancia(const std::vector<int> &rota) const {
  const std::size_t n = rota.size();
  double total = 0.0;
  for (std::size_t i = 0; i < n - 1; ++i) {
    total += matrizDistancias[static_cast<std::size_t>(rota[i]) * n +
                              static_cast<std::size_t>(rota[i + 1])];
  }
  total += matrizDistancias[static_cast<std::size_t>(rota[n - 1]) * n +
                            static_cast<std::size_t>(rota[0])];
  return total;
}

double Mundo::Distancia(int i, int j) const {
  if (i == j)
    return 0.0;
  const std::size_t n = cidades.size();
  return matrizDistancias[static_cast<std::size_t>(i) * n +
                          static_cast<std::size_t>(j)];
}

void Mundo::avancar(int quantidadeEpocas) {
  const auto inicio = std::chrono::steady_clock::now();
  const double baseTempo = tempoAcumulado;
  const std::size_t tamPop = individuos.size();

  std::vector<double> distancias(tamPop);
#pragma omp parallel for
  for (std::size_t i = 0; i < tamPop; ++i) {
    distancias[i] = distancia(individuos[i]);
  }

  for (int passo = 0; passo < quantidadeEpocas && !concluido(); ++passo) {
    const auto itMelhor =
        std::min_element(distancias.begin(), distancias.end());
    const std::size_t idxMelhor =
        static_cast<std::size_t>(std::distance(distancias.begin(), itMelhor));

    std::vector<std::vector<int>> novaPopulacao(tamanhoPopulacao);
    std::vector<double> novasDistancias(tamanhoPopulacao);

    // Elitismo
    novaPopulacao[0] = individuos[idxMelhor];
    novasDistancias[0] = distancias[idxMelhor];

#pragma omp parallel for
    for (int i = 1; i < tamanhoPopulacao; ++i) {
      std::size_t pA = operadorSelecao->Selecionar(distancias);
      std::size_t pB = operadorSelecao->Selecionar(distancias);

      std::vector<int> filho =
          operadorCrossover->Cruzar(individuos[pA], individuos[pB]);
      operadorMutacao->Mutar(filho, taxaMutacao);

      novasDistancias[i] = distancia(filho);
      novaPopulacao[i] = std::move(filho);
    }

    individuos = std::move(novaPopulacao);
    distancias = std::move(novasDistancias);
    ++epoca;

    const double tAtual =
        baseTempo +
        std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio)
            .count();
    registrarEpoca(tAtual, distancias);
  }

  tempoAcumulado = baseTempo + std::chrono::duration<double>(
                                   std::chrono::steady_clock::now() - inicio)
                                   .count();
  salvarEstado();
}

void Mundo::registrarEpoca(double tempoAtual,
                           const std::vector<double> &distancias) {
  double soma = 0.0;
  std::size_t idxMelhorGen = 0;
  double menorGen = distancias[0];
  bool melhora = false;

  const double melhorAnt = melhorDistancia;

  for (std::size_t i = 0; i < distancias.size(); ++i) {
    const double d = distancias[i];
    soma += d;
    if (d < menorGen) {
      menorGen = d;
      idxMelhorGen = i;
    }
    if (d + 1e-9 < melhorDistancia) {
      melhorDistancia = d;
      melhorRota = individuos[i];
      melhora = true;
    }
  }

  epocasSemMelhora = melhora ? 0 : epocasSemMelhora + 1;
  const double mediaAtual = soma / distancias.size();
  if (epoca == 1)
    mediaAnterior = mediaAtual;

  const double deltaMelhor = melhorDistancia - melhorAnt;
  const double deltaMedia = mediaAtual - mediaAnterior;
  mediaAnterior = mediaAtual;

  std::ofstream arq(std::filesystem::path(diretorio) / "historico.csv",
                    std::ios::app);
  arq << epoca << ',' << std::setprecision(12) << melhorDistancia << ','
      << mediaAtual << ',' << deltaMelhor << ',' << deltaMedia << ",\"";
  escreverRota(arq, melhorRota);
  arq << "\"," << menorGen << ",\"";
  escreverRota(arq, individuos[idxMelhorGen]);
  arq << "\"," << tempoAtual << '\n';

  tempoAcumulado = tempoAtual;
}

bool Mundo::concluido() const {
  return epoca >= maxEpocas || epocasSemMelhora >= limiteSemMelhora;
}

int Mundo::epocaAtual() const { return epoca; }

void Mundo::salvarResultados() {
  std::ofstream arqCidades(std::filesystem::path(diretorio) / "cidades.csv");
  arqCidades << "#cenario:" << cenario << "\n";
  arqCidades << "indice,x,y\n" << std::setprecision(12);
  for (const auto &c : cidades) {
    arqCidades << c.Id() << ',' << c.X() << ',' << c.Y() << '\n';
  }

  std::ofstream hist(std::filesystem::path(diretorio) / "historico.csv");
  hist << "epoca,melhor_distancia,distancia_media,distancia_delta,"
          "distancia_delta_media,melhor_rota,melhor_geracao,rota_geracao,tempo_"
          "segundos\n";

  double soma = 0.0;
  for (const auto &ind : individuos)
    soma += distancia(ind);
  mediaAnterior = soma / individuos.size();

  hist << "0," << std::setprecision(12) << melhorDistancia << ','
       << mediaAnterior << ",0.0,0.0,\"";
  escreverRota(hist, melhorRota);
  hist << "\"," << melhorDistancia << ",\"";
  escreverRota(hist, melhorRota);
  hist << "\"," << tempoAcumulado << '\n';
}

void Mundo::salvarEstado() const {
  std::ofstream arq(std::filesystem::path(diretorio) / "estado.dat");
  arq << epoca << ' ' << epocasSemMelhora << ' ' << std::setprecision(17)
      << melhorDistancia << ' ' << taxaMutacao << ' ' << tamanhoPopulacao << ' '
      << maxEpocas << ' ' << limiteSemMelhora << ' ' << cidades.size() << '\n';

  for (const auto &c : cidades) {
    arq << c.Id() << ' ' << c.X() << ' ' << c.Y() << '\n';
  }

  arq << individuos.size() << '\n';
  for (const auto &r : individuos) {
    escreverRota(arq, r);
    arq << '\n';
  }

  arq << melhorRota.size();
  for (int c : melhorRota)
    arq << ' ' << c;
  arq << '\n' << tempoAcumulado << '\n';
}
