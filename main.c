#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// Constantes do sistema
#define MAX_RANKING 10
#define MAX_HISTORICO 14
#define ARQUIVO_RANKING "ranking.txt"

// Cores ANSI
#define RESET    "\033[0m"
#define VERMELHO "\033[31m"
#define VERDE    "\033[32m"
#define AMARELO  "\033[33m"
#define AZUL     "\033[34m"
#define CIANO    "\033[36m"
#define NEGRITO  "\033[1m"

// Estrutura utilizada para armazenar os jogadores
typedef struct Jogador {
    char nome[50];
    int tentativas;
} Jogador;

// Estrutura utilizada para armazenar os palpites
typedef struct {
    int numero;
    int distancia;
} Palpite;

// Limpar o buffer de entrada
void limparBuffer(void) {
    int c;

    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

// Ler e validar numeros inteiros
int lerInteiro(const char *mensagem, int minimo, int maximo) {
    char entrada[100];
    char *fim;
    long valor;

    while (1) {
        printf("%s", mensagem);

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            printf("\nEntrada encerrada.\n");
            exit(EXIT_SUCCESS);
        }

        if (strchr(entrada, '\n') == NULL && !feof(stdin)) {
            limparBuffer();
            printf(VERMELHO "Entrada muito longa!\n" RESET);
            continue;
        }

        valor = strtol(entrada, &fim, 10);

        if (fim == entrada) {
            printf(VERMELHO "Digite um numero valido!\n" RESET);
            continue;
        }

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        if (*fim != '\0') {
            printf(VERMELHO "Entrada invalida!\n" RESET);
            continue;
        }

        if (valor < minimo || valor > maximo) {
            printf(VERMELHO
                   "Digite um numero entre %d e %d.\n"
                   RESET, minimo, maximo);
            continue;
        }

        return (int)valor;
    }
}

// Exibir cabecalho do sistema
void cabecalho(void) {
    printf(CIANO NEGRITO);
    printf("\n==============================================\n");
    printf("       CIPHER - TERMINAL DE SEGURANCA\n");
    printf("==============================================\n");
    printf(RESET);
}

// Exibir menu principal
void exibirMenu(void) {
    cabecalho();

    printf(VERMELHO "ACESSO BLOQUEADO!\n" RESET);
    printf("Descubra o codigo secreto para escapar.\n\n");

    printf("[1] Jogar - Humano vs Cipher\n");
    printf("[2] Modo IA - Busca Binaria\n");
    printf("[3] Ver Ranking\n");
    printf("[0] Sair\n");

    printf("==============================================\n");
}

// Escolher nivel de dificuldade
int escolherDificuldade(void) {
    int opcao;

    printf("\n========== DIFICULDADE ==========\n");
    printf("[1] Facil   - 1 a 100\n");
    printf("[2] Medio   - 1 a 1000\n");
    printf("[3] Dificil - 1 a 10000\n");

    opcao = lerInteiro("Escolha: ", 1, 3);

    switch (opcao) {
        case 1:
            return 100;

        case 2:
            return 1000;

        case 3:
            return 10000;

        default:
            return 1000;
    }
}

// Calcular limite de tentativas utilizando log2
int calcularTentativas(int limite) {
    return (int)ceil(log2(limite));
}

// Exibir temperatura do palpite
void exibirTemperatura(int distancia, int limite) {
    double percentual = (double)distancia / limite;

    if (distancia == 0) {
        printf(VERDE "ACERTOU!" RESET);
    }
    else if (percentual <= 0.02) {
        printf(VERMELHO "MUITO QUENTE" RESET);
    }
    else if (percentual <= 0.10) {
        printf(AMARELO "QUENTE" RESET);
    }
    else if (percentual <= 0.25) {
        printf(VERDE "MORNO" RESET);
    }
    else {
        printf(AZUL "FRIO" RESET);
    }
}

// Exibir todos os palpites realizados
void exibirHistorico(const Palpite historico[],
                     int quantidade, int limite) {

    printf("\n========== HISTORICO ==========\n");

    for (int i = 0; i < quantidade; i++) {
        printf("Tentativa %02d: %5d - ",
               i + 1, historico[i].numero);

        exibirTemperatura(historico[i].distancia, limite);

        printf("\n");
    }

    printf("================================\n");
}

// Ordenar ranking utilizando Insertion Sort
// O vetor possui capacidade maxima de 10 jogadores.
void ordenarRanking(Jogador ranking[], int quantidade) {
    for (int i = 1; i < quantidade; i++) {
        Jogador chave = ranking[i];
        int j = i - 1;

        while (j >= 0 &&
               ranking[j].tentativas > chave.tentativas) {

            ranking[j + 1] = ranking[j];
            j--;
        }

        ranking[j + 1] = chave;
    }
}

// Salvar ranking em arquivo
int salvarRanking(const Jogador ranking[], int quantidade) {
    FILE *arquivo = fopen(ARQUIVO_RANKING, "w");
    int sucesso = 1;

    if (arquivo == NULL) {
        printf(VERMELHO
               "Erro ao salvar ranking!\n" RESET);
        return 0;
    }

    for (int i = 0; i < quantidade; i++) {
        if (fprintf(arquivo, "%d %s\n",
                    ranking[i].tentativas,
                    ranking[i].nome) < 0) {
            sucesso = 0;
            break;
        }
    }

    if (fclose(arquivo) != 0) {
        sucesso = 0;
    }

    if (!sucesso) {
        printf(VERMELHO
               "Erro ao gravar o arquivo!\n" RESET);
    }

    return sucesso;
}

// Carregar ranking salvo anteriormente
// Funcao corrigida utilizando fscanf.
void carregarRanking(Jogador ranking[], int *quantidade) {
    FILE *arquivo = fopen(ARQUIVO_RANKING, "r");
    Jogador jogador;
    int resultado;
    int c;
    char linha[150];
    char *fim;
    long tentativas;

    *quantidade = 0;

    if (arquivo == NULL) {
        return;
    }

    while (*quantidade < MAX_RANKING) {

        // Ler uma linha sem ultrapassar o tamanho do vetor
        resultado = fscanf(arquivo, "%149[^\n]", linha);

        if (resultado == EOF) {
            break;
        }

        // Consumir a quebra de linha ou o restante da linha
        c = fgetc(arquivo);

        if (c != '\n' && c != EOF) {
            while ((c = fgetc(arquivo)) != '\n' && c != EOF) {
            }

            continue;
        }

        if (resultado != 1) {
            continue;
        }

        // Validar a quantidade de tentativas
        tentativas = strtol(linha, &fim, 10);

        if (fim == linha ||
            tentativas < 1 ||
            tentativas > MAX_HISTORICO ||
            !isspace((unsigned char)*fim)) {
            continue;
        }

        while (isspace((unsigned char)*fim)) {
            fim++;
        }

        fim[strcspn(fim, "\r\n")] = '\0';

        // Validar o nome armazenado
        if (*fim == '\0' || strlen(fim) >= sizeof(jogador.nome)) {
            continue;
        }

        jogador.tentativas = (int)tentativas;
        strcpy(jogador.nome, fim);

        ranking[*quantidade] = jogador;
        (*quantidade)++;
    }

    fclose(arquivo);

    ordenarRanking(ranking, *quantidade);
}

// Ler nome do jogador
void lerNome(char nome[50]) {
    while (1) {
        printf("Digite seu nome: ");

        if (fgets(nome, 50, stdin) == NULL) {
            exit(EXIT_SUCCESS);
        }

        if (strchr(nome, '\n') == NULL && !feof(stdin)) {
            int c = getchar();

            if (c != '\n' && c != EOF) {
                limparBuffer();
                printf("Nome muito longo! Maximo de 49 caracteres.\n");
                continue;
            }
        }

        nome[strcspn(nome, "\r\n")] = '\0';

        int possuiTexto = 0;

        for (int i = 0; nome[i] != '\0'; i++) {
            if (!isspace((unsigned char)nome[i])) {
                possuiTexto = 1;
                break;
            }
        }

        if (possuiTexto) {
            return;
        }

        printf("O nome nao pode estar vazio!\n");
    }
}

// Registrar resultado no ranking
void registrarRanking(Jogador ranking[],
                      int *quantidade,
                      int tentativas) {

    Jogador novo;

    lerNome(novo.nome);

    novo.tentativas = tentativas;

    if (*quantidade < MAX_RANKING) {
        ranking[*quantidade] = novo;
        (*quantidade)++;
    }
    else if (tentativas <
             ranking[MAX_RANKING - 1].tentativas) {

        ranking[MAX_RANKING - 1] = novo;
    }
    else {
        printf("\nResultado fora do Top 10.\n");
        return;
    }

    ordenarRanking(ranking, *quantidade);

    if (salvarRanking(ranking, *quantidade)) {
        printf(VERDE
               "\nResultado registrado no ranking!\n"
               RESET);
    }
    else {
        printf(AMARELO
               "Resultado mantido apenas na memoria.\n"
               RESET);
    }
}

// Exibir ranking
void exibirRanking(const Jogador ranking[], int quantidade) {
    printf(CIANO NEGRITO);
    printf("\n================================================\n");
    printf("          RANKING - MELHORES HACKERS\n");
    printf("================================================\n");
    printf(RESET);

    if (quantidade == 0) {
        printf("Nenhum jogador registrado.\n");
        return;
    }

    printf("%-8s %-25s %s\n",
           "POSICAO", "JOGADOR", "TENTATIVAS");

    printf("------------------------------------------------\n");

    for (int i = 0; i < quantidade; i++) {
        printf("%-8d %-25s %d\n",
               i + 1,
               ranking[i].nome,
               ranking[i].tentativas);
    }

    printf("================================================\n");
}

// Exibir dicas para o jogador
void exibirDica(int palpite, int secreto) {
    if (palpite < secreto) {
        printf(AMARELO
               "CIPHER: O numero secreto e MAIOR!\n"
               RESET);
    }
    else if (palpite > secreto) {
        printf(AMARELO
               "CIPHER: O numero secreto e MENOR!\n"
               RESET);
    }
    else {
        printf(VERDE "CIPHER: Voce ACERTOU!\n" RESET);
    }
}

