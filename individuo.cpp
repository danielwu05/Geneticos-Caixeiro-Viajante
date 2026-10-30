#include "individuo.hpp"
#include "mundo.hpp"
#include <algorithm>
#include <random>

Individuo::Individuo(const std::vector<int> &genetica, Genetica, double mutacao,
                     const Mundo &mundo)
    : genetica(genetica), distanciaPercorrida(mundo.distancia(genetica)),
      taxaMutacao(mutacao) {
  this->fitness = (this->distanciaPercorrida > 0.0)
                      ? 1.0 / (1.0 + this->distanciaPercorrida)
                      : 0.0;
}

Individuo::Individuo(const std::vector<int> &cidades, Cidades, double mutacao,
                     const Mundo &mundo)
    : genetica(cidades), distanciaPercorrida(0.0), fitness(0.0),
      taxaMutacao(mutacao) {
  thread_local std::mt19937 gerador(std::random_device{}());
  std::shuffle(genetica.begin(), genetica.end(), gerador);

  distanciaPercorrida = mundo.distancia(this->genetica);
  this->fitness = (this->distanciaPercorrida > 0.0)
                      ? 1.0 / (1.0 + this->distanciaPercorrida)
                      : 0.0;
}

const std::vector<int> &Individuo::GeneticaDoIndividuo() const {
  return this->genetica;
}

double Individuo::DistanciaTotal() const { return this->distanciaPercorrida; }

double Individuo::FitnessDoIndividuo() const { return this->fitness; }

double Individuo::TaxaDeMutacao() const { return this->taxaMutacao; }
