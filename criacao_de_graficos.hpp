#ifndef CRIACAO_DE_GRAFICOS_HPP
#define CRIACAO_DE_GRAFICOS_HPP

using namespace std;

#include <fstream>
#include <iostream>
#include <string>

void CriarArquivoTabela(ofstream &meuArquivo, string nomeArquivo,
                        string parametro1, string parametro2);

void AdicionarLinhaArquivo(ofstream &meuArquivo, float valorParametro1,
                           float valorParametro2);

void FecharArquivo(ofstream &meuArquivo);

#endif
