#include "criacao_de_graficos.hpp"

void CriarArquivoTabela(ofstream &meuArquivo, string nomeArquivo,
                        vector<string> parametros) {

  string nomeArquivoComExtensao = nomeArquivo + ".csv";

  meuArquivo.open(nomeArquivoComExtensao);

  if (!meuArquivo.is_open()) {
    cerr << "Erro: Não foi possível criar/acessar arquivo" << endl;
  } else {
    cout << "Arquivo " << nomeArquivoComExtensao << " foi criado com sucesso"
         << endl;
  }

  int i = 0;
  for (i = 0; i < parametros.size() - 1; i++) {
    meuArquivo << parametros[i] << ",";
  }

  meuArquivo << parametros.back() << "\n";
}

void AdicionarLinhaArquivo(ofstream &meuArquivo, vector<float> parametros) {

  if (meuArquivo.is_open() && meuArquivo.good()) {

    int i = 0;
    for (i = 0; i < parametros.size() - 1; i++) {
      meuArquivo << parametros[i] << ",";
    }

    meuArquivo << parametros.back() << "\n";
  } else {
    cerr << "Erro: Arquivo não está aberto ou não pode ser editado" << endl;
  }
}

void FecharArquivo(ofstream &meuArquivo) {
  if (meuArquivo.is_open()) {
    meuArquivo.close();
    cout << "Arquivo encerrado com sucesso" << endl;
  }
}
