#ifndef INDIVIDUO_HPP
#define INDIVIDUO_HPP

#include "cidade.hpp"
#include <vector>

class Mundo;

struct Genetica{};			// Tag utilizada para afirmar que um vetor de Cidade passado no construtor deve ser tratado imediatamente como a genética do individuo criado.
struct Cidades{};			// Tag utilizada para afirmar que um vetor de Cidade passado no construtor deve ser transformado em uma genética.

inline constexpr Genetica AfirmarGenetica{};
inline constexpr Cidades AfirmarCidade{};

class Individuo {
private:
	std::vector<Cidade> genetica;					// Cadeia genética do indíviduo.
	double distanciaPercorrida;						// Distância total da rota representada pelo individuo.
	double fitness;									// Pontuação da rota que o individuo realiza (≡ 1 / distância percorrida)
	double taxaMutacao;								// Taxa de mutação que o individuo em si carrega. Deve estar entre 0 e 1.

public:
	// Construtor 1: O vetor de cidades vem acompanhado da tag Genetica. Dessa forma, o vetor passado é a genética do individuo e não deve ser alterado.
	// Todo: Mudar quando a matriz de distância for implementada. vai receber pra calcular coisas já no construtor.
	Individuo(const std::vector<Cidade>& genetica, Genetica, double mutacao, const Mundo& mundo);

	// Construtor 2: O vetor de cidades vem acompanhado da tag Cidade. Dessa forma, o vetor passado deve passar por transformações, e então, se torna a genética do individuo.
	// Todo: Mudar quando a matriz de distância for implementada. vai receber pra calcular coisas já no construtor.
	Individuo(const std::vector<Cidade>& cidades, Cidades, double mutacao, const Mundo& mundo);

	// Retorna uma cópia da genética do individuo.
	const std::vector<Cidade>& GeneticaDoIndividuo() const;

	// Retorna a distância que representa a rota do individuo.
	double DistanciaTotal() const;

	// Retorna a pontuação (fitness) da rota do individuo.
	double FitnessDoIndividuo() const;

	// Retorna a taxa de mutação que o individuo atual carrega.
	double TaxaDeMutacao() const;
};

#endif // INDIVIDUO_HPP