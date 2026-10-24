import csv
import json
import subprocess
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
        self.pasta_experimento: Path | None = None
        self.cidades_atuais: list[tuple[float, float]] = []
        self.historico_atual: list[dict] = []
        self.botao_avancar = None
        self.botao_avancar_10 = None
        self.seletor_epoca = None
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
        ttk.Button(controles, text="Iniciar experimento...", command=self.iniciar_experimento).grid(
            row=8, column=0, columnspan=2, sticky="ew", pady=4
        )
        self.botao_avancar = ttk.Button(
            controles, text="Avançar época", command=lambda: self.avancar_epocas(1), state=tk.DISABLED
        )
        self.botao_avancar.grid(row=9, column=0, columnspan=2, sticky="ew", pady=4)
        self.botao_avancar_10 = ttk.Button(
            controles, text="Avançar 10 épocas", command=lambda: self.avancar_epocas(10), state=tk.DISABLED
        )
        self.botao_avancar_10.grid(row=10, column=0, columnspan=2, sticky="ew", pady=4)
        ttk.Button(controles, text="Abrir resultados (CSV)...", command=self.abrir_resultados).grid(
            row=11, column=0, columnspan=2, sticky="ew", pady=4
        )
        ttk.Label(
            controles,
            text="Inicie um experimento e avance uma época por clique. Os gráficos são atualizados a cada avanço.",
            wraplength=230,
        ).grid(row=12, column=0, columnspan=2, sticky="w", pady=(10, 0))

        area = ttk.Frame(painel)
        area.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)
        barra_epoca = ttk.Frame(area)
        barra_epoca.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(barra_epoca, text="Rota da época:").pack(side=tk.LEFT)
        self.seletor_epoca = ttk.Combobox(barra_epoca, state="readonly", width=10)
        self.seletor_epoca.pack(side=tk.LEFT, padx=6)
        self.seletor_epoca.bind("<<ComboboxSelected>>", self._selecionar_epoca)
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
            self.pasta_experimento = Path(pasta) if (Path(pasta) / "estado.dat").exists() else None
            self.cidades_atuais = cidades
            self.historico_atual = historico
            if self.pasta_experimento:
                self.botao_avancar.configure(state=tk.NORMAL)
                self.botao_avancar_10.configure(state=tk.NORMAL)
            else:
                self.botao_avancar.configure(state=tk.DISABLED)
                self.botao_avancar_10.configure(state=tk.DISABLED)
            self._desenhar(historico, cidades)
        except (OSError, ValueError, KeyError) as erro:
            messagebox.showerror("Não foi possível abrir os resultados", str(erro))

    def _executavel(self) -> Path:
        pasta = Path(__file__).resolve().parent
        fontes = [pasta / "main.cpp", pasta / "mundo.cpp", pasta / "mundo.hpp", pasta / "cidade.cpp", pasta / "cidade.hpp"]
        executavel = pasta / "caixeiro"
        if not executavel.exists() or any(fonte.stat().st_mtime > executavel.stat().st_mtime for fonte in fontes):
            subprocess.run(
                ["g++", "-std=c++17", "-O2", "main.cpp", "mundo.cpp", "cidade.cpp", "-o", "caixeiro"],
                cwd=pasta, check=True, capture_output=True, text=True,
            )
        return executavel

    def iniciar_experimento(self) -> None:
        try:
            config = self._ler_configuracao()
        except ValueError as erro:
            messagebox.showerror("Parâmetros inválidos", str(erro))
            return
        destino = filedialog.askdirectory(title="Escolha a pasta para salvar o experimento")
        if not destino:
            return
        pasta = Path(destino)
        if any((pasta / nome).exists() for nome in ("estado.dat", "historico.csv", "cidades.csv")):
            if not messagebox.askyesno("Substituir experimento?", "Esta pasta já contém arquivos de resultados. Deseja iniciar um novo experimento e substituí-los?"):
                return
        try:
            subprocess.run(
                [str(self._executavel()), "--start", str(config["quantidade_cidades"]),
                 str(config["tamanho_populacao"]), str(config["taxa_mutacao"]),
                 str(config["max_epocas"]), str(config["epocas_sem_melhora"]),
                 config["cenario"], str(pasta)], check=True, capture_output=True, text=True,
            )
            self.pasta_experimento = pasta
            self.botao_avancar.configure(state=tk.NORMAL)
            self.botao_avancar_10.configure(state=tk.NORMAL)
            self._atualizar_experimento()
        except (OSError, subprocess.CalledProcessError) as erro:
            detalhe = erro.stderr if isinstance(erro, subprocess.CalledProcessError) else str(erro)
            messagebox.showerror("Falha ao iniciar o experimento", detalhe)

    def avancar_epocas(self, quantidade: int) -> None:
        if not self.pasta_experimento:
            return
        try:
            resultado = subprocess.run(
                [str(self._executavel()), "--step", str(self.pasta_experimento), str(quantidade)],
                check=True, capture_output=True, text=True,
            )
            self._atualizar_experimento()
            if "concluído" in resultado.stdout:
                self.botao_avancar.configure(state=tk.DISABLED)
                self.botao_avancar_10.configure(state=tk.DISABLED)
        except (OSError, subprocess.CalledProcessError) as erro:
            detalhe = erro.stderr if isinstance(erro, subprocess.CalledProcessError) else str(erro)
            messagebox.showerror("Falha ao avançar época", detalhe)

    def _atualizar_experimento(self) -> None:
        assert self.pasta_experimento is not None
        self.historico_atual = self._ler_historico(self.pasta_experimento / "historico.csv")
        self.cidades_atuais = self._ler_cidades(self.pasta_experimento / "cidades.csv")
        self._desenhar(self.historico_atual, self.cidades_atuais)

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
            registro["melhor_geracao"] = float(registro.get("melhor_geracao") or registro["melhor_distancia"])
            rota_geracao = registro.get("rota_geracao") or registro["melhor_rota"]
            registro["rota_geracao"] = [int(i) for i in rota_geracao.replace(",", " ").split()]
            registro["tempo_segundos"] = float(registro.get("tempo_segundos") or 0.0)
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
        melhores_epoca = [registro["melhor_geracao"] for registro in historico]
        medias = [registro["distancia_media"] for registro in historico]
        self.ax_distancia.clear()
        self.ax_distancia.plot(epocas, melhores_epoca, label="Melhor da época")
        self.ax_distancia.plot(epocas, melhores, label="Melhor histórico", linestyle="--")
        self.ax_distancia.plot(epocas, medias, label="Média da população", alpha=0.8)
        self.ax_distancia.set_title("Evolução por época")
        self.ax_distancia.set_xlabel("Época")
        self.ax_distancia.set_ylabel("Distância")
        self.ax_distancia.grid(True, alpha=0.25)
        self.ax_distancia.legend()

        self.seletor_epoca["values"] = [str(epoca) for epoca in epocas]
        self.seletor_epoca.set(str(epocas[-1]))
        self._desenhar_rota(historico, cidades, len(historico) - 1)
        tempos = historico[-1]["tempo_segundos"]
        self.canvas.draw_idle()
        self.status.set(
            f"{len(cidades)} cidades | época {historico[-1]['epoca']} | "
            f"melhor distância: {min(melhores):.3f} | tempo: {tempos:.2f} s"
        )

    def _selecionar_epoca(self, _evento=None) -> None:
        if not self.historico_atual or not self.cidades_atuais:
            return
        selecionada = int(self.seletor_epoca.get())
        indice = next(i for i, registro in enumerate(self.historico_atual)
                      if registro["epoca"] == selecionada)
        self._desenhar_rota(self.historico_atual, self.cidades_atuais, indice)
        self.canvas.draw_idle()

    def _desenhar_rota(self, historico: list[dict], cidades: list[tuple[float, float]], indice: int) -> None:
        registro = historico[indice]
        rota = registro["rota_geracao"]
        if not rota or any(indice < 0 or indice >= len(cidades) for indice in rota):
            raise ValueError("A rota do último registro contém índices inválidos.")
        percurso = rota + [rota[0]]
        xs = [cidades[indice][0] for indice in percurso]
        ys = [cidades[indice][1] for indice in percurso]
        self.ax_rota.clear()
        self.ax_rota.plot(xs, ys, "o-", markersize=4)
        self.ax_rota.scatter(xs[0], ys[0], color="red", zorder=3, label="Início/fim")
        self.ax_rota.set_title(f"Melhor rota da época {registro['epoca']}")
        self.ax_rota.set_xlabel("x")
        self.ax_rota.set_ylabel("y")
        self.ax_rota.axis("equal")
        self.ax_rota.grid(True, alpha=0.25)
        self.ax_rota.legend()


def main() -> None:
    root = tk.Tk()
    Interface(root)
    root.mainloop()


if __name__ == "__main__":
    main()
