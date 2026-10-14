#ifndef CIDADE_HPP
#define CIDADE_HPP

//ENCAPSULAMENTO
class Cidade { 
private:
    double x;   // Coordenada X da cidade
    double y;   // Coordenada Y da cidade
    int id;     // Identificador único da cidade
    

//PARA PASSAR O VALOR DO MEMBRO PRIVATE PARA FORA DELE
public:
    Cidade(int id, double x, double y);

    double X() const;
    double Y() const;
    int Id() const;
};

#endif // CIDADE_HPP