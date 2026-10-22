#ifndef CRIACAO_DE_GRAFICOS_HPP
#define CRIACAO_DE_GRAFICOS_HPP

using namespace std;

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void CriarArquivoTabela(ofstream &meuArquivo, string nomeArquivo,
                        vector<string> parametros);

void AdicionarLinhaArquivo(ofstream &meuArquivo,
                           vector<float> valoresParametros);

void FecharArquivo(ofstream &meuArquivo);

#endif
