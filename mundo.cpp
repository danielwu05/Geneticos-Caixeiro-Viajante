#include "mundo.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {

constexpr double PI = 3.14159265358979323846;

void escreverRota(std::ostream& saida, const std::vector<int>& rota) {
    for (std::size_t i = 0; i < rota.size(); ++i) {
        if (i > 0) {
            saida << ' ';
        }
        saida << rota[i];
    }
}

}

// Cria cidades, população inicial e arquivos do experimento.
Mundo::Mundo(int quantidadeCidades, int tamanhoPopulacao, double taxaMutacao,
             int maxEpocas, int limiteSemMelhora, const std::string& cenario,
             const std::string& diretorio)
    : taxaMutacao(taxaMutacao),
      tamanhoPopulacao(tamanhoPopulacao),
      maxEpocas(maxEpocas),
      limiteSemMelhora(limiteSemMelhora),
      diretorio(diretorio),
      gerador(std::random_device{}()) {
    if (quantidadeCidades < 3 || tamanhoPopulacao < 2 ||
        taxaMutacao < 0.0 || taxaMutacao > 1.0) {
        throw std::invalid_argument(
            "Parâmetros inválidos para iniciar o algoritmo.");
    }

    std::filesystem::create_directories(diretorio);
    criarCidades(quantidadeCidades, cenario);
    criarPopulacaoInicial(quantidadeCidades);
    encontrarMelhorRotaInicial();

    salvarResultados();
    salvarEstado();
}

// Restaura os dados salvos para continuar um experimento.
Mundo::Mundo(const std::string& diretorio)
    : diretorio(diretorio), gerador(std::random_device{}()) {
    std::ifstream arquivo(std::filesystem::path(diretorio) / "estado.dat");
    if (!arquivo) {
        throw std::runtime_error(
            "Não encontrei estado.dat; inicie um experimento primeiro.");
    }

    int quantidadeCidades = 0;
    arquivo >> epoca >> epocasSemMelhora >> melhorDistancia >> taxaMutacao
            >> tamanhoPopulacao >> maxEpocas >> limiteSemMelhora
            >> quantidadeCidades;

    if (!arquivo || quantidadeCidades < 3) {
        throw std::runtime_error("O arquivo de estado do experimento está inválido.");
    }

    for (int i = 0; i < quantidadeCidades; ++i) {
        int indice = 0;
        double x = 0.0;
        double y = 0.0;
        arquivo >> indice >> x >> y;
        cidades.emplace_back(indice, x, y);
    }

    int quantidadeIndividuos = 0;
    arquivo >> quantidadeIndividuos;
    for (int i = 0; i < quantidadeIndividuos; ++i) {
        std::vector<int> rota(quantidadeCidades);
        for (int& cidade : rota) {
            arquivo >> cidade;
        }
        individuos.push_back(std::move(rota));
    }

    int tamanhoMelhorRota = 0;
    arquivo >> tamanhoMelhorRota;
    melhorRota.resize(tamanhoMelhorRota);
    for (int& cidade : melhorRota) {
        arquivo >> cidade;
    }

    if (!arquivo) {
        throw std::runtime_error("O arquivo de estado do experimento está truncado.");
    }

    // Estados antigos podem não conter o tempo acumulado.
    if (!(arquivo >> tempoAcumulado)) {
        tempoAcumulado = 0.0;
    }
}

void Mundo::criarCidades(int quantidadeCidades, const std::string& cenario) {
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

    for (const auto& individuo : individuos) {
        const double distanciaIndividuo = distancia(individuo);
        if (distanciaIndividuo < melhorDistancia) {
            melhorDistancia = distanciaIndividuo;
            melhorRota = individuo;
        }
    }
}

double Mundo::distancia(const std::vector<int>& rota) const {
    double distanciaTotal = 0.0;

    for (std::size_t i = 0; i < rota.size(); ++i) {
        const Cidade& cidadeAtual = cidades.at(rota[i]);
        const Cidade& proximaCidade = cidades.at(rota[(i + 1) % rota.size()]);
        distanciaTotal += std::hypot(cidadeAtual.X() - proximaCidade.X(),
                                     cidadeAtual.Y() - proximaCidade.Y());
    }

    return distanciaTotal;
}

