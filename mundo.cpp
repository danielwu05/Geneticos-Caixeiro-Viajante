#include "mundo.hpp"

#include <cmath>    // sqrt, cos, sin, M_PI
#include <random>
#include <stdexcept>
#include <algorithm> // std::max_element

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

// Cria a primeira geração de individuos
// Todos eles têm como cidade inicial uma cidade aleatória.
std::vector<Individuo> Mundo::PrimeiraGeracao(int quantidade, double mutacaoInicial) {
    std::vector<Individuo> precursores;
    precursores.reserve(quantidade);

    for (int i = 0; i < quantidade; i++) {
        // emplace_back -> cria uma instância do tipo do vetor usando o construtor da instância em questão e coloca na última posição vaga no vetor.
        precursores.emplace_back(this->cidades, AfirmarCidade, mutacaoInicial, *this);
    }

    return precursores;
}

// Matriz de distâncias triangular superior
// Layout: distancias[i][j-i-1] com i < j  →  distância entre cidade i e cidade j.
// A diagonal (i == j) e o triângulo inferior (i > j) não são armazenados.
// Para consultar, use sempre Distancia(i, j) — ele normaliza os índices.

void Mundo::GerarMatrizDistancias() {
    int n = static_cast<int>(cidades.size());

    // distancias[i] tem exatamente (n-i-1) entradas (colunas i+1 até n-1)
    distancias.resize(n);
    for (int i = 0; i < n; i++) {
        distancias[i].resize(n - i - 1);  // linha i tem (n-i-1) colunas
    }

    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            double dx = cidades[i].X() - cidades[j].X();
            double dy = cidades[i].Y() - cidades[j].Y();
            //PEGA A DISTÂNCIA EUCLIDIANA entre cada cidade
            distancias[i][j - i - 1] = std::sqrt(dx * dx + dy * dy);
        }
    }
}

// Consulta de distância entre cidades i e j

double Mundo::Distancia(int i, int j) const {
    if (i == j) return 0.0;

    // Garante que acessamos sempre o triângulo superior (linha < coluna)
    if (i > j) std::swap(i, j);

    return distancias[i][j - i - 1];
}

void Mundo::RegistrarEpoca (int indiceEpoca, const std::string& selecao, const std::string& crossover, const std::string& mutacao) {
    const std::vector<Individuo>& populacao = this->individuos[indiceEpoca];

    double menorDistancia = populacao[0].DistanciaTotal();
    double maiorDistancia = menorDistancia;
    double somaDistancias = 0.0;
    const Individuo* melhorIndividuo = &populacao[0];

    for (const auto& ind : populacao) {
        double d = ind.DistanciaTotal();
        somaDistancias += d;

        if (d < menorDistancia) {
            menorDistancia = d;
            melhorIndividuo = &ind;
        }

        if (d > maiorDistancia) {
            maiorDistancia = d;
        }
    }

    std::vector<int> idsRota;
    idsRota.reserve(melhorIndividuo->GeneticaDoIndividuo().size());

    for (const Cidade& c : melhorIndividuo->GeneticaDoIndividuo()) {
        idsRota.push_back(c.Id());
    }

    this->eras.push_back(
        Epoca{
            indiceEpoca,
            menorDistancia,
            somaDistancias / populacao.size(),
            maiorDistancia,
            std::move(idsRota),
            selecao,
            crossover,
            mutacao
        }
    );
}

// Reprodução
Individuo Mundo::Reproduzir(const std::vector<Individuo>& populacao, const Selecao& selecao, const Crossover& crossover, const Mutacao& mutacao) {
    std::vector<const Individuo*> pais = selecao.SelecionarVarios(populacao, 2);
    const Individuo& pai = *pais[0];
    const Individuo& mae = *pais[1];

    double taxa = (pai.TaxaDeMutacao() + mae.TaxaDeMutacao()) / 2.0;

    // A mutação acontece antes de construir o filho, para que distância e fitness calculados no construtor já reflitam a rota final.
    std::vector<Cidade> genetica = crossover.Cruzar(pai, mae);
    mutacao.Mutar(genetica, taxa);

    return Individuo(genetica, AfirmarGenetica, taxa, *this);
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
    mundo->tamanhoPopulacao = tamanhoPopulacao;
    mundo->taxaDeMutacaoInicial = taxaMutacao;
    mundo->epocaAtual = 0;

    mundo->GerarCidades(quantidadeCidades, distribuicao);
    mundo->GerarMatrizDistancias();

    // mundo->individuos.push_back( ... primeira geração ... );
    // Geração da primeira epoca
    mundo->individuos.push_back(mundo->PrimeiraGeracao(tamanhoPopulacao, taxaMutacao));

    // Registra a informacao da primeira epoca
    mundo->RegistrarEpoca(0, "Nenhuma (inicial)", "Nenhum (inicial)", "Nenhum (inicial)");

    return mundo;
}

void Mundo::AvancarEpoca(const Selecao& selecao, const Crossover& crossover, const Mutacao& mutacao) {
    const std::vector<Individuo>& populacaoAtual = this->individuos[this->epocaAtual];

    std::vector<Individuo> novaGeracao;
    novaGeracao.reserve(this->tamanhoPopulacao);

    // A melhor rota de uma geração é preservada na próxima.
    auto melhorIndividuo = std::max_element(
        populacaoAtual.begin(), populacaoAtual.end(),
        [](const Individuo& a, const Individuo& b) {
            return a.FitnessDoIndividuo() < b.FitnessDoIndividuo();
        }
    );

    novaGeracao.push_back(*melhorIndividuo);

    // Reprodução para preencher o restante (taxa de reposição mantida em 1 (não cresce nem diminui))
    while (static_cast<int>(novaGeracao.size()) < this->tamanhoPopulacao) {
        novaGeracao.push_back(this->Reproduzir(populacaoAtual, selecao, crossover, mutacao));
    }

    // atualiza as informações quanto a epoca
    this->individuos.push_back(std::move(novaGeracao));
    this->epocaAtual++;

    // registra as informações da nova geração em 'eras'
    this->RegistrarEpoca(this->epocaAtual, selecao.Nome(), crossover.Nome(), mutacao.Nome());
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

const std::vector<std::vector<Individuo>>& Mundo::Populacao() const{
    return this->individuos;
}

int Mundo::TamanhoPopulacao() const {
    return this->tamanhoPopulacao;
}

double Mundo::TaxaDeMutacaoInicial() const {
    return this->taxaDeMutacaoInicial;
}

const std::vector<Epoca>& Mundo::Eras() const {
    return this->eras;
}