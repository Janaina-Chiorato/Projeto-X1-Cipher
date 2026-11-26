import subprocess
import tempfile
from pathlib import Path

PROJETO = Path(__file__).resolve().parent

aprovados = 0
reprovados = 0


def verificar(nome, condicao):
    global aprovados, reprovados

    if condicao:
        print(f"[APROVADO] {nome}")
        aprovados += 1
    else:
        print(f"[REPROVADO] {nome}")
        reprovados += 1


with tempfile.TemporaryDirectory() as pasta:

    ranking = Path(pasta) / "ranking.txt"

    def executar(entradas):
        resultado = subprocess.run(
            [str(PROJETO / "cipher")],
            input=entradas,
            capture_output=True,
            text=True,
            cwd=pasta,
            timeout=5,
            check=True
        )

        return resultado.stdout

    def jogar_humano(dificuldade, limite, nome,
                     testar_invalidos=False):

        processo = subprocess.Popen(
            ["stdbuf", "-o0", str(PROJETO / "cipher")],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            cwd=pasta
        )

        def enviar(valor):
            processo.stdin.write(str(valor) + "\n")
            processo.stdin.flush()

        enviar(1)
        enviar(dificuldade)

        inicio = 1
        fim = limite
        tentativas = 0
        invalidos = 0
        saida = ""
        venceu = False

        try:
            while True:

                caractere = processo.stdout.read(1)

                if not caractere:
                    break

                saida += caractere

                if saida.endswith("Digite seu palpite: "):

                    if testar_invalidos and invalidos < 4:
                        valores = ["abc", 0, limite + 1, -5]
                        enviar(valores[invalidos])
                        invalidos += 1

                    else:
                        meio = inicio + (fim - inicio) // 2
                        enviar(meio)
                        tentativas += 1

                elif saida.endswith("numero secreto e MAIOR!"):
                    inicio = meio + 1

                elif saida.endswith("numero secreto e MENOR!"):
                    fim = meio - 1

                elif saida.endswith("Digite seu nome: "):
                    venceu = True
                    enviar(nome)
                    enviar(0)

                elif saida.endswith("Cipher venceu!"):
                    enviar(0)

            processo.wait(timeout=5)

        finally:
            if processo.poll() is None:
                processo.kill()
                processo.wait()

        return venceu, tentativas, saida

    # Teste 15 - Humano facil
    ranking.write_text("")

    venceu, tentativas, saida = jogar_humano(
        1, 100, "Teste Facil"
    )

    verificar(
        "Modo humano - facil",
        venceu and tentativas <= 7
    )

    # Teste 16 - Humano medio
    venceu, tentativas, saida = jogar_humano(
        2, 1000, "Teste Medio"
    )

    verificar(
        "Modo humano - medio",
        venceu and tentativas <= 10
    )

    # Teste 17 - Humano dificil
    venceu, tentativas, saida = jogar_humano(
        3, 10000, "Teste Dificil"
    )

    verificar(
        "Modo humano - dificil",
        venceu and tentativas <= 14
    )

    # Teste 18 - Palpites invalidos
    venceu, tentativas, saida = jogar_humano(
        1, 100, "Teste Validacao", True
    )

    verificar(
        "Validacao de palpites invalidos",
        venceu
        and tentativas <= 7
        and "Digite um numero valido" in saida
        and saida.count("Digite um numero entre") == 3
    )

    # Teste 19 - Substituicao no ranking cheio
    ranking.write_text(
        "".join(
            f"14 Jogador_{i:02d}\n"
            for i in range(1, 11)
        )
    )

    venceu, tentativas, saida = jogar_humano(
        1, 100, "Novo Campeao"
    )

    dados = ranking.read_text()

    verificar(
        "Substituicao do ultimo colocado",
        venceu
        and "Novo Campeao" in dados
        and "Jogador_10" not in dados
        and len(dados.splitlines()) == 10
    )

    # Teste 20 - Resultado fora do Top 10
    ranking.write_text(
        "".join(
            f"1 Campeao_{i:02d}\n"
            for i in range(1, 11)
        )
    )

    venceu, tentativas, saida = jogar_humano(
        1, 100, "Fora do Ranking"
    )

    verificar(
        "Rejeicao de resultado fora do Top 10",
        venceu
        and "Resultado fora do Top 10" in saida
        and "Fora do Ranking" not in ranking.read_text()
        and len(ranking.read_text().splitlines()) == 10
    )

    # Teste 21 - Registros invalidos
    ranking.write_text(
        "abc Registro invalido\n"
        "0 Jogador invalido\n"
        "99 Outro invalido\n"
        "3 Maria\n"
        "\n"
        "5 Pedro\n"
    )

    saida = executar("3\n0\n")

    verificar(
        "Tratamento de registros invalidos",
        "Maria" in saida
        and "Pedro" in saida
        and len(ranking.read_text().splitlines()) == 2
    )

    # Teste 22 - Codigos de cores ANSI
    saida = executar("0\n")

    verificar(
        "Presenca dos codigos ANSI",
        "\033[36m" in saida
        and "\033[31m" in saida
        and "\033[0m" in saida
    )


print("\n" + "=" * 50)
print("RELATORIO DE TESTES COMPLEMENTARES")
print("=" * 50)
print(f"Total de testes: {aprovados + reprovados}")
print(f"Aprovados: {aprovados}")
print(f"Reprovados: {reprovados}")
print("=" * 50)

raise SystemExit(1 if reprovados else 0)