#include "crossover.hpp"
#include <algorithm>
#include <random>

namespace {
	std::mt19937& Gerador() {
		thread_local std::mt19937 gerador(std::random_device{}());
		return gerador;
	}

	// Sorteia o trecho [a, b] (a <= b) herdado do pai.
	void SortearTrecho(int n, int& a, int& b) {
		std::uniform_int_distribution<int> corte(0, n - 1);
		a = corte(Gerador());
		b = corte(Gerador());
		if (a > b) std::swap(a, b);
	}
}

// OX

std::vector<Cidade> CrossoverOX::Cruzar(const Individuo& pai, const Individuo& mae) const {
	const std::vector<Cidade>& genesPai = pai.GeneticaDoIndividuo();
	const std::vector<Cidade>& genesMae = mae.GeneticaDoIndividuo();
	int n = static_cast<int>(genesPai.size());

	int a, b;
	SortearTrecho(n, a, b);

	// Marca (pelo Id) as cidades do trecho [a, b] herdado do pai.
	std::vector<bool> noTrecho(n, false);
	for (int i = a; i <= b; i++) {
		noTrecho[genesPai[i].Id()] = true;
	}

	// Cidades que faltam, na ordem em que aparecem na mãe a partir da posição b+1 (dando a volta).
	std::vector<Cidade> resto;
	resto.reserve(n - (b - a + 1));
	for (int k = 1; k <= n; k++) {
		const Cidade& cidade = genesMae[(b + k) % n];
		if (!noTrecho[cidade.Id()]) resto.push_back(cidade);
	}

	// Monta o filho: posições b+1..n-1 recebem o início de "resto", posições 0..a-1 recebem o final dele e a..b fica com o trecho do pai.
	int depoisDoTrecho = n - 1 - b;
	std::vector<Cidade> filho;
	filho.reserve(n);
	filho.insert(filho.end(), resto.begin() + depoisDoTrecho, resto.end());
	filho.insert(filho.end(), genesPai.begin() + a, genesPai.begin() + b + 1);
	filho.insert(filho.end(), resto.begin(), resto.begin() + depoisDoTrecho);
	return filho;
}

std::string CrossoverOX::Nome() const {
	return "OX";
}

// PMX

std::vector<Cidade> CrossoverPMX::Cruzar(const Individuo& pai, const Individuo& mae) const {
	const std::vector<Cidade>& genesPai = pai.GeneticaDoIndividuo();
	const std::vector<Cidade>& genesMae = mae.GeneticaDoIndividuo();
	int n = static_cast<int>(genesPai.size());

	int a, b;
	SortearTrecho(n, a, b);

	// posicaoNoPai[id] = posição da cidade "id" na rota do pai.
	std::vector<int> posicaoNoPai(n);
	for (int i = 0; i < n; i++) {
		posicaoNoPai[genesPai[i].Id()] = i;
	}
	std::vector<bool> noTrecho(n, false);
	for (int i = a; i <= b; i++) {
		noTrecho[genesPai[i].Id()] = true;
	}

	std::vector<Cidade> filho = genesMae;
	for (int i = 0; i < n; i++) {
		if (i >= a && i <= b) {
			filho[i] = genesPai[i];
			continue;
		}
		// Se a cidade da mãe já veio no trecho do pai, segue o mapeamento pai -> mãe até achar uma que não veio.
		Cidade cidade = genesMae[i];
		while (noTrecho[cidade.Id()]) {
			cidade = genesMae[posicaoNoPai[cidade.Id()]];
		}
		filho[i] = cidade;
	}
	return filho;
}

std::string CrossoverPMX::Nome() const {
	return "PMX";
}

// ERX

std::vector<Cidade> CrossoverERX::Cruzar(const Individuo& pai, const Individuo& mae) const {
	const std::vector<Cidade>& genesPai = pai.GeneticaDoIndividuo();
	const std::vector<Cidade>& genesMae = mae.GeneticaDoIndividuo();
	int n = static_cast<int>(genesPai.size());

	// Tabela de arestas: vizinhos[id] = cidades ligadas a "id" em qualquer um dos pais (a rota é cíclica).
	std::vector<std::vector<int>> vizinhos(n);
	auto adicionarArestas = [&](const std::vector<Cidade>& genes) {
		for (int i = 0; i < n; i++) {
			int atual = genes[i].Id();
			for (int vizinho : {genes[(i + 1) % n].Id(), genes[(i + n - 1) % n].Id()}) {
				if (std::find(vizinhos[atual].begin(), vizinhos[atual].end(), vizinho) == vizinhos[atual].end()) {
					vizinhos[atual].push_back(vizinho);
				}
			}
		}
	};
	adicionarArestas(genesPai);
	adicionarArestas(genesMae);

	std::vector<const Cidade*> cidadePorId(n);
	for (const Cidade& cidade : genesPai) {
		cidadePorId[cidade.Id()] = &cidade;
	}

	// Cidades ainda não visitadas, com remoção O(1) (troca com a última).
	std::vector<int> naoVisitadas(n), posicaoEmNaoVisitadas(n);
	for (int i = 0; i < n; i++) {
		naoVisitadas[i] = i;
		posicaoEmNaoVisitadas[i] = i;
	}
	auto marcarVisitada = [&](int id) {
		int pos = posicaoEmNaoVisitadas[id];
		int ultima = naoVisitadas.back();
		naoVisitadas[pos] = ultima;
		posicaoEmNaoVisitadas[ultima] = pos;
		naoVisitadas.pop_back();
	};

	std::vector<Cidade> filho;
	filho.reserve(n);
	int atual = genesPai[0].Id();
	while (true) {
		filho.push_back(*cidadePorId[atual]);
		marcarVisitada(atual);
		if (naoVisitadas.empty()) break;

		// Remove a cidade atual das listas dos seus vizinhos.
		for (int vizinho : vizinhos[atual]) {
			std::vector<int>& lista = vizinhos[vizinho];
			lista.erase(std::find(lista.begin(), lista.end(), atual));
		}

		if (vizinhos[atual].empty()) {
			// Sem aresta herdada disponível: segue para uma cidade não visitada aleatória.
			std::uniform_int_distribution<int> sorteio(0, static_cast<int>(naoVisitadas.size()) - 1);
			atual = naoVisitadas[sorteio(Gerador())];
		} else {
			// Escolhe o vizinho com menos vizinhos restantes (evita que ele fique isolado depois).
			int proximo = vizinhos[atual][0];
			for (int vizinho : vizinhos[atual]) {
				if (vizinhos[vizinho].size() < vizinhos[proximo].size()) proximo = vizinho;
			}
			atual = proximo;
		}
	}
	return filho;
}

std::string CrossoverERX::Nome() const {
	return "ERX";
}

// Uniforme (Uniform Order-Based)

std::vector<Cidade> CrossoverUniforme::Cruzar(const Individuo& pai, const Individuo& mae) const {
	const std::vector<Cidade>& genesPai = pai.GeneticaDoIndividuo();
	const std::vector<Cidade>& genesMae = mae.GeneticaDoIndividuo();
	int n = static_cast<int>(genesPai.size());

	// Máscara: true = a posição herda a cidade do pai.
	std::bernoulli_distribution moeda(0.5);
	std::vector<bool> doPai(n);
	std::vector<bool> usada(n, false);
	for (int i = 0; i < n; i++) {
		doPai[i] = moeda(Gerador());
		if (doPai[i]) usada[genesPai[i].Id()] = true;
	}

	// Preenche as posições restantes com as cidades que faltam, na ordem da mãe.
	std::vector<Cidade> filho = genesPai;
	int posicaoMae = 0;
	for (int i = 0; i < n; i++) {
		if (doPai[i]) continue;
		while (usada[genesMae[posicaoMae].Id()]) posicaoMae++;
		filho[i] = genesMae[posicaoMae++];
	}
	return filho;
}

std::string CrossoverUniforme::Nome() const {
	return "Uniforme";
}
