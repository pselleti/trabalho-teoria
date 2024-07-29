#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIMBOLOS 256
#define MAX_PRODUCOES 256
#define MAX_TAMANHO_PRODUCAO 64

typedef struct {
    char nao_terminal;
    char producoes[MAX_PRODUCOES][MAX_TAMANHO_PRODUCAO];
    int quantidade_producoes;
} Regra;

typedef struct {
    Regra regras[MAX_SIMBOLOS];
    int quantidade_regras;
    char simbolo_inicial;
} Gramatica;

void inicializar_gramatica(Gramatica *gramatica) {
    gramatica->quantidade_regras = 0;
}

void adicionar_regra(Gramatica *gramatica, char nao_terminal, const char *producao) {
    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        if (gramatica->regras[i].nao_terminal == nao_terminal) {
            strcpy(gramatica->regras[i].producoes[gramatica->regras[i].quantidade_producoes++], producao);
            return;
        }
    }
    gramatica->regras[gramatica->quantidade_regras].nao_terminal = nao_terminal;
    gramatica->regras[gramatica->quantidade_regras].quantidade_producoes = 1;
    strcpy(gramatica->regras[gramatica->quantidade_regras].producoes[0], producao);
    gramatica->quantidade_regras++;
}

void ler_entrada(const char *nome_arquivo, Gramatica *gramatica) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir arquivo de entrada");
        exit(EXIT_FAILURE);
    }

    inicializar_gramatica(gramatica);
    gramatica->simbolo_inicial = 'S';  // Assumindo 'S' como símbolo inicial

    char linha[MAX_TAMANHO_PRODUCAO];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char nao_terminal = linha[0];
        char *producao = strtok(linha + 3, "|");
        while (producao) {
            producao[strcspn(producao, "\n")] = '\0';
            adicionar_regra(gramatica, nao_terminal, producao);
            producao = strtok(NULL, "|");
        }
    }

    fclose(arquivo);
}

void remover_simbolos_inalcancaveis(Gramatica *gramatica) {
    bool alcancavel[MAX_SIMBOLOS] = { false };
    alcancavel[gramatica->simbolo_inicial] = true;

    bool alterado;
    do {
        alterado = false;
        for (int i = 0; i < gramatica->quantidade_regras; ++i) {
            if (alcancavel[gramatica->regras[i].nao_terminal]) {
                for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
                    for (int k = 0; gramatica->regras[i].producoes[j][k] != '\0'; ++k) {
                        if (gramatica->regras[i].producoes[j][k] >= 'A' && gramatica->regras[i].producoes[j][k] <= 'Z' &&
                            !alcancavel[gramatica->regras[i].producoes[j][k]]) {
                            alcancavel[gramatica->regras[i].producoes[j][k]] = true;
                            alterado = true;
                        }
                    }
                }
            }
        }
    } while (alterado);

    int nova_quantidade_regras = 0;
    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        if (alcancavel[gramatica->regras[i].nao_terminal]) {
            gramatica->regras[nova_quantidade_regras++] = gramatica->regras[i];
        }
    }
    gramatica->quantidade_regras = nova_quantidade_regras;
}

void remover_producoes_vazias(Gramatica *gramatica) {
    bool anulavel[MAX_SIMBOLOS] = { false };

    bool alterado;
    do {
        alterado = false;
        for (int i = 0; i < gramatica->quantidade_regras; ++i) {
            for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
                if (strcmp(gramatica->regras[i].producoes[j], "ε") == 0) {
                    if (!anulavel[gramatica->regras[i].nao_terminal]) {
                        anulavel[gramatica->regras[i].nao_terminal] = true;
                        alterado = true;
                    }
                } else {
                    bool todos_anulaveis = true;
                    for (int k = 0; gramatica->regras[i].producoes[j][k] != '\0'; ++k) {
                        if (!anulavel[gramatica->regras[i].producoes[j][k]]) {
                            todos_anulaveis = false;
                            break;
                        }
                    }
                    if (todos_anulaveis && !anulavel[gramatica->regras[i].nao_terminal]) {
                        anulavel[gramatica->regras[i].nao_terminal] = true;
                        alterado = true;
                    }
                }
            }
        }
    } while (alterado);

    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        int nova_quantidade_producoes = 0;
        for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
            if (strcmp(gramatica->regras[i].producoes[j], "ε") != 0) {
                strcpy(gramatica->regras[i].producoes[nova_quantidade_producoes++], gramatica->regras[i].producoes[j]);
            }
        }
        gramatica->regras[i].quantidade_producoes = nova_quantidade_producoes;
    }

    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
            for (int k = 0; gramatica->regras[i].producoes[j][k] != '\0'; ++k) {
                if (anulavel[gramatica->regras[i].producoes[j][k]]) {
                    char nova_producao[MAX_TAMANHO_PRODUCAO];
                    int l = 0;
                    for (int m = 0; gramatica->regras[i].producoes[j][m] != '\0'; ++m) {
                        if (gramatica->regras[i].producoes[j][m] != gramatica->regras[i].producoes[j][k]) {
                            nova_producao[l++] = gramatica->regras[i].producoes[j][m];
                        }
                    }
                    nova_producao[l] = '\0';
                    if (l > 0) {
                        strcpy(gramatica->regras[i].producoes[gramatica->regras[i].quantidade_producoes++], nova_producao);
                    }
                }
            }
        }
    }
}

void substituir_producoes_unitarias(Gramatica *gramatica) {
    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
            if (strlen(gramatica->regras[i].producoes[j]) == 1 &&
                gramatica->regras[i].producoes[j][0] >= 'A' &&
                gramatica->regras[i].producoes[j][0] <= 'Z') {
                char unidade_nao_terminal = gramatica->regras[i].producoes[j][0];
                for (int k = 0; k < gramatica->quantidade_regras; ++k) {
                    if (gramatica->regras[k].nao_terminal == unidade_nao_terminal) {
                        for (int l = 0; l < gramatica->regras[k].quantidade_producoes; ++l) {
                            strcpy(gramatica->regras[i].producoes[gramatica->regras[i].quantidade_producoes++],
                                   gramatica->regras[k].producoes[l]);
                        }
                    }
                }
            }
        }
    }
}

void simplificar_gramatica(Gramatica *gramatica) {
    remover_simbolos_inalcancaveis(gramatica);
    remover_producoes_vazias(gramatica);
    substituir_producoes_unitarias(gramatica);
}

void escrever_saida(const char *nome_arquivo, const Gramatica *gramatica) {
    FILE *arquivo = fopen(nome_arquivo, "w");
    if (!arquivo) {
        perror("Erro ao abrir arquivo de saída");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < gramatica->quantidade_regras; ++i) {
        fprintf(arquivo, "%c -> ", gramatica->regras[i].nao_terminal);
        for (int j = 0; j < gramatica->regras[i].quantidade_producoes; ++j) {
            if (j > 0) {
                fprintf(arquivo, " | ");
            }
            fprintf(arquivo, "%s", gramatica->regras[i].producoes[j]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_saida>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Gramatica gramatica;
    ler_entrada(argv[1], &gramatica);

    simplificar_gramatica(&gramatica);

    escrever_saida(argv[2], &gramatica);

    return EXIT_SUCCESS;
}