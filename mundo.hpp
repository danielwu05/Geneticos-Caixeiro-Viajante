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

class Mundo {
private:
    std::vector<Cidade> cidades;
    std::vector<std::vector<Individuo>> individuos;  // Bidimensional: [epoca][individuo]

    int quantidadeCidades;
    int epocaAtual;

    // Matriz de distâncias triangular superior.
    // distancias[i][j-i-1] com i < j armazena a distância euclidiana entre cidade i e cidade j.
    // Acesse sempre via Distancia(i, j) para normalizar os índices automaticamente.
    std::vector<std::vector<double>> distancias;

    // Aloca as cidades conforme o cenário escolhido.
    void GerarCidades(int quantidade, DistribuicaoCidades distribuicao);

    // Preenche a matriz triangular superior com as distâncias euclidianas entre todas as cidades.
    void GerarMatrizDistancias();

public:
    // Inicializa o mundo: gera cidades, matriz de distâncias e primeira geração (época 0).
    // Retorna ponteiro para o Mundo criado em memória gerenciada pelo C++.
    static Mundo* Setup(int quantidadeCidades, DistribuicaoCidades distribuicao, double taxaMutacao, int tamanhoPopulacao);

    // Consulta a distância entre cidade i e cidade j.
    // Normaliza os índices automaticamente (funciona para qualquer ordem de i, j).
    double Distancia(int i, int j) const;

    // Reprodução: seleciona pai e mãe, aplica o crossover e a mutação, e retorna o filho.
    // Os tipos são passados por referência à classe base (polimorfismo), para comparar combinações diferentes.
    // O filho carrega a média das taxas de mutação dos pais.
    static Individuo Reproduzir(const std::vector<Individuo>& populacao, const Selecao& selecao, const Crossover& crossover, const Mutacao& mutacao);

    // Getters
    const std::vector<Cidade>& Cidades() const;
    int QuantidadeCidades() const;
    int EpocaAtual() const;
};

#endif // MUNDO_HPP