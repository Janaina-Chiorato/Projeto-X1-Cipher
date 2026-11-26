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


# Compilar o programa
compilacao = subprocess.run(
    [
        "gcc", "-std=c99",
        "-Wall", "-Wextra", "-pedantic",
        "main.c", "-o", "cipher", "-lm"
    ],
    cwd=PROJETO,
    capture_output=True,
    text=True
)

verificar(
    "Compilacao sem erros e avisos",
    compilacao.returncode == 0
    and not compilacao.stderr.strip()
)

if compilacao.returncode != 0:
    print(compilacao.stderr)
    raise SystemExit(1)


# Ambiente isolado para preservar o ranking original
with tempfile.TemporaryDirectory() as pasta:

    ranking = Path(pasta) / "ranking.txt"

    def executar(entradas):
        resultado = subprocess.run(
            [str(PROJETO / "cipher")],
            input=entradas,
            capture_output=True,
            text=True,
            cwd=pasta,
            timeout=5
        )

        if resultado.returncode != 0:
            raise RuntimeError(resultado.stderr)

        return resultado.stdout

    # Teste 02 - Menu
    saida = executar("0\n")

    verificar(
        "Menu principal",
        "Jogar - Humano vs Cipher" in saida
        and "Modo IA - Busca Binaria" in saida
        and "Ver Ranking" in saida
    )

    # Teste 03 - Entradas invalidas
    saida = executar("abc\n9\n-1\n2abc\n0\n")

    verificar(
        "Validacao de entradas numericas",
        "Digite um numero valido" in saida
        and saida.count("Digite um numero entre") == 2
        and "Entrada invalida" in saida
    )

    # Testes da busca binaria
    def testar_ia(dificuldade, limite, alvo, maximo):

        inicio = 1
        fim = limite
        respostas = []
        tentativas = 0

        while inicio <= fim:

            meio = inicio + (fim - inicio) // 2
            tentativas += 1

            if alvo == meio:
                respostas.append("=")
                break

            elif alvo > meio:
                respostas.append(">")
                inicio = meio + 1

            else:
                respostas.append("<")
                fim = meio - 1

        entradas = (
            f"2\n{dificuldade}\n"
            + "\n".join(respostas)
            + "\n0\n"
        )

        saida = executar(entradas)

        return (
            f"NUMERO DESCOBERTO: {alvo}\n" in saida
            and f"Total de tentativas: {tentativas}\n" in saida
            and tentativas <= maximo
        )

    verificar(
        "IA - nivel facil",
        testar_ia(1, 100, 80, 7)
    )

    verificar(
        "IA - nivel medio",
        testar_ia(2, 1000, 800, 10)
    )

    verificar(
        "IA - nivel dificil",
        testar_ia(3, 10000, 7500, 14)
    )

    # Pior caso da busca binaria
    verificar(
        "Busca binaria - pior caso",
        all([
            testar_ia(1, 100, 100, 7),
            testar_ia(2, 1000, 1000, 10),
            testar_ia(3, 10000, 10000, 14)
        ])
    )

    # Respostas contraditorias
    saida = executar("2\n1\n<\n>\n<\n<\n<\n<\n0\n")

    verificar(
        "Respostas contraditorias",
        "Respostas inconsistentes" in saida
    )

    # Simbolos invalidos
    saida = executar("2\n1\n88\nabc\n=\n0\n")

    verificar(
        "Validacao dos simbolos da IA",
        saida.count("Resposta invalida") == 2
        and "Total de tentativas: 1" in saida
    )

    # Ranking vazio
    ranking.write_text("")

    saida = executar("3\n0\n")

    verificar(
        "Ranking vazio",
        "Nenhum jogador registrado" in saida
    )

    # Ordenacao do ranking
    ranking.write_text(
        "7 Maria\n"
        "2 Pedro\n"
        "4 Ana\n"
    )

    saida = executar("3\n0\n")

    verificar(
        "Ordenacao do ranking",
        0 <= saida.find("Pedro")
        < saida.find("Ana")
        < saida.find("Maria")
    )

    # Limite de jogadores
    ranking.write_text(
        "".join(
            f"{i} Jogador_{i:02d}\n"
            for i in range(1, 13)
        )
    )

    saida = executar("3\n0\n")

    verificar(
        "Limite de 10 jogadores",
        "Jogador_10" in saida
        and "Jogador_11" not in saida
        and "Jogador_12" not in saida
        and len(ranking.read_text().splitlines()) == 10
    )

    # Persistencia
    ranking.write_text("3 Maria\n")

    executar("3\n0\n")
    saida = executar("3\n0\n")

    verificar(
        "Persistencia do ranking",
        "Maria" in saida
        and ranking.read_text().strip() == "3 Maria"
    )

    # Encerramento
    saida = executar("0\n")

    verificar(
        "Encerramento do programa",
        "Terminal encerrado" in saida
    )


# Relatorio final
print("\n" + "=" * 45)
print("RELATORIO FINAL - PROJETO CIPHER")
print("=" * 45)
print(f"Total de testes: {aprovados + reprovados}")
print(f"Aprovados: {aprovados}")
print(f"Reprovados: {reprovados}")
print("=" * 45)

raise SystemExit(1 if reprovados else 0)