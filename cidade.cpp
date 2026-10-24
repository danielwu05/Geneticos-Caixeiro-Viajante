#include "cidade.hpp"

Cidade::Cidade(int indice, double x, double y) : indice(indice), x(x), y(y) {}
int Cidade::Indice() const { return indice; }
double Cidade::X() const { return x; }
double Cidade::Y() const { return y; }
