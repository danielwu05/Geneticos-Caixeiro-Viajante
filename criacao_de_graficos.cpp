#include "criacao_de_graficos.hpp"

void CriarArquivoTabela(ofstream &meuArquivo, string nomeArquivo,
                        string parametro1, string parametro2) {

  string nomeArquivoComExtensao = nomeArquivo + ".csv";

  meuArquivo.open(nomeArquivoComExtensao);

  if (!meuArquivo.is_open()) {
    cerr << "Erro: Não foi possível criar/acessar arquivo" << endl;
  } else {
    cout << "Arquivo " << nomeArquivoComExtensao << " foi criado com sucesso"
         << endl;
  }

  string cabecalhoDoArquivo = parametro1 + "," + parametro2 + "\n";

  meuArquivo << cabecalhoDoArquivo;
}

void AdicionarLinhaArquivo(ofstream &meuArquivo, float valorParametro1,
                           float valorParametro2) {
  string parametrosConcatenados =
      to_string(valorParametro1) + "," + to_string(valorParametro2) + "\n";

  if (meuArquivo.is_open() && meuArquivo.good()) {
    meuArquivo << parametrosConcatenados;
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
