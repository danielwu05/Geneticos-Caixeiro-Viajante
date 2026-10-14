import csv
import json
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox, ttk

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


class Interface:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Caixeiro viajante — algoritmo genético")
        self.root.geometry("1100x720")
        self.root.minsize(850, 600)

        self.vars = {
            "cenario": tk.StringVar(value="Aleatório"),
            "cidades": tk.StringVar(value="20"),
            "populacao": tk.StringVar(value="100"),
            "mutacao": tk.StringVar(value="0.02"),
            "epocas": tk.StringVar(value="500"),
            "sem_melhora": tk.StringVar(value="100"),
        }
        self.status = tk.StringVar(value="Carregue uma pasta com os CSVs dos resultados.")
        self._montar_janela()

    def _montar_janela(self) -> None:
        painel = ttk.Frame(self.root, padding=12)
        painel.pack(fill=tk.BOTH, expand=True)

        controles = ttk.LabelFrame(painel, text="Parâmetros do experimento", padding=12)
        controles.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 12))

        ttk.Label(controles, text="Cenário").grid(row=0, column=0, sticky="w", pady=4)
        ttk.Combobox(
            controles, textvariable=self.vars["cenario"],
            values=("Aleatório", "Circular"), state="readonly", width=20,
        ).grid(row=0, column=1, pady=4)

        campos = (
            ("Quantidade de cidades", "cidades"),
            ("Tamanho da população", "populacao"),
            ("Taxa de mutação (0–1)", "mutacao"),
            ("Máximo de épocas", "epocas"),
            ("Épocas sem melhora", "sem_melhora"),
        )
        for linha, (rotulo, chave) in enumerate(campos, start=1):
            ttk.Label(controles, text=rotulo).grid(row=linha, column=0, sticky="w", pady=4)
            ttk.Entry(controles, textvariable=self.vars[chave], width=23).grid(
                row=linha, column=1, pady=4
            )

        ttk.Button(controles, text="Salvar configuração", command=self.salvar_configuracao).grid(
            row=6, column=0, columnspan=2, sticky="ew", pady=(12, 4)
        )
        ttk.Separator(controles).grid(row=7, column=0, columnspan=2, sticky="ew", pady=10)
        ttk.Button(controles, text="Abrir resultados (CSV)...", command=self.abrir_resultados).grid(
            row=8, column=0, columnspan=2, sticky="ew", pady=4
        )
        ttk.Label(
            controles,
            text="O algoritmo C++ poderá ler a configuração salva e gerar os CSVs.",
            wraplength=230,
        ).grid(row=9, column=0, columnspan=2, sticky="w", pady=(10, 0))

        area = ttk.Frame(painel)
        area.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        self.figure, (self.ax_distancia, self.ax_rota) = plt.subplots(
            1, 2, figsize=(10, 5), constrained_layout=True
        )
        self.canvas = FigureCanvasTkAgg(self.figure, master=area)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
        ttk.Label(area, textvariable=self.status, anchor="w").pack(fill=tk.X, pady=(8, 0))
        self._mostrar_telas_vazias()

    def _mostrar_telas_vazias(self) -> None:
        self.ax_distancia.clear()
        self.ax_distancia.set_title("Evolução por época")
        self.ax_distancia.set_xlabel("Época")
        self.ax_distancia.set_ylabel("Distância")
        self.ax_distancia.text(0.5, 0.5, "Carregue os resultados CSV", ha="center", va="center",
                               transform=self.ax_distancia.transAxes)
        self.ax_rota.clear()
        self.ax_rota.set_title("Melhor rota")
        self.ax_rota.text(0.5, 0.5, "A rota aparecerá aqui", ha="center", va="center",
                          transform=self.ax_rota.transAxes)
        self.canvas.draw_idle()

    def _ler_configuracao(self) -> dict:
        cidades = int(self.vars["cidades"].get())
        populacao = int(self.vars["populacao"].get())
        mutacao = float(self.vars["mutacao"].get())
        epocas = int(self.vars["epocas"].get())
        sem_melhora = int(self.vars["sem_melhora"].get())
        if cidades < 8 or populacao < 2 or not 0 <= mutacao <= 1 or epocas < 1 or sem_melhora < 1:
            raise ValueError("Use pelo menos 8 cidades, população ≥ 2, mutação entre 0 e 1 e épocas positivas.")
        return {
            "cenario": self.vars["cenario"].get().lower(),
            "quantidade_cidades": cidades,
            "tamanho_populacao": populacao,
            "taxa_mutacao": mutacao,
            "max_epocas": epocas,
            "epocas_sem_melhora": sem_melhora,
        }

    def salvar_configuracao(self) -> None:
        try:
            config = self._ler_configuracao()
        except ValueError as erro:
            messagebox.showerror("Parâmetros inválidos", str(erro))
            return
        destino = filedialog.asksaveasfilename(
            title="Salvar configuração", initialfile="configuracao.json",
            defaultextension=".json", filetypes=(("JSON", "*.json"),),
        )
        if not destino:
            return
        Path(destino).write_text(json.dumps(config, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        self.status.set(f"Configuração salva em {destino}")

    def abrir_resultados(self) -> None:
        pasta = filedialog.askdirectory(title="Escolha a pasta com historico.csv e cidades.csv")
        if not pasta:
            return
        try:
            historico = self._ler_historico(Path(pasta) / "historico.csv")
            cidades = self._ler_cidades(Path(pasta) / "cidades.csv")
            self._desenhar(historico, cidades)
        except (OSError, ValueError, KeyError) as erro:
            messagebox.showerror("Não foi possível abrir os resultados", str(erro))

    @staticmethod
    def _ler_historico(caminho: Path) -> list[dict]:
        with caminho.open(newline="", encoding="utf-8") as arquivo:
            registros = list(csv.DictReader(arquivo))
        obrigatorias = {"epoca", "melhor_distancia", "distancia_media", "melhor_rota"}
        if not registros or not obrigatorias.issubset(registros[0]):
            raise ValueError("historico.csv vazio ou sem as colunas esperadas.")
        for registro in registros:
            registro["epoca"] = int(registro["epoca"])
            registro["melhor_distancia"] = float(registro["melhor_distancia"])
            registro["distancia_media"] = float(registro["distancia_media"])
            registro["melhor_rota"] = [int(i) for i in registro["melhor_rota"].replace(",", " ").split()]
        return registros

    @staticmethod
    def _ler_cidades(caminho: Path) -> list[tuple[float, float]]:
        with caminho.open(newline="", encoding="utf-8") as arquivo:
            registros = list(csv.DictReader(arquivo))
        if not registros or not {"indice", "x", "y"}.issubset(registros[0]):
            raise ValueError("cidades.csv vazio ou sem as colunas esperadas.")
        registros.sort(key=lambda linha: int(linha["indice"]))
        return [(float(linha["x"]), float(linha["y"])) for linha in registros]

    def _desenhar(self, historico: list[dict], cidades: list[tuple[float, float]]) -> None:
        epocas = [registro["epoca"] for registro in historico]
        melhores = [registro["melhor_distancia"] for registro in historico]
        medias = [registro["distancia_media"] for registro in historico]
        self.ax_distancia.clear()
        self.ax_distancia.plot(epocas, melhores, label="Melhor distância")
        self.ax_distancia.plot(epocas, medias, label="Média da população", alpha=0.8)
        self.ax_distancia.set_title("Evolução por época")
        self.ax_distancia.set_xlabel("Época")
        self.ax_distancia.set_ylabel("Distância")
        self.ax_distancia.grid(True, alpha=0.25)
        self.ax_distancia.legend()

        rota = historico[-1]["melhor_rota"]
        if not rota or any(indice < 0 or indice >= len(cidades) for indice in rota):
            raise ValueError("A rota do último registro contém índices inválidos.")
        percurso = rota + [rota[0]]
        xs = [cidades[indice][0] for indice in percurso]
        ys = [cidades[indice][1] for indice in percurso]
        self.ax_rota.clear()
        self.ax_rota.plot(xs, ys, "o-", markersize=4)
        self.ax_rota.scatter(xs[0], ys[0], color="red", zorder=3, label="Início/fim")
        self.ax_rota.set_title(f"Melhor rota — época {historico[-1]['epoca']}")
        self.ax_rota.set_xlabel("x")
        self.ax_rota.set_ylabel("y")
        self.ax_rota.axis("equal")
        self.ax_rota.grid(True, alpha=0.25)
        self.ax_rota.legend()
        self.canvas.draw_idle()
        self.status.set(
            f"{len(cidades)} cidades | {len(historico)} épocas | "
            f"melhor distância: {min(melhores):.3f}"
        )


def main() -> None:
    root = tk.Tk()
    Interface(root)
    root.mainloop()


if __name__ == "__main__":
    main()
