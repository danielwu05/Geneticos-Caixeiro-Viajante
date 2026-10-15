#ifndef SELECAO_HPP
#define SELECAO_HPP

#include "individuo.hpp"
#include <string>
#include <vector>

// Interface dos tipos de seleção de pais para a reprodução. Todos favorecem indivíduos de maior fitness.
class Selecao {
public:
	virtual ~Selecao() = default;
	virtual const Individuo& Selecionar(const std::vector<Individuo>& populacao) const = 0;
	virtual std::string Nome() const = 0;

	// Seleciona "quantidade" indivíduos. Por padrão chama Selecionar() várias vezes;
	// métodos que escolhem todos de uma vez (como a Amostragem Estocástica Universal) sobrescrevem.
	virtual std::vector<const Individuo*> SelecionarVarios(const std::vector<Individuo>& populacao, int quantidade) const;
};

// Torneio: sorteia "tamanho" indivíduos da população e retorna o de maior fitness.
// Depende só de quem é melhor (não da escala do fitness); a pressão seletiva é ajustada pelo tamanho do torneio.
class SelecaoTorneio : public Selecao {
private:
	int tamanho;

public:
	explicit SelecaoTorneio(int tamanho = 3);
	const Individuo& Selecionar(const std::vector<Individuo>& populacao) const override;
	std::string Nome() const override;
};

// Roleta: chance de ser escolhido proporcional ao fitness.
// Com fitness = 1/distância as aptidões tendem a ficar próximas, o que reduz a pressão seletiva.
class SelecaoRoleta : public Selecao {
public:
	const Individuo& Selecionar(const std::vector<Individuo>& populacao) const override;
	std::string Nome() const override;
};

// Ranking (linear): ordena a população por fitness e a chance de ser escolhido é proporcional à posição (pior = 1, melhor = N).
// Mantém a pressão seletiva mesmo quando os fitness são muito próximos.
class SelecaoRanking : public Selecao {
public:
	const Individuo& Selecionar(const std::vector<Individuo>& populacao) const override;
	std::string Nome() const override;
};

// Top Ranking (truncamento): só os "fracao" melhores da população (ex.: 0.2 = 20% melhores) podem ser escolhidos, todos com a mesma chance.
// Pressão seletiva alta: converge rápido, mas perde diversidade se a fração for pequena.
class SelecaoTopRanking : public Selecao {
private:
	double fracao;

public:
	explicit SelecaoTopRanking(double fracao = 0.2);
	const Individuo& Selecionar(const std::vector<Individuo>& populacao) const override;
	std::string Nome() const override;
};

// Amostragem Estocástica Universal (SUS): como a roleta (chance proporcional ao fitness), mas escolhe todos de uma vez
// com ponteiros igualmente espaçados em um único giro. Reduz o acaso da roleta: quem tem fitness acima da média é escolhido quase sempre.
class SelecaoAmostragemEstocastica : public Selecao {
public:
	const Individuo& Selecionar(const std::vector<Individuo>& populacao) const override;
	std::vector<const Individuo*> SelecionarVarios(const std::vector<Individuo>& populacao, int quantidade) const override;
	std::string Nome() const override;
};

#endif // SELECAO_HPP
