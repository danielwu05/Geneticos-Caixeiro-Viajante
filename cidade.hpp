#ifndef CIDADE_HPP
#define CIDADE_HPP

class Cidade {
private:
  int id;
  double x;
  double y;

  // PARA PASSAR O VALOR DO MEMBRO PRIVATE PARA FORA DELE
public:
  Cidade(int id = 0, double x = 0.0, double y = 0.0);
  int Id() const;
  double X() const;
  double Y() const;
};

#endif
