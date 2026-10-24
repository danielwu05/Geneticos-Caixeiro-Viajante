#ifndef CROSSOVER_HPP
#define CROSSOVER_HPP

#include "individuo.hpp"
#include <string>
#include <vector>

// Interface dos tipos de crossover. Todos recebem dois pais e retornam a
// genética de um filho que é sempre uma rota válida (cada cidade aparece
// exatamente uma vez). Requerem que os Ids das cidades sejam 0..n-1 (garantido
// por Mundo::Gerarints).
class Crossover {
public:
  virtual ~Crossover() = default;
  virtual std::vector<int> Cruzar(const std::vector<int> &pai,
                                  const std::vector<int> &mae) const = 0;
  virtual std::string Nome() const = 0;
};

// OX (Order Crossover): copia um trecho contínuo do pai e completa com as
// cidades restantes na ordem em que aparecem na mãe. Preserva a ordem relativa
// das cidades.
class CrossoverOX : public Crossover {
public:
  std::vector<int> Cruzar(const std::vector<int> &pai,
                          const std::vector<int> &mae) const override;
  std::string Nome() const override;
};

// PMX (Partially Mapped Crossover): copia um trecho do pai e preenche o resto
// com as cidades da mãe nas mesmas posições, resolvendo repetições pelo
// mapeamento entre os trechos. Preserva posições absolutas das cidades.
class CrossoverPMX : public Crossover {
public:
  std::vector<int> Cruzar(const std::vector<int> &pai,
                          const std::vector<int> &mae) const override;
  std::string Nome() const override;
};

// ERX (Edge Recombination Crossover): monta o filho usando, sempre que
// possível, arestas (ligações entre cidades vizinhas) presentes em algum dos
// pais. Feito para o caixeiro viajante, onde o que define a distância são as
// arestas.
class CrossoverERX : public Crossover {
public:
  std::vector<int> Cruzar(const std::vector<int> &pai,
                          const std::vector<int> &mae) const override;
  std::string Nome() const override;
};

// Uniforme adaptado para rotas (Uniform Order-Based Crossover): uma máscara
// aleatória escolhe, posição a posição, quais cidades vêm do pai (na mesma
// posição); as posições restantes recebem as cidades que faltam, na ordem em
// que aparecem na mãe. O crossover uniforme clássico (cada gene sorteado entre
// pai e mãe) não serve aqui, pois repetiria e perderia cidades.
class CrossoverUniforme : public Crossover {
public:
  std::vector<int> Cruzar(const std::vector<int> &pai,
                          const std::vector<int> &mae) const override;
  std::string Nome() const override;
};

#endif // CROSSOVER_HPP
