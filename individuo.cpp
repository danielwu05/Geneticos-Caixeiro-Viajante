#include "individuo.hpp"
#include "mundo.hpp"
#include <algorithm>		// possibilita o uso do shuffle
#include <random>			// me dá um gerador de números pseudoaleatórios

Individuo::Individuo(const std::vector<Cidade>& genetica, Genetica, double mutacao, const Mundo& mundo)
	: genetica (genetica), distanciaPercorrida(0.0), fitness(0.0), taxaMutacao(mutacao)
{
	double distanciaIteracao = 0.0;

	for (std::size_t i = 0; i < genetica.size() - 1; i++) {
		distanciaIteracao = mundo.Distancia(genetica[i].Id(), genetica[i + 1].Id());

		this->distanciaPercorrida += distanciaIteracao;
	}

	this->distanciaPercorrida += mundo.Distancia(genetica.front().Id(), genetica.back().Id());
	this->fitness = (this->distanciaPercorrida > 0.0) ? 1.0 / this->distanciaPercorrida : 0.0 ;
}

Individuo::Individuo(const std::vector<Cidade>& cidades, Cidades, double mutacao, const Mundo& mundo)
	: genetica(cidades), distanciaPercorrida(0.0), fitness(0.0), taxaMutacao(mutacao)
{
	double distanciaIteracao = 0.0;

	// o thread_local faz com que esse valor não precise ser recalculado para cada individuo, pois ele é calculado uma vez somente
	// e fica salvo na thread do processo responsável enquanto a thread existir.
	thread_local std::mt19937 gerador(std::random_device{}());
	std::shuffle(genetica.begin(), genetica.end(), gerador);

	for (std::size_t i = 0; i < genetica.size() - 1; i++) {
		distanciaIteracao = mundo.Distancia(genetica[i].Id(), genetica[i + 1].Id());

		this->distanciaPercorrida += distanciaIteracao;
	}

	this->distanciaPercorrida += mundo.Distancia(genetica.front().Id(), genetica.back().Id());
	this->fitness = (this->distanciaPercorrida > 0.0) ? 1.0 / this->distanciaPercorrida : 0.0 ;
}

const std::vector<Cidade>& Individuo::GeneticaDoIndividuo() const {
	return this->genetica;
}

double Individuo::DistanciaTotal() const {
	return this->distanciaPercorrida;
}

double Individuo::FitnessDoIndividuo() const {
	return this->fitness;
}

double Individuo::TaxaDeMutacao() const {
	return this->taxaMutacao;
}