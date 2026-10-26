import csv
from os import process_cpu_count
import sys
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
        self.processo_ativo: subprocess.Popen | None = None
        self.root.protocol("WM_DELETE_WINDOW", self.ao_fechar)
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
        self.status = tk.StringVar(
            value="Carregue uma pasta com os CSVs dos resultados."
        )
        self.pasta_experimento: Path | None = None
        self.cidades_atuais: list[tuple[float, float]] = []
        self.historico_atual: list[dict] = []
        self.botao_avancar = None
        self.botao_avancar_10 = None
        self.botao_avancar_100 = None
        self.seletor_epoca = None

        self.linha_epoca = None
        self.linha_delta = None
        self.linha_delta_media = None
        self.linha_historico = None
        self.linha_media = None
        self.linha_rota = None
        self.ponto_inicio = None

        self._montar_janela()

    def ao_fechar(self) -> None:
        if self.processo_ativo and self.processo_ativo.poll() is None:
            try:
                self.processo_ativo.terminate()

            except Exception:
                pass
        plt.close("all")
        self.root.quit()
        self.root.destroy()

    def _montar_janela(self) -> None:
        painel = ttk.Frame(self.root, padding=12)
        painel.pack(fill=tk.BOTH, expand=True)

        controles = ttk.LabelFrame(painel, text="Parâmetros do experimento", padding=12)
        controles.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 12))

        ttk.Label(controles, text="Cenário").grid(row=0, column=0, sticky="w", pady=4)
        ttk.Combobox(
            controles,
            textvariable=self.vars["cenario"],
            values=("Aleatório", "Circular"),
            state="readonly",
            width=20,
        ).grid(row=0, column=1, pady=4)

        campos = (
            ("Quantidade de cidades", "cidades"),
            ("Tamanho da população", "populacao"),
            ("Taxa de mutação (0–1)", "mutacao"),
            ("Máximo de épocas", "epocas"),
            ("Épocas sem melhora", "sem_melhora"),
        )
        for linha, (rotulo, chave) in enumerate(campos, start=1):
            ttk.Label(controles, text=rotulo).grid(
                row=linha, column=0, sticky="w", pady=4
            )
            ttk.Entry(controles, textvariable=self.vars[chave], width=23).grid(
                row=linha, column=1, pady=4
            )

        ttk.Button(
            controles, text="Salvar configuração", command=self.salvar_configuracao
        ).grid(row=6, column=0, columnspan=2, sticky="ew", pady=(12, 4))
        ttk.Separator(controles).grid(
            row=7, column=0, columnspan=2, sticky="ew", pady=10
        )
        ttk.Button(
            controles, text="Iniciar experimento...", command=self.iniciar_experimento
        ).grid(row=8, column=0, columnspan=2, sticky="ew", pady=4)
        self.botao_avancar = ttk.Button(
            controles,
            text="Avançar 1 época",
            command=lambda: self.avancar_epocas(1),
            state=tk.DISABLED,
        )
        self.botao_avancar.grid(row=9, column=0, columnspan=2, sticky="ew", pady=4)
        self.botao_avancar_10 = ttk.Button(
            controles,
            text="Avançar 10 épocas",
            command=lambda: self.avancar_epocas(10),
            state=tk.DISABLED,
        )
        self.botao_avancar_10.grid(row=10, column=0, columnspan=2, sticky="ew", pady=4)
        self.botao_avancar_100 = ttk.Button(
            controles,
            text="Avançar 100 épocas",
            command=lambda: self.avancar_epocas(100),
            state=tk.DISABLED,
        )
        self.botao_avancar_100.grid(row=11, column=0, columnspan=2, sticky="ew", pady=4)
        ttk.Button(
            controles, text="Abrir resultados (CSV)...", command=self.abrir_resultados
        ).grid(row=12, column=0, columnspan=2, sticky="ew", pady=4)
        ttk.Label(
            controles,
            text="Inicie um experimento e avance uma época por clique. Os gráficos são atualizados a cada avanço.",
            wraplength=230,
        ).grid(row=13, column=0, columnspan=2, sticky="w", pady=(10, 0))

        ttk.Button(
            controles, text="Salvar Gráficos", command=self.salvar_graficos
        ).grid(row=14, column=0, columnspan=2, sticky="ew", pady=4)

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
        ttk.Label(area, textvariable=self.status, anchor="w").pack(
            fill=tk.X, pady=(8, 0)
        )
        self._mostrar_telas_vazias()

    def _mostrar_telas_vazias(self) -> None:
        self.ax_distancia.clear()
        self.ax_distancia.set_title("Evolução por época")
        self.ax_distancia.set_xlabel("Época")
        self.ax_distancia.set_ylabel("Distância Absoluta")
        self.ax_distancia.grid(True, alpha=0.25)

        (self.linha_epoca,) = self.ax_distancia.plot(
            [], [], label="Melhor da época", color="tab:blue"
        )
        (self.linha_historico,) = self.ax_distancia.plot(
            [], [], label="Melhor histórico", linestyle="--", color="tab:green"
        )
        (self.linha_media,) = self.ax_distancia.plot(
            [], [], label="Média da população", alpha=0.8, color="tab:orange"
        )

        # Criação do eixo secundário no lado direito
        if not hasattr(self, "ax_delta"):
            self.ax_delta = self.ax_distancia.twinx()
        else:
            self.ax_delta.clear()
        self.ax_delta.set_ylabel("Variação Época a Época (Δ)")

        (self.linha_delta,) = self.ax_delta.plot(
            [], [], label="Δ Melhor", color="tab:red", alpha=0.6, linestyle="--"
        )
        (self.linha_delta_media,) = self.ax_delta.plot(
            [], [], label="Δ Média", color="tab:blue", alpha=0.6, linestyle="--"
        )

        # Unifica as etiquetas na legenda do eixo principal
        linhas = [
            self.linha_epoca,
            self.linha_historico,
            self.linha_media,
            self.linha_delta,
            self.linha_delta_media,
        ]
        etiquetas = [l.get_label() for l in linhas]
        self.ax_distancia.legend(linhas, etiquetas, loc="upper right")

        # Configuração do gráfico de rota à direita
        self.ax_rota.clear()
        self.ax_rota.set_title("Melhor rota")
        self.ax_rota.set_xlabel("x")
        self.ax_rota.set_ylabel("y")
        self.ax_rota.grid(True, alpha=0.25)
        (self.linha_rota,) = self.ax_rota.plot([], [], "o-", markersize=3)
        (self.ponto_inicio,) = self.ax_rota.plot(
            [], [], "ro", markersize=6, label="Início/fim"
        )
        self.ax_rota.legend()
        self.canvas.draw_idle()

    def _ler_configuracao(self) -> dict:
        cidades = int(self.vars["cidades"].get())
        populacao = int(self.vars["populacao"].get())
        mutacao = float(self.vars["mutacao"].get())
        epocas = int(self.vars["epocas"].get())
        sem_melhora = int(self.vars["sem_melhora"].get())
        if (
            cidades < 8
            or populacao < 2
            or not 0 <= mutacao <= 1
            or epocas < 1
            or sem_melhora < 1
        ):
            raise ValueError(
                "Use pelo menos 8 cidades, população ≥ 2, mutação entre 0 e 1 e épocas positivas."
            )
        return {
            "cenario": self.vars["cenario"].get().lower(),
            "quantidade_cidades": cidades,
            "tamanho_populacao": populacao,
            "taxa_mutacao": mutacao,
            "max_epocas": epocas,
            "epocas_sem_melhora": sem_melhora,
        }

    def salvar_graficos(
        self, caminho_arquivo: str | Path | None = None, dpi: int = 300
    ) -> None:
        if not self.pasta_experimento or not self.historico_atual:
            messagebox.showwarning("Aviso", "Não há dados desenhados para salvar.")
            return

        if caminho_arquivo is None:
            epoca_atual = self.seletor_epoca.get() if self.seletor_epoca else "final"
            caminho_arquivo = filedialog.asksaveasfilename(
                title="Salvar gráfico como imagem",
                initialfile=f"grafico_epoca_{epoca_atual}.png",
                defaultextension=".png",
                filetypes=(
                    ("Imagem PNG", "*.png"),
                    ("Documento PDF", "*.pdf"),
                    ("Vetor SVG", "*.svg"),
                    ("Imagem JPEG", "*.jpg"),
                ),
            )
            if not caminho_arquivo:
                return

        caminho = Path(caminho_arquivo)
        caminho.parent.mkdir(parents=True, exist_ok=True)

        self.figure.savefig(caminho, dpi=dpi, bbox_inches="tight")
        self.status.set(f"Gráfico salvo com sucesso em: {caminho.name}")

    def salvar_configuracao(self) -> None:
        try:
            config = self._ler_configuracao()
        except ValueError as erro:
            messagebox.showerror("Parâmetros inválidos", str(erro))
            return
        destino = filedialog.asksaveasfilename(
            title="Salvar configuração",
            initialfile="configuracao.json",
            defaultextension=".json",
            filetypes=(("JSON", "*.json"),),
        )
        if not destino:
            return
        Path(destino).write_text(
            json.dumps(config, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
        )
        self.status.set(f"Configuração salva em {destino}")

    def abrir_resultados(self) -> None:
        pasta = filedialog.askdirectory(
            title="Escolha a pasta com historico.csv e cidades.csv"
        )
        if not pasta:
            return
        try:
            historico = self._ler_historico(Path(pasta) / "historico.csv")
            cidades = self._ler_cidades(Path(pasta) / "cidades.csv")
            self.pasta_experimento = (
                Path(pasta) if (Path(pasta) / "estado.dat").exists() else None
            )
            self.cidades_atuais = cidades
            self.historico_atual = historico
            self._linhas_lidas = len(historico) + 1
            if self.pasta_experimento:
                self.botao_avancar.configure(state=tk.NORMAL)
                self.botao_avancar_10.configure(state=tk.NORMAL)
                self.botao_avancar_100.configure(state=tk.NORMAL)
            else:
                self.botao_avancar.configure(state=tk.DISABLED)
                self.botao_avancar_10.configure(state=tk.DISABLED)
                self.botao_avancar_100.configure(state=tk.DISABLED)
            self._desenhar(historico, cidades)
        except (OSError, ValueError, KeyError) as erro:
            messagebox.showerror("Não foi possível abrir os resultados", str(erro))

    def _executavel(self) -> Path:
        pasta = Path(__file__).resolve().parent
        fontes = [
            pasta / "main.cpp",
            pasta / "mundo.cpp",
            pasta / "mundo.hpp",
            pasta / "cidade.cpp",
            pasta / "cidade.hpp",
        ]
        executavel = pasta / "caixeiro"
        if not executavel.exists() or any(
            fonte.stat().st_mtime > executavel.stat().st_mtime for fonte in fontes
        ):
            subprocess.run(
                [
                    "g++",
                    "-std=c++17",
                    "-O3",
                    "-march=native",
                    "-fopenmp",
                    "main.cpp",
                    "mundo.cpp",
                    "cidade.cpp",
                    "-o",
                    "caixeiro",
                ],
                cwd=pasta,
                check=True,
                capture_output=True,
                text=True,
            )
        return executavel

    def iniciar_experimento(self) -> None:
        try:
            config = self._ler_configuracao()
        except ValueError as erro:
            messagebox.showerror("Parâmetros inválidos", str(erro))
            return
        destino = filedialog.askdirectory(
            title="Escolha a pasta para salvar o experimento"
        )
        if not destino:
            return
        pasta = Path(destino)
        if any(
            (pasta / nome).exists()
            for nome in ("estado.dat", "historico.csv", "cidades.csv")
        ):
            if not messagebox.askyesno(
                "Substituir experimento?",
                "Esta pasta já contém arquivos de resultados. Deseja iniciar um novo experimento e substituí-los?",
            ):
                return
        try:
            self.processo_ativo = subprocess.Popen(
                [
                    str(self._executavel()),
                    "--start",
                    str(config["quantidade_cidades"]),
                    str(config["tamanho_populacao"]),
                    str(config["taxa_mutacao"]),
                    str(config["max_epocas"]),
                    str(config["epocas_sem_melhora"]),
                    config["cenario"],
                    str(pasta),
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            stdout, stderr = self.processo_ativo.communicate()
            if self.processo_ativo.returncode != 0:
                raise subprocess.CalledProcessError(
                    self.processo_ativo.returncode,
                    self.processo_ativo.args,
                    stdout,
                    stderr,
                )

            self.pasta_experimento = pasta
            self.cidades_atuais = self._ler_cidades(pasta / "cidades.csv")
            self._linhas_lidas = 0
            self.historico_atual = []
            self.botao_avancar.configure(state=tk.NORMAL)
            self.botao_avancar_10.configure(state=tk.NORMAL)
            self.botao_avancar_100.configure(state=tk.NORMAL)
            self._atualizar_experimento()
        except (OSError, subprocess.CalledProcessError) as erro:
            detalhe = (
                (erro.stderr or erro.stdout or str(erro))
                if isinstance(erro, subprocess.CalledProcessError)
                else str(erro)
            )
            messagebox.showerror("Falha", detalhe)
        finally:
            self.processo_ativo = None

    def avancar_epocas(self, quantidade: int) -> None:
        if not self.pasta_experimento:
            return
        try:
            self.processo_ativo = subprocess.Popen(
                [
                    str(self._executavel()),
                    "--step",
                    str(self.pasta_experimento),
                    str(quantidade),
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
            )
            stdout, stderr = self.processo_ativo.communicate()
            if self.processo_ativo.returncode != 0:
                raise subprocess.CalledProcessError(
                    self.processo_ativo.returncode,
                    self.processo_ativo.args,
                    stdout,
                    stderr,
                )
            self._atualizar_experimento()
            if "concluído" in stdout:
                self.botao_avancar.configure(state=tk.DISABLED)
                self.botao_avancar_10.configure(state=tk.DISABLED)
                self.botao_avancar_100.configure(state=tk.DISABLED)
        except (OSError, subprocess.CalledProcessError) as erro:
            detalhe = (
                erro.stderr
                if isinstance(erro, subprocess.CalledProcessError)
                else str(erro)
            )
            messagebox.showerror("Falha ao avançar época", detalhe)
        finally:
            self.processo_ativo = None

    def _atualizar_experimento(self) -> None:
        caminho_hist = self.pasta_experimento / "historico.csv"

        if not hasattr(self, "_linhas_lidas"):
            self._linhas_lidas = 0
            self.historico_atual = []

        with caminho_hist.open(newline="", encoding="utf-8") as f:
            linhas = f.readlines()

        if len(linhas) <= self._linhas_lidas:
            self._linhas_lidas = 1
            self.historico_atual = []

        if (
            not hasattr(self, "_linhas_lidas")
            or self._linhas_lidas < 1
            or len(linhas) <= self._linhas_lidas
        ):
            self._linhas_lidas = 1
            self.historico_atual = []
        novas_linhas = linhas[self._linhas_lidas :]
        self._linhas_lidas = len(linhas)
        cabecalho = [h.strip() for h in linhas[0].strip().split(",")]

        if novas_linhas:
            for linha in csv.DictReader(novas_linhas, fieldnames=cabecalho):
                if linha["epoca"].strip() == "epoca":
                    continue
                linha["epoca"] = int(linha["epoca"])
                linha["melhor_distancia"] = float(linha["melhor_distancia"])
                linha["distancia_media"] = float(linha["distancia_media"])
                linha["distancia_delta"] = float(linha.get("distancia_delta") or 0.0)
                linha["distancia_delta_media"] = float(
                    linha.get("distancia_delta_media") or 0.0
                )
                linha["melhor_geracao"] = float(
                    linha.get("melhor_geracao") or linha["melhor_distancia"]
                )
                rota_str = linha.get("rota_geracao") or linha["melhor_rota"]
                linha["rota_geracao"] = [
                    int(i) for i in rota_str.replace('"', "").replace(",", " ").split()
                ]
                linha["tempo_segundos"] = float(linha.get("tempo_segundos") or 0.0)
                self.historico_atual.append(linha)

        if self.historico_atual:
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
            registro["distancia_delta"] = float(registro.get("distancia_delta") or 0.0)
            registro["distancia_delta_media"] = float(
                registro.get("distancia_delta_media") or 0.0
            )
            registro["melhor_rota"] = [
                int(i) for i in registro["melhor_rota"].replace(",", " ").split()
            ]
            registro["melhor_geracao"] = float(
                registro.get("melhor_geracao") or registro["melhor_distancia"]
            )
            rota_geracao = registro.get("rota_geracao") or registro["melhor_rota"]
            registro["rota_geracao"] = [
                int(i) for i in rota_geracao.replace(",", " ").split()
            ]
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

    def _desenhar(
        self, historico: list[dict], cidades: list[tuple[float, float]]
    ) -> None:
        epocas = [registro["epoca"] for registro in historico]
        melhores = [registro["melhor_distancia"] for registro in historico]
        melhores_epoca = [registro["melhor_geracao"] for registro in historico]
        medias = [registro["distancia_media"] for registro in historico]
        delta = [registro["distancia_delta"] for registro in historico]
        delta_media = [registro["distancia_delta_media"] for registro in historico]
        self.linha_epoca.set_data(epocas, melhores_epoca)
        self.linha_historico.set_data(epocas, melhores)
        self.linha_media.set_data(epocas, medias)
        self.linha_delta.set_data(epocas, delta)
        self.linha_delta_media.set_data(epocas, delta_media)

        self.ax_distancia.relim()
        self.ax_distancia.autoscale_view()

        self.ax_delta.relim()
        self.ax_delta.autoscale_view()

        self.canvas.draw_idle()
        self.seletor_epoca["values"] = [str(e) for e in epocas]
        self.seletor_epoca.set(str(epocas[-1]))
        self._desenhar_rota(historico, cidades, len(historico) - 1)

        tempos = historico[-1]["tempo_segundos"]
        self.status.set(
            f"{len(cidades)} cidades | época {historico[-1]['epoca']} | "
            f"melhor distância: {min(melhores):.3f} | tempo: {tempos:.2f} s"
        )
        self.canvas.draw_idle()

    def _selecionar_epoca(self, _evento=None) -> None:
        if not self.historico_atual or not self.cidades_atuais:
            return
        selecionada = int(self.seletor_epoca.get())
        indice = next(
            i
            for i, registro in enumerate(self.historico_atual)
            if registro["epoca"] == selecionada
        )
        self._desenhar_rota(self.historico_atual, self.cidades_atuais, indice)
        self.canvas.draw_idle()

    def _desenhar_rota(
        self, historico: list[dict], cidades: list[tuple[float, float]], indice: int
    ) -> None:
        registro = historico[indice]
        rota = registro["rota_geracao"]
        if not rota or any(c < 0 or c >= len(cidades) for c in rota):
            raise ValueError("A rota do último registro contém índices inválidos.")
        percurso = rota + [rota[0]]
        xs = [cidades[c][0] for c in percurso]
        ys = [cidades[c][1] for c in percurso]
        self.linha_rota.set_data(xs, ys)
        self.ponto_inicio.set_data([xs[0]], [ys[0]])
        self.ax_rota.set_title(f"Melhor rota da época {registro['epoca']}")

        self.ax_rota.relim()
        self.ax_rota.autoscale_view()
        self.canvas.draw_idle()


def main() -> None:
    root = tk.Tk()
    app = Interface(root)
    root.mainloop()
    # 1. Se foi selecionada uma pasta para o experimento, limpa os ficheiros gerados nela
    if app.pasta_experimento and app.pasta_experimento.exists():
        nomes_alvo = ("estado.dat", "historico.csv", "cidades.csv", "configuracao.json")
        for nome in nomes_alvo:
            caminho_ficheiro = app.pasta_experimento / nome
            if caminho_ficheiro.exists():
                try:
                    caminho_ficheiro.unlink()
                except OSError:
                    pass

    # 2. Limpa eventuais resíduos e o binário compilado na pasta local do script
    pasta_local = Path(__file__).resolve().parent
    for nome in (
        "estado.dat",
        "configuracao.json",
        "historico.csv",
        "cidades.csv",
        "caixeiro",
    ):
        caminho_local = pasta_local / nome
        if caminho_local.exists():
            try:
                caminho_local.unlink()
            except OSError:
                pass
    sys.exit(0)


if __name__ == "__main__":
    main()
