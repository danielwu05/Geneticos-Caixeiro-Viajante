#ifndef MUNDO_HPP
#define MUNDO_HPP

#include "individuo.hpp"
#include "cidade.hpp"
#include "selecao.hpp"
#include "crossover.hpp"
#include "mutacao.hpp"

#include <vector>

// Dois cenários de distribuição de cidades, conforme enunciado.
enum class DistribuicaoCidades {
    Uniforme,   // Pontos uniformemente distribuídos
    Circular    // Círculo de pontos (benchmark)
};

// Sumariza as informações de cada época.
struct Epoca {
    int indiceEpoca;                // indice da epoca.
    double melhorDistancia;         // melhor distância obtida na epoca.
    double distanciaMedia;          // distância média da epoca - parâmetro utilizado como critério de parada. Se a diferença de média entre duas épocas for menor que 0.01, deixa de valer a pena avançar ainda mais.
    double piorDistancia;           // pior distância obtida na epoca.
    std::vector<int> melhorRota;    // melhor rota obtida.

    // Escolhas selecionadas para gerar a epoca atual.
    std::string nomeSelecao;        // ex: "Torneio(3)", "Roleta"
    std::string nomeCrossover;      // ex: "ERX", "OX", "PMX"
    std::string nomeMutacao;        // ex: "Inversao", "Troca"
};


class Mundo {
private:
    std::vector<Cidade> cidades;
    std::vector<Individuo> individuos;     // Bidimensional: [epoca][individuo]
    std::vector<Epoca> eras;                            // Vetor que armazena a sumarização do resultado de cada epoca.

    int quantidadeCidades;
    int tamanhoPopulacao;
    int epocaAtual;
    double taxaDeMutacaoInicial;

    // Matriz de distâncias triangular superior.
    // distancias[i][j-i-1] com i < j armazena a distância euclidiana entre cidade i e cidade j.
    // Acesse sempre via Distancia(i, j) para normalizar os índices automaticamente.
    std::vector<std::vector<double>> distancias;

    // Aloca as cidades conforme o cenário escolhido.
    void GerarCidades(int quantidade, DistribuicaoCidades distribuicao);

    // Preenche a matriz triangular superior com as distâncias euclidianas entre todas as cidades.
    void GerarMatrizDistancias();

    // Gera a primeira gereação de individuos;
    std::vector<Individuo> PrimeiraGeracao(int quantidade, double mutacaoInicial);

    void RegistrarEpoca(int indiceEpoca, const std::string& selecao, const std::string& crossover, const std::string& mutacao);

public:
    // Inicializa o mundo: gera cidades, matriz de distâncias e primeira geração (época 0).
    // Retorna ponteiro para o Mundo criado em memória gerenciada pelo C++.
    static Mundo* Setup(int quantidadeCidades, DistribuicaoCidades distribuicao, double taxaMutacao, int tamanhoPopulacao);

    // Responsável por gerar o avanço de épocas.
    void AvancarEpoca(const Selecao& selecao, const Crossover& crossover, const Mutacao& mutacao);

    // Consulta a distância entre cidade i e cidade j.
    // Normaliza os índices automaticamente (funciona para qualquer ordem de i, j).
    double Distancia(int i, int j) const;

    // Reprodução: seleciona pai e mãe, aplica o crossover e a mutação, e retorna o filho.
    // Os tipos são passados por referência à classe base (polimorfismo), para comparar combinações diferentes.
    // O filho carrega a média das taxas de mutação dos pais.
    Individuo Reproduzir(const std::vector<Individuo>& populacao, const Selecao& selecao, const Crossover& crossover, const Mutacao& mutacao);

    // Getters
    const std::vector<Cidade>& Cidades() const;
    int QuantidadeCidades() const;
    int EpocaAtual() const;
    const std::vector<Individuo>& Populacao() const;
    int TamanhoPopulacao() const;
    double TaxaDeMutacaoInicial() const;
    const std::vector<Epoca>& Eras() const;
};

#endif // MUNDO_HPP