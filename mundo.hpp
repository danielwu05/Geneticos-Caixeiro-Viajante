#ifndef MUNDO_HPP
#define MUNDO_HPP

#include "cidade.hpp"
#include <random>
#include <string>
#include <vector>

class Mundo {
private:
    std::vector<Cidade> cidades;
    std::vector<std::vector<int>> individuos;
    int epoca = 0;
    int epocasSemMelhora = 0;
    double melhorDistancia = 0.0;
    std::vector<int> melhorRota;
    double tempoAcumulado = 0.0;
    double taxaMutacao = 0.02;
    int tamanhoPopulacao = 100;
    int maxEpocas = 500;
    int limiteSemMelhora = 100;
    std::string diretorio;
    std::mt19937 gerador;

    void criarCidades(int quantidadeCidades, const std::string& cenario);
    void criarPopulacaoInicial(int quantidadeCidades);
    void encontrarMelhorRotaInicial();
    double distancia(const std::vector<int>& rota) const;
    double aptidao(const std::vector<int>& rota) const;
    std::vector<int> torneio() const;
    std::vector<int> cruzar(const std::vector<int>& a, const std::vector<int>& b);
    void mutar(std::vector<int>& rota);
    void registrarEpoca(double tempoAtual);
    void salvarEstado() const;
    void salvarResultados() const;

public:
    Mundo(int quantidadeCidades, int tamanhoPopulacao, double taxaMutacao,
          int maxEpocas, int limiteSemMelhora, const std::string& cenario,
          const std::string& diretorio);
    explicit Mundo(const std::string& diretorio);
    void avancar(int quantidadeEpocas = 1);
    bool concluido() const;
    int epocaAtual() const;
};

#endif