double Mundo::aptidao(const std::vector<int>& rota) const {
    return 1.0 / (1.0 + distancia(rota));
}

std::vector<int> Mundo::torneio() const {
    static thread_local std::mt19937 geradorSelecao(std::random_device{}());
    std::uniform_int_distribution<std::size_t> sortearIndividuo(
        0, individuos.size() - 1);

    std::vector<int> melhorIndividuo = individuos[sortearIndividuo(geradorSelecao)];

    for (int i = 1; i < 4; ++i) {
        const auto& candidato = individuos[sortearIndividuo(geradorSelecao)];
        if (aptidao(candidato) > aptidao(melhorIndividuo)) {
            melhorIndividuo = candidato;
        }
    }

    return melhorIndividuo;
}

std::vector<int> Mundo::cruzar(const std::vector<int>& paiA,
                               const std::vector<int>& paiB) {
    std::uniform_int_distribution<int> sortearCorte(
        0, static_cast<int>(paiA.size()) - 1);

    int inicioTrecho = sortearCorte(gerador);
    int fimTrecho = sortearCorte(gerador);
    if (inicioTrecho > fimTrecho) {
        std::swap(inicioTrecho, fimTrecho);
    }

    std::vector<int> filho(paiA.size(), -1);
    for (int i = inicioTrecho; i <= fimTrecho; ++i) {
        filho[i] = paiA[i];
    }

    int proximaPosicao = (fimTrecho + 1) % static_cast<int>(paiA.size());
    for (std::size_t i = 0; i < paiB.size(); ++i) {
        const std::size_t indicePaiB =
            (fimTrecho + 1 + static_cast<int>(i)) % paiB.size();
        const int gene = paiB[indicePaiB];

        if (std::find(filho.begin(), filho.end(), gene) == filho.end()) {
            filho[proximaPosicao] = gene;
            proximaPosicao = (proximaPosicao + 1) % static_cast<int>(paiA.size());
        }
    }

    return filho;
}

void Mundo::mutar(std::vector<int>& rota) {
    std::bernoulli_distribution deveMutar(taxaMutacao);
    if (rota.size() <= 1 || !deveMutar(gerador)) {
        return;
    }

    std::uniform_int_distribution<std::size_t> sortearPosicao(0, rota.size() - 1);
    const std::size_t posicaoA = sortearPosicao(gerador);
    const std::size_t posicaoB = sortearPosicao(gerador);
    std::swap(rota[posicaoA], rota[posicaoB]);
}
void Mundo::avancar(int quantidadeEpocas) {
    const auto inicioExecucao = std::chrono::steady_clock::now();
    const double tempoBase = tempoAcumulado;

    for (int passo = 0; passo < quantidadeEpocas && !concluido(); ++passo) {
        std::vector<std::vector<int>> novaPopulacao;
        std::vector<std::size_t> individuosOrdenados(individuos.size());
        std::iota(individuosOrdenados.begin(), individuosOrdenados.end(), 0);

        std::sort(individuosOrdenados.begin(), individuosOrdenados.end(),
                  [&](std::size_t indiceA, std::size_t indiceB) {
                      return distancia(individuos[indiceA]) <
                             distancia(individuos[indiceB]);
                  });

        //mantém a melhor rota da geração atual.
        novaPopulacao.push_back(individuos[individuosOrdenados.front()]);

        while (static_cast<int>(novaPopulacao.size()) < tamanhoPopulacao) {
            std::vector<int> paiA = torneio();
            std::vector<int> paiB = torneio();
            std::vector<int> filho = cruzar(paiA, paiB);
            mutar(filho);
            novaPopulacao.push_back(std::move(filho));
        }

        individuos = std::move(novaPopulacao);
        ++epoca;
        const double tempoAtual = tempoBase + std::chrono::duration<double>(
            std::chrono::steady_clock::now() - inicioExecucao).count();
        registrarEpoca(tempoAtual);
    }

    tempoAcumulado = tempoBase + std::chrono::duration<double>(
        std::chrono::steady_clock::now() - inicioExecucao).count();
    salvarEstado();
}

