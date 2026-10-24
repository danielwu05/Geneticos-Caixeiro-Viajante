#include "cidade.hpp"

Cidade::Cidade(int id, double x, double y) : id(id), x(x), y(y) {}
int Cidade::Id() const { return id; }
double Cidade::X() const { return x; }
double Cidade::Y() const { return y; }
