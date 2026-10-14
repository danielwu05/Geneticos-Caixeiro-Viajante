#include "selecao.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

namespace {
	std::mt19937& Gerador() {
		thread_local std::mt19937 gerador(std::random_device{}());
		return gerador;
	}
}

std::vector<const Individuo*> Selecao::SelecionarVarios(const std::vector<Individuo>& populacao, int quantidade) const {
	std::vector<const Individuo*> selecionados;
	selecionados.reserve(quantidade);
	for (int k = 0; k < quantidade; k++) {
		selecionados.push_back(&Selecionar(populacao));
	}
	return selecionados;
}

// Torneio

SelecaoTorneio::SelecaoTorneio(int tamanho)
	: tamanho(tamanho)
{
}

const Individuo& SelecaoTorneio::Selecionar(const std::vector<Individuo>& populacao) const {
	std::uniform_int_distribution<int> sorteio(0, static_cast<int>(populacao.size()) - 1);

	const Individuo* melhor = &populacao[sorteio(Gerador())];
	for (int k = 1; k < tamanho; k++) {
		const Individuo* competidor = &populacao[sorteio(Gerador())];
		if (competidor->FitnessDoIndividuo() > melhor->FitnessDoIndividuo()) {
			melhor = competidor;
		}
	}
	return *melhor;
}

std::string SelecaoTorneio::Nome() const {
	return "Torneio(" + std::to_string(tamanho) + ")";
}

// Roleta

const Individuo& SelecaoRoleta::Selecionar(const std::vector<Individuo>& populacao) const {
	double total = 0.0;
	for (const Individuo& individuo : populacao) {
		total += individuo.FitnessDoIndividuo();
	}

	double alvo = std::uniform_real_distribution<double>(0.0, total)(Gerador());
	double acumulado = 0.0;
	for (const Individuo& individuo : populacao) {
		acumulado += individuo.FitnessDoIndividuo();
		if (alvo < acumulado) return individuo;
	}
	return populacao.back();	// Só por arredondamento de ponto flutuante.
}

std::string SelecaoRoleta::Nome() const {
	return "Roleta";
}

// Ranking

const Individuo& SelecaoRanking::Selecionar(const std::vector<Individuo>& populacao) const {
	int n = static_cast<int>(populacao.size());

	// Índices da população do pior para o melhor fitness.
	std::vector<int> ordem(n);
	std::iota(ordem.begin(), ordem.end(), 0);
	std::sort(ordem.begin(), ordem.end(), [&](int a, int b) {
		return populacao[a].FitnessDoIndividuo() < populacao[b].FitnessDoIndividuo();
	});

	// Peso da posição k (0 = pior) é k+1. Soma dos pesos = n(n+1)/2.
	double total = n * (n + 1) / 2.0;
	double alvo = std::uniform_real_distribution<double>(0.0, total)(Gerador());
	double acumulado = 0.0;
	for (int k = 0; k < n; k++) {
		acumulado += k + 1;
		if (alvo < acumulado) return populacao[ordem[k]];
	}
	return populacao[ordem.back()];	// Só por arredondamento de ponto flutuante.
}

std::string SelecaoRanking::Nome() const {
	return "Ranking";
}

// Top Ranking

SelecaoTopRanking::SelecaoTopRanking(double fracao)
	: fracao(fracao)
{
}

const Individuo& SelecaoTopRanking::Selecionar(const std::vector<Individuo>& populacao) const {
	int n = static_cast<int>(populacao.size());
	int quantidadeTopo = std::max(1, static_cast<int>(fracao * n));

	// Separa os "quantidadeTopo" melhores no começo do vetor de índices (sem ordenar o resto).
	std::vector<int> ordem(n);
	std::iota(ordem.begin(), ordem.end(), 0);
	std::nth_element(ordem.begin(), ordem.begin() + (quantidadeTopo - 1), ordem.end(), [&](int a, int b) {
		return populacao[a].FitnessDoIndividuo() > populacao[b].FitnessDoIndividuo();
	});

	std::uniform_int_distribution<int> sorteio(0, quantidadeTopo - 1);
	return populacao[ordem[sorteio(Gerador())]];
}

std::string SelecaoTopRanking::Nome() const {
	return "TopRanking(" + std::to_string(std::lround(fracao * 100)) + "%)";
}

// Amostragem Estocástica Universal

const Individuo& SelecaoAmostragemEstocastica::Selecionar(const std::vector<Individuo>& populacao) const {
	return *SelecionarVarios(populacao, 1)[0];
}

std::vector<const Individuo*> SelecaoAmostragemEstocastica::SelecionarVarios(const std::vector<Individuo>& populacao, int quantidade) const {
	double total = 0.0;
	for (const Individuo& individuo : populacao) {
		total += individuo.FitnessDoIndividuo();
	}

	// Um único sorteio define o primeiro ponteiro; os demais ficam a "passo" de distância um do outro.
	double passo = total / quantidade;
	double ponteiro = std::uniform_real_distribution<double>(0.0, passo)(Gerador());

	std::vector<const Individuo*> selecionados;
	selecionados.reserve(quantidade);
	double acumulado = 0.0;
	for (const Individuo& individuo : populacao) {
		acumulado += individuo.FitnessDoIndividuo();
		while (static_cast<int>(selecionados.size()) < quantidade && ponteiro < acumulado) {
			selecionados.push_back(&individuo);
			ponteiro += passo;
		}
	}
	// Só por arredondamento de ponto flutuante.
	while (static_cast<int>(selecionados.size()) < quantidade) {
		selecionados.push_back(&populacao.back());
	}

	// Os ponteiros saem na ordem da população; embaralha para que a ordem (ex.: quem é pai e quem é mãe) não tenha viés.
	std::shuffle(selecionados.begin(), selecionados.end(), Gerador());
	return selecionados;
}

std::string SelecaoAmostragemEstocastica::Nome() const {
	return "AmostragemEstocastica";
}
