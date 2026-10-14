#include "cidade.hpp"

//Recebe os parâmetros e atribui os valores aos atributos (privados)
Cidade::Cidade(int id, double x, double y)
    : id(id), x(x), y(y)
{
}

//Retorna os valores dos atributos 
double Cidade::X() const {
    return this->x;
}

double Cidade::Y() const {
    return this->y;
}

int Cidade::Id() const {
    return this->id;
}