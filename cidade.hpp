#ifndef CIDADE_HPP
#define CIDADE_HPP

class Cidade {
private:
    int indice;
    double x;
    double y;

public:
    Cidade(int indice = 0, double x = 0.0, double y = 0.0);
    int Indice() const;
    double X() const;
    double Y() const;
};

#endif
