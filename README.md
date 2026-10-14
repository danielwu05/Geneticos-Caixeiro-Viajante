# Caixeiro viajante com algoritmo genético

## Interface gráfica em Python

A interface usa Tkinter para os controles e Matplotlib para os gráficos. Ela
permite preencher e salvar os parâmetros em JSON, carregar os CSVs produzidos
pelo programa C++ e visualizar a evolução das distâncias e a melhor rota.

No Ubuntu/Debian, instale o suporte do Tkinter e ambientes virtuais, crie um
ambiente Python local e instale o Matplotlib:

```bash
sudo apt install python3-tk python3-venv
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
python3 interface.py
```

Para visualizar resultados, escolha a pasta que contém:

- `historico.csv`: colunas `epoca,melhor_distancia,distancia_media,melhor_rota`;
- `cidades.csv`: colunas `indice,x,y`.

`melhor_rota` deve conter os índices das cidades separados por espaços, por
exemplo `0 3 1 2`. O algoritmo C++ ainda precisa ser ligado à interface para
ler a configuração JSON e gerar esses arquivos.
