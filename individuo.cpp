#include "individuo.hpp"
#include <algorithm>		// possibilita o uso do shuffle
#include <random>			// me dá um gerador de números pseudoaleatórios

Individuo::Individuo(const std::vector<Cidade>& genetica, Genetica, double mutacao)
	: genetica (genetica), distanciaPercorrida(0.0), fitness(0.0), taxaMutacao(mutacao), mediaDistancias(0.0)
{
	/**
	 * Todo:
	 * já no construtor calcular distanciaPercorrida & fitness - Requer: matriz de distâncias
	 */
}

Individuo::Individuo(const std::vector<Cidade>& cidades, Cidades, double mutacao)
	: genetica(cidades), distanciaPercorrida(0.0), fitness(0.0), taxaMutacao(mutacao), mediaDistancias(0.0)
{
	/**
	 * Todo:
	 * já no construtor calcular distanciaPercorrida & fitness - Requer: matriz de distâncias
	 */

	// o thread_local faz com que esse valor não precise ser recalculado para cada individuo, pois ele é calculado uma vez somente
	// e fica salvo na thread do processo responsável enquanto a thread existir.
	thread_local std::mt19937 gerador(std::random_device{}());
	std::shuffle(genetica.begin(), genetica.end(), gerador);
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

double Individuo::MediaDeDistancias() const {
	return this->mediaDistancias;
}