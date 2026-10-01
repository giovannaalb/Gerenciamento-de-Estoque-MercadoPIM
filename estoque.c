#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    int id;
    char nome[100];
    char codigoBarras[100];
    float preco;
    char marca[100];
    char categoria[100];
    char validade[100];
    int quantidade;
    int quantidadeMinima;
} Produto;

Produto produtos[500];
int totalProdutos = 0;

int estoqueMensal[500][12];

int mesAtual(void) {
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);
    return data->tm_mon;
}

void lerTexto(const char *mensagem, char *destino, int tamanho) {
    do {
        printf("%s", mensagem);
        fgets(destino, tamanho, stdin);
        destino[strcspn(destino, "\n")] = '\0';

        if (strlen(destino) == 0)
            printf("Este campo nao pode ficar vazio. Tente novamente.\n");
    } while (strlen(destino) == 0);
}

float lerPreco(const char *mensagem) {
    char buffer[100];
    float preco;
    char *endptr;

    while (1) {
        printf("%s", mensagem);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        // Substitui virgula por ponto para aceitar formato brasileiro (ex: 15,50)
        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == ',') {
                buffer[i] = '.';
            }
        }

        // Ignora espacos em branco no inicio
        char *ptr = buffer;
        while (isspace((unsigned char)*ptr)) ptr++;

        // Valida entrada nula/vazia
        if (*ptr == '\0') {
            printf("O preco nao pode ficar vazio. Tente novamente.\n");
            continue;
        }

        preco = strtof(ptr, &endptr);

        // Ignora espacos no final
        while (isspace((unsigned char)*endptr)) endptr++;

        // Valida se ha caracteres aleatorios apos o numero
        if (endptr == ptr || *endptr != '\0') {
            printf("Entrada invalida! Digite apenas um numero valido (ex: 15.50).\n");
            continue;
        }

        if (preco <= 0) {
            printf("O preco deve ser maior que zero. Tente novamente.\n");
            continue;
        }

        return preco;
    }
}

int lerInteiro(const char *mensagem, int minValor) {
    char buffer[100];
    int valor;
    char *endptr;

    while (1) {
        printf("%s", mensagem);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            continue;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        char *ptr = buffer;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') {
            printf("Este campo nao pode ficar vazio. Tente novamente.\n");
            continue;
        }

        valor = (int)strtol(ptr, &endptr, 10);

        while (isspace((unsigned char)*endptr)) endptr++;

        if (endptr == ptr || *endptr != '\0') {
            printf("Entrada invalida! Digite apenas um numero inteiro valido.\n");
            continue;
        }

        if (valor < minValor) {
            printf("O valor deve ser igual ou maior que %d. Tente novamente.\n", minValor);
            continue;
        }

        return valor;
    }
}

int dataValida(const char *data) {
    if (strlen(data) != 10) return 0;
    if (data[2] != '/' || data[5] != '/') return 0;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)data[i])) return 0;
    }

    int dia = (data[0] - '0') * 10 + (data[1] - '0');
    int mes = (data[3] - '0') * 10 + (data[4] - '0');

    if (dia < 1 || dia > 31) return 0;
    if (mes < 1 || mes > 12) return 0;

    return 1;
}

int textosIguais(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (tolower(*a) != tolower(*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

void cadastrarProduto(void) {
    totalProdutos = totalProdutos + 1;
    int i = totalProdutos - 1;

    produtos[i].id = totalProdutos;

    lerTexto("Nome do produto: ", produtos[i].nome, 100);
    lerTexto("Codigo de barras: ", produtos[i].codigoBarras, 100);
    produtos[i].preco = lerPreco("Preco de venda: ");
    lerTexto("Marca: ", produtos[i].marca, 100);
    lerTexto("Categoria: ", produtos[i].categoria, 100);

    do {
        lerTexto("Data de validade (dd/mm/aaaa): ", produtos[i].validade, 100);
        if (!dataValida(produtos[i].validade))
            printf("Data invalida! Use o formato dd/mm/aaaa (ex: 25/12/2026).\n");
    } while (!dataValida(produtos[i].validade));

    produtos[i].quantidade = lerInteiro("Quantidade em estoque: ", 0);
    produtos[i].quantidadeMinima = lerInteiro("Quantidade minima: ", 0);

    estoqueMensal[i][mesAtual()] = produtos[i].quantidade;

    printf("Produto cadastrado com sucesso!\n");
}

void procurarProdutoPorNome(void) {
    char nomeBusca[100];
    int encontrado = 0;

    lerTexto("Nome do produto: ", nomeBusca, 100);

    for (int i = 0; i < totalProdutos; i++) {
        if (textosIguais(produtos[i].nome, nomeBusca)) {
            printf("\nID: %d\n", produtos[i].id);
            printf("Nome: %s\n", produtos[i].nome);
            printf("Codigo de barras: %s\n", produtos[i].codigoBarras);
            printf("Preco: R$ %.2f\n", produtos[i].preco);
            printf("Marca: %s\n", produtos[i].marca);
            printf("Categoria: %s\n", produtos[i].categoria);
            printf("Validade: %s\n", produtos[i].validade);
            printf("Quantidade: %d\n", produtos[i].quantidade);
            printf("Quantidade minima: %d\n\n", produtos[i].quantidadeMinima);
            encontrado = 1;
        }
    }

    if (encontrado == 0)
        printf("Produto nao encontrado.\n");
}

void filtrarPorMarca(void) {
    char marcaBusca[100];
    int encontrado = 0;

    lerTexto("Marca: ", marcaBusca, 100);

    for (int i = 0; i < totalProdutos; i++) {
        if (textosIguais(produtos[i].marca, marcaBusca)) {
            printf("%d - %s (Qtd: %d)\n", produtos[i].id, produtos[i].nome, produtos[i].quantidade);
            encontrado = 1;
        }
    }

    if (encontrado == 0)
        printf("Nenhum produto encontrado para essa marca.\n");
}

void listarProdutos(void) {
    if (totalProdutos == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    for (int i = 0; i < totalProdutos; i++) {
        printf("%d - %s | Marca: %s | Qtd: %d\n",
               produtos[i].id, produtos[i].nome, produtos[i].marca, produtos[i].quantidade);
    }
}

void registrarSaida(void) {
    char codigo[100];
    int quantidadeSaida;
    int indice = -1;

    lerTexto("Codigo de barras: ", codigo, 100);

    for (int i = 0; i < totalProdutos; i++) {
        if (strcmp(produtos[i].codigoBarras, codigo) == 0) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        printf("Produto nao encontrado.\n");
        return;
    }

    printf("Estoque atual: %d\n", produtos[indice].quantidade);
    quantidadeSaida = lerInteiro("Quantidade a retirar: ", 1);

    if (quantidadeSaida > produtos[indice].quantidade) {
        printf("Quantidade invalida (maior que o estoque em loja).\n");
        return;
    }

    produtos[indice].quantidade = produtos[indice].quantidade - quantidadeSaida;
    estoqueMensal[indice][mesAtual()] = produtos[indice].quantidade;
    printf("Saida registrada. Novo estoque: %d\n", produtos[indice].quantidade);
}

void exibirEstoqueMensal(void) {
    char *meses[12] = {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
                        "Jul", "Ago", "Set", "Out", "Nov", "Dez"};

    if (totalProdutos == 0) {
        printf("Nenhum produto cadastrado.\n");
        return;
    }

    printf("\n%-20s", "Produto");
    for (int mes = 0; mes < 12; mes++)
        printf("%5s", meses[mes]);
    printf("\n");

    for (int i = 0; i < totalProdutos; i++) {
        printf("%-20s", produtos[i].nome);
        for (int mes = 0; mes < 12; mes++)
            printf("%5d", estoqueMensal[i][mes]);
        printf("\n");
    }
}

void salvarDados(void) {
    FILE *arquivo = fopen("produtos.txt", "w");

    for (int i = 0; i < totalProdutos; i++) {
        fprintf(arquivo, "%d;%s;%s;%.2f;%s;%s;%s;%d;%d\n",
                produtos[i].id, produtos[i].nome, produtos[i].codigoBarras,
                produtos[i].preco, produtos[i].marca, produtos[i].categoria,
                produtos[i].validade, produtos[i].quantidade, produtos[i].quantidadeMinima);
    }
    fclose(arquivo);
}

void carregarDados(void) {
    FILE *arquivo = fopen("produtos.txt", "r");
    if (arquivo == NULL) return;

    while (fscanf(arquivo, "%d;%99[^;];%99[^;];%f;%99[^;];%99[^;];%99[^;];%d;%d\n",
                  &produtos[totalProdutos].id,
                  produtos[totalProdutos].nome,
                  produtos[totalProdutos].codigoBarras,
                  &produtos[totalProdutos].preco,
                  produtos[totalProdutos].marca,
                  produtos[totalProdutos].categoria,
                  produtos[totalProdutos].validade,
                  &produtos[totalProdutos].quantidade,
                  &produtos[totalProdutos].quantidadeMinima) == 9) {
        totalProdutos = totalProdutos + 1;
    }
    fclose(arquivo);
}

void exibirMenu(void) {
    printf("\n1 - Cadastrar produto\n");
    printf("2 - Procurar produto pelo nome\n");
    printf("3 - Filtrar produtos por marca\n");
    printf("4 - Listar todos os produtos\n");
    printf("5 - Registrar saida de estoque\n");
    printf("6 - Exibir estoque mensal\n");
    printf("0 - Sair\n");
}

int main(void) {
    int opcao;

    carregarDados();

    do {
        exibirMenu();
        opcao = lerInteiro("Escolha uma opcao: ", 0);

        switch (opcao) {
            case 1: cadastrarProduto(); break;
            case 2: procurarProdutoPorNome(); break;
            case 3: filtrarPorMarca(); break;
            case 4: listarProdutos(); break;
            case 5: registrarSaida(); break;
            case 6: exibirEstoqueMensal(); break;
            case 0:
                salvarDados();
                printf("Dados salvos. Encerrando...\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    return 0;
}
