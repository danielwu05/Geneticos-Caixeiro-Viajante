#include "mundo.hpp"

#include <cmath>    // sqrt, cos, sin, M_PI
#include <random>
#include <stdexcept>

// Alocação de cidades

void Mundo::GerarCidades(int quantidade, DistribuicaoCidades distribuicao) {
    cidades.clear();
    cidades.reserve(quantidade);

    thread_local std::mt19937 gerador(std::random_device{}());

    if (distribuicao == DistribuicaoCidades::Uniforme) {
        // Coordenadas uniformemente distribuídas em [0, 1)
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        for (int i = 0; i < quantidade; i++) {
            cidades.emplace_back(i, dist(gerador), dist(gerador));
        }
        
    } else {
        // Cenário circular: cidades igualmente espaçadas em um círculo unitário (benchmark)
        const double raio = 1.0;
        for (int i = 0; i < quantidade; i++) {
            double angulo = 2.0 * M_PI * i / quantidade;
            cidades.emplace_back(i, raio * std::cos(angulo), raio * std::sin(angulo));
        }
    }
}

// Matriz de distâncias triangular inferior 
// Layout: distancias[i][j] com i > j  →  distância entre cidade i e cidade j.
// A diagonal (i == j) e o triângulo superior (i < j) não são armazenados.
// Para consultar, use sempre Distancia(i, j) — ele normaliza os índices.

void Mundo::GerarMatrizDistancias() {
    int n = static_cast<int>(cidades.size());

    // distancias[i] tem exatamente i entradas (índices 0..i-1)
    distancias.resize(n);
    for (int i = 0; i < n; i++) {
        distancias[i].resize(i);  // linha i tem i colunas (0 até i-1)
    }

    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            double dx = cidades[i].X() - cidades[j].X();
            double dy = cidades[i].Y() - cidades[j].Y();
            //PEGA A DISTÂNCIA EUCLIDIANA entre cada cidade
            distancias[i][j] = std::sqrt(dx * dx + dy * dy);
     
        }
    }
}

// Consulta de distância entre cidades i e j

double Mundo::Distancia(int i, int j) const {
    if (i == j) return 0.0;

    // Garante que acessamos sempre o triângulo inferior (linha > coluna)
    if (i < j) std::swap(i, j);

    return distancias[i][j];
}

// Setup
Mundo* Mundo::Setup(int quantidadeCidades, DistribuicaoCidades distribuicao, double taxaMutacao, int tamanhoPopulacao) {
    
    // Validação Do critério do enunciado
    //POSSIVEL REMOVER
    if (quantidadeCidades < 8) {
        throw std::invalid_argument("O enunciado exige ao menos 8 cidades.");
    }

    Mundo* mundo = new Mundo();
    mundo->quantidadeCidades = quantidadeCidades;
    mundo->epocaAtual = 0;

    mundo->GerarCidades(quantidadeCidades, distribuicao);
    mundo->GerarMatrizDistancias();

    // Todo: Gerar a primeira geração (época 0) — tarefa de Indivíduos + pareamento.
    // mundo->individuos.push_back( ... primeira geração ... );

    return mundo;
}

// Getters

const std::vector<Cidade>& Mundo::Cidades() const {
    return this->cidades;
}

int Mundo::QuantidadeCidades() const {
    return this->quantidadeCidades;
}

int Mundo::EpocaAtual() const {
    return this->epocaAtual;
}