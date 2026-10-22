import matplotlib.pyplot as plt
import pandas as pd
from pathlib import Path

dir = Path(".")

arquivos = list(dir.glob("*.csv"))

cores = plt.cm.tab10.colors

for arquivo in arquivos:
    df = pd.read_csv(arquivo)

    coluna_x = df.columns[0]
    colunas_y = df.columns[1:]

    plt.figure(figsize=(10, 6))

    plt.title(f"Gráfico: {arquivo}", fontsize=14, pad=15)

    for i, coluna in enumerate(colunas_y):
        cor = cores[i % len(cores)]

        plt.plot(df[coluna_x], df[coluna], marker="o", label=coluna, color=cor)

    plt.xlabel(coluna_x, fontsize=12)
    plt.ylabel("Valores", fontsize=12)

    plt.grid(True, linestyle="--", alpha=0.6)

    plt.legend()

    plt.tight_layout()

    nome_imagem = arquivo.with_suffix(".png")
    plt.savefig(nome_imagem, dpi=300, bbox_inches="tight")
    print(f"Gráfico salvo com sucesso: {nome_imagem}")

    plt.show()
    plt.close()