void Mundo::registrarEpoca(double tempoAtual) {
    double somaDistancias = 0.0;
    double melhorDistanciaGeracao = distancia(individuos.front());
    std::vector<int> melhorRotaGeracao = individuos.front();
    bool encontrouMelhora = false;

    for (const auto& individuo : individuos) {
        const double distanciaIndividuo = distancia(individuo);
        somaDistancias += distanciaIndividuo;

        if (distanciaIndividuo < melhorDistanciaGeracao) {
            melhorDistanciaGeracao = distanciaIndividuo;
            melhorRotaGeracao = individuo;
        }

        if (distanciaIndividuo + 1e-9 < melhorDistancia) {
            melhorDistancia = distanciaIndividuo;
            melhorRota = individuo;
            encontrouMelhora = true;
        }
    }

    epocasSemMelhora = encontrouMelhora ? 0 : epocasSemMelhora + 1;

    std::ofstream historico(std::filesystem::path(diretorio) / "historico.csv",
                            std::ios::app);
    historico << epoca << ',' << std::setprecision(12) << melhorDistancia << ','
              << somaDistancias / individuos.size() << ",\"";
    escreverRota(historico, melhorRota);
    historico << "\"," << melhorDistanciaGeracao << ",\"";
    escreverRota(historico, melhorRotaGeracao);
    historico << "\"," << tempoAtual << '\n';

    tempoAcumulado = tempoAtual;
    salvarEstado();
}

bool Mundo::concluido() const {
    return epoca >= maxEpocas || epocasSemMelhora >= limiteSemMelhora;
}

int Mundo::epocaAtual() const {
    return epoca;
}

void Mundo::salvarResultados() const {
    std::ofstream arquivoCidades(std::filesystem::path(diretorio) / "cidades.csv");
    arquivoCidades << "indice,x,y\n" << std::setprecision(12);
    for (const auto& cidade : cidades) {
        arquivoCidades << cidade.Indice() << ',' << cidade.X() << ',' << cidade.Y()
                       << '\n';
    }

    std::ofstream historico(std::filesystem::path(diretorio) / "historico.csv");
    historico << "epoca,melhor_distancia,distancia_media,melhor_rota,"
                 "melhor_geracao,rota_geracao,tempo_segundos\n";

    double somaDistancias = 0.0;
    for (const auto& individuo : individuos) {
        somaDistancias += distancia(individuo);
    }

    historico << "0," << std::setprecision(12) << melhorDistancia << ','
              << somaDistancias / individuos.size() << ",\"";
    escreverRota(historico, melhorRota);
    historico << "\"," << melhorDistancia << ",\"";
    escreverRota(historico, melhorRota);
    historico << "\"," << tempoAcumulado << '\n';
}

void Mundo::salvarEstado() const {
    std::ofstream arquivo(std::filesystem::path(diretorio) / "estado.dat");

    arquivo << epoca << ' ' << epocasSemMelhora << ' '
            << std::setprecision(17) << melhorDistancia << ' ' << taxaMutacao << ' '
            << tamanhoPopulacao << ' ' << maxEpocas << ' ' << limiteSemMelhora << ' '
            << cidades.size() << '\n';

    for (const auto& cidade : cidades) {
        arquivo << cidade.Indice() << ' ' << cidade.X() << ' ' << cidade.Y() << '\n';
    }

    arquivo << individuos.size() << '\n';
    for (const auto& rota : individuos) {
        escreverRota(arquivo, rota);
        arquivo << '\n';
    }

    arquivo << melhorRota.size();
    for (int cidade : melhorRota) {
        arquivo << ' ' << cidade;
    }
    arquivo << '\n' << tempoAcumulado << '\n';
}