// Modo humano
void jogarHumano(Jogador ranking[], int *quantidade) {
    int limite = escolherDificuldade();
    int maxTentativas = calcularTentativas(limite);
    int secreto = rand() % limite + 1;
    int tentativas = 0;
    int palpite;

    // Vetor de structs para armazenar o historico.
    // A capacidade de 14 corresponde ao nivel dificil.
    Palpite historico[MAX_HISTORICO];

    cabecalho();

    printf("Intervalo: 1 a %d\n", limite);
    printf("Tentativas permitidas: %d\n", maxTentativas);

    do {
        printf("\nTentativa %d/%d\n",
               tentativas + 1, maxTentativas);

        palpite = lerInteiro("Digite seu palpite: ",
                             1, limite);

        historico[tentativas].numero = palpite;

        historico[tentativas].distancia =
            abs(secreto - palpite);

        tentativas++;

        exibirDica(palpite, secreto);

        exibirHistorico(historico, tentativas, limite);

    } while (palpite != secreto &&
             tentativas < maxTentativas);

    if (palpite == secreto) {
        printf(VERDE NEGRITO);
        printf("\nACESSO LIBERADO!\n");
        printf("Voce derrotou Cipher em %d tentativas!\n",
               tentativas);
        printf(RESET);

        registrarRanking(ranking, quantidade, tentativas);
    }
    else {
        printf(VERMELHO NEGRITO);
        printf("\nACESSO NEGADO!\n");
        printf("Cipher venceu!\n");
        printf("O codigo secreto era: %d\n", secreto);
        printf(RESET);
    }
}

// Ler e validar resposta do modo IA
char lerRespostaIA(void) {
    char entrada[50];

    while (1) {
        printf("Resposta (>, < ou =): ");

        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            exit(EXIT_SUCCESS);
        }

        if (strchr(entrada, '\n') == NULL && !feof(stdin)) {
            limparBuffer();
            printf("Resposta invalida!\n");
            continue;
        }

        int i = 0;

        while (isspace((unsigned char)entrada[i])) {
            i++;
        }

        char resposta = entrada[i];

        if (resposta == '>' ||
            resposta == '<' ||
            resposta == '=') {

            i++;

            while (isspace((unsigned char)entrada[i])) {
                i++;
            }

            if (entrada[i] == '\0') {
                return resposta;
            }
        }

        printf(VERMELHO
               "Resposta invalida! Utilize >, < ou =.\n"
               RESET);
    }
}

// Exibir historico do modo IA
void exibirHistoricoIA(const int historico[], int quantidade) {
    printf("\nHistorico de chutes da IA: ");

    for (int i = 0; i < quantidade; i++) {
        printf("%d ", historico[i]);
    }

    printf("\n");
}

// Modo IA utilizando busca binaria
void jogarIA(void) {
    int limite = escolherDificuldade();
    int maxTentativas = calcularTentativas(limite);

    int inicio = 1;
    int fim = limite;
    int meio;
    int tentativas = 0;

    // Vetor de inteiros para registrar os chutes da IA
    int historico[MAX_HISTORICO];

    char resposta;

    cabecalho();

    printf("Pense em um numero entre 1 e %d.\n", limite);

    printf("\n> O numero pensado e maior.");
    printf("\n< O numero pensado e menor.");
    printf("\n= O computador acertou.\n");

    while (inicio <= fim && tentativas < maxTentativas) {

        // Busca binaria: divide o intervalo pela metade
        meio = inicio + (fim - inicio) / 2;

        historico[tentativas] = meio;
        tentativas++;

        printf(CIANO "\nTentativa %d/%d: %d\n" RESET,
               tentativas, maxTentativas, meio);

        resposta = lerRespostaIA();

        exibirHistoricoIA(historico, tentativas);

        if (resposta == '=') {
            printf(VERDE NEGRITO);
            printf("\nNUMERO DESCOBERTO: %d\n", meio);
            printf("Total de tentativas: %d\n", tentativas);
            printf(RESET);

            return;
        }
        else if (resposta == '>') {
            inicio = meio + 1;
        }
        else {
            fim = meio - 1;
        }
    }

    printf(VERMELHO);
    printf("\nRespostas inconsistentes!\n");
    printf("Nao foi possivel descobrir o numero.\n");
    printf(RESET);
}

// Funcao principal
int main(void) {

    // Ranking local, passado por parametro para as funcoes
    Jogador ranking[MAX_RANKING];

    int quantidade = 0;
    int opcao;

    srand((unsigned int)time(NULL));

    carregarRanking(ranking, &quantidade);

    do {
        exibirMenu();
        opcao = lerInteiro("Selecione uma opcao: ", 0, 3);

        switch (opcao) {
            case 1:
                jogarHumano(ranking, &quantidade);
                break;

            case 2:
                jogarIA();
                break;

            case 3:
                exibirRanking(ranking, quantidade);
                break;

            case 0:
                salvarRanking(ranking, quantidade);
                printf("\nTerminal encerrado. Ate logo!\n");
                break;
        }

    } while (opcao != 0);

    return 0;
}