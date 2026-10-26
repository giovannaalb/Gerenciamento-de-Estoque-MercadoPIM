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
    char lote[100];
    char validade[100];
    int quantidade;
    int quantidadeMinima;
} Produto;

Produto *produtos = NULL;
int (*estoqueMensal)[12] = NULL;
int totalProdutos = 0;
int capacidade = 0;

int garantirCapacidade(void) {
    if (totalProdutos < capacidade) return 1;

    int novaCapacidade = capacidade == 0 ? 16 : capacidade * 2;

    Produto *novosProdutos = realloc(produtos, novaCapacidade * sizeof(Produto));
    if (novosProdutos == NULL) return 0;
    produtos = novosProdutos;

    int (*novoEstoque)[12] = realloc(estoqueMensal, novaCapacidade * sizeof(*estoqueMensal));
    if (novoEstoque == NULL) return 0;
    estoqueMensal = novoEstoque;

    memset(estoqueMensal + capacidade, 0, (novaCapacidade - capacidade) * sizeof(*estoqueMensal));

    capacidade = novaCapacidade;
    return 1;
}

void limparTela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar(void) {
    printf("\nPressione Enter para voltar ao menu...");
    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);
}

int mesAtual(void) {
    time_t agora = time(NULL);
    struct tm *data = localtime(&agora);
    return data->tm_mon;
}

void lerTexto(const char *mensagem, char *destino, int tamanho) {
    do {
        printf("%s", mensagem);
        if (fgets(destino, tamanho, stdin) == NULL) {
            destino[0] = '\0';
            continue;
        }
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

        for (int i = 0; buffer[i] != '\0'; i++) {
            if (buffer[i] == ',') {
                buffer[i] = '.';
            }
        }

        char *ptr = buffer;
        while (isspace((unsigned char)*ptr)) ptr++;

        if (*ptr == '\0') {
            printf("O preco nao pode ficar vazio. Tente novamente.\n");
            continue;
        }

        preco = strtof(ptr, &endptr);

        while (isspace((unsigned char)*endptr)) endptr++;

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
    int ano = (data[6] - '0') * 1000 + (data[7] - '0') * 100 + (data[8] - '0') * 10 + (data[9] - '0');

    if (ano < 1) return 0;
    if (mes < 1 || mes > 12) return 0;

    int diasNoMes[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((ano % 4 == 0 && ano % 100 != 0) || ano % 400 == 0)
        diasNoMes[1] = 29;

    if (dia < 1 || dia > diasNoMes[mes - 1]) return 0;

    return 1;
}

int dataParaNumero(const char *data) {
    int dia = (data[0] - '0') * 10 + (data[1] - '0');
    int mes = (data[3] - '0') * 10 + (data[4] - '0');
    int ano = (data[6] - '0') * 1000 + (data[7] - '0') * 100 + (data[8] - '0') * 10 + (data[9] - '0');
    return ano * 10000 + mes * 100 + dia;
}

int textosIguais(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

int existeNomeELote(const char *nome, const char *lote) {
    for (int i = 0; i < totalProdutos; i++) {
        if (textosIguais(produtos[i].nome, nome) && textosIguais(produtos[i].lote, lote))
            return 1;
    }
    return 0;
}

int indiceConflitoIdentidade(const char *nome, const char *codigo) {
    for (int i = 0; i < totalProdutos; i++) {
        int mesmoNome = textosIguais(produtos[i].nome, nome);
        int mesmoCodigo = strcmp(produtos[i].codigoBarras, codigo) == 0;
        if (mesmoNome != mesmoCodigo)
            return i;
    }
    return -1;
}

int existeProduto(const char *nome) {
    for (int i = 0; i < totalProdutos; i++) {
        if (textosIguais(produtos[i].nome, nome))
            return 1;
    }
    return 0;
}

void cadastrarProduto(void) {
    while (1) {
        limparTela();
        printf("=========================================\n");
        printf("           CADASTRO DE PRODUTO           \n");
        printf("=========================================\n\n");

        Produto p;
        int conflito;

        if (!garantirCapacidade()) {
            printf("Memoria insuficiente para cadastrar novos produtos.\n");
            pausar();
            break;
        }

        do {
            lerTexto("Nome do produto: ", p.nome, 100);
            lerTexto("Codigo de barras: ", p.codigoBarras, 100);

            conflito = indiceConflitoIdentidade(p.nome, p.codigoBarras);
            if (conflito != -1) {
                printf("Conflito: o produto \"%s\" ja esta cadastrado com o codigo de barras %s.\n",
                       produtos[conflito].nome, produtos[conflito].codigoBarras);
                printf("O mesmo nome e codigo de barras devem identificar o mesmo produto. Informe novamente.\n");
            }
        } while (conflito != -1);

        int produtoExistente = existeProduto(p.nome);
        if (produtoExistente)
            printf("Produto ja cadastrado. Sera registrado um novo lote.\n");

        p.preco = lerPreco("Preco de venda: ");
        lerTexto("Marca: ", p.marca, 100);
        lerTexto("Categoria: ", p.categoria, 100);

        do {
            lerTexto("Lote: ", p.lote, 100);
            if (strchr(p.lote, ';') != NULL) {
                printf("O lote nao pode conter ponto e virgula.\n");
                p.lote[0] = '\0';
                continue;
            }
            if (existeNomeELote(p.nome, p.lote))
                printf("Produto ja cadastrado neste lote. Informe um lote diferente.\n");
        } while (strlen(p.lote) == 0 || existeNomeELote(p.nome, p.lote));

        do {
            lerTexto("Data de validade (dd/mm/aaaa): ", p.validade, 100);
            if (!dataValida(p.validade))
                printf("Data invalida! Use o formato dd/mm/aaaa (ex: 25/12/2026).\n");
        } while (!dataValida(p.validade));

        p.quantidade = lerInteiro("Quantidade em estoque: ", 0);
        p.quantidadeMinima = lerInteiro("Quantidade minima: ", 0);

        p.id = totalProdutos + 1;
        produtos[totalProdutos] = p;
        estoqueMensal[totalProdutos][mesAtual()] = p.quantidade;
        totalProdutos = totalProdutos + 1;

        printf("\nProduto cadastrado com sucesso!\n");

        char resposta[10];
        lerTexto("\nDeseja cadastrar mais produtos? (s/n): ", resposta, sizeof(resposta));
        if (resposta[0] == 's' || resposta[0] == 'S') {
            continue;
        } else {
            break;
        }
    }
}

void procurarProdutoPorNome(void) {
    while (1) {
        limparTela();
        printf("=========================================\n");
        printf("       BUSCA DE PRODUTO POR NOME         \n");
        printf("=========================================\n\n");

        if (totalProdutos == 0) {
            printf("Nenhum produto cadastrado.\n");
            pausar();
            break;
        }

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
                printf("Lote: %s\n", produtos[i].lote);
                printf("Validade: %s\n", produtos[i].validade);
                printf("Quantidade: %d\n", produtos[i].quantidade);
                printf("Quantidade minima: %d\n\n", produtos[i].quantidadeMinima);
                encontrado = 1;
            }
        }

        if (encontrado == 0)
            printf("\nProduto nao encontrado.\n");

        char resposta[10];
        lerTexto("\nDeseja realizar outra busca por nome? (s/n): ", resposta, sizeof(resposta));
        if (resposta[0] == 's' || resposta[0] == 'S') {
            continue;
        } else {
            break;
        }
    }
}

void filtrarPorMarca(void) {
    while (1) {
        limparTela();
        printf("=========================================\n");
        printf("          FILTRAR POR MARCA              \n");
        printf("=========================================\n\n");

        if (totalProdutos == 0) {
            printf("Nenhum produto cadastrado.\n");
            pausar();
            break;
        }

        char marcaBusca[100];
        int encontrado = 0;

        lerTexto("Marca: ", marcaBusca, 100);

        printf("\n");
        for (int i = 0; i < totalProdutos; i++) {
            if (textosIguais(produtos[i].marca, marcaBusca)) {
                printf("%d - %s | Lote: %s (Qtd: %d)\n", produtos[i].id, produtos[i].nome,
                       produtos[i].lote, produtos[i].quantidade);
                encontrado = 1;
            }
        }

        if (encontrado == 0)
            printf("Nenhum produto encontrado para essa marca.\n");

        char resposta[10];
        lerTexto("\nDeseja filtrar por outra marca? (s/n): ", resposta, sizeof(resposta));
        if (resposta[0] == 's' || resposta[0] == 'S') {
            continue;
        } else {
            break;
        }
    }
}

void listarProdutos(void) {
    limparTela();
    printf("=========================================\n");
    printf("          LISTA DE PRODUTOS              \n");
    printf("=========================================\n\n");

    if (totalProdutos == 0) {
        printf("Nenhum produto cadastrado.\n");
        pausar();
        return;
    }

    for (int i = 0; i < totalProdutos; i++) {
        printf("%d - %s | Marca: %s | Lote: %s | Validade: %s | Qtd: %d\n",
               produtos[i].id, produtos[i].nome, produtos[i].marca,
               produtos[i].lote, produtos[i].validade, produtos[i].quantidade);
    }

    pausar();
}

int indiceLotePrioritario(const char *codigo) {
    int indice = -1;
    for (int i = 0; i < totalProdutos; i++) {
        if (strcmp(produtos[i].codigoBarras, codigo) != 0) continue;
        if (produtos[i].quantidade <= 0) continue;
        if (indice == -1 ||
            dataParaNumero(produtos[i].validade) < dataParaNumero(produtos[indice].validade))
            indice = i;
    }
    return indice;
}

void registrarSaida(void) {
    while (1) {
        limparTela();
        printf("=========================================\n");
        printf("         REGISTRAR SAIDA DE ESTOQUE      \n");
        printf("=========================================\n\n");

        if (totalProdutos == 0) {
            printf("Nenhum produto cadastrado.\n");
            pausar();
            break;
        }

        char codigo[100];
        int quantidadeSaida;
        int encontrado = 0;
        int totalDisponivel = 0;
        int quantidadeMinima = 0;
        char nomeProduto[100] = "";

        lerTexto("Codigo de barras: ", codigo, 100);

        for (int i = 0; i < totalProdutos; i++) {
            if (strcmp(produtos[i].codigoBarras, codigo) == 0) {
                encontrado = 1;
                totalDisponivel = totalDisponivel + produtos[i].quantidade;
                if (nomeProduto[0] == '\0') {
                    strcpy(nomeProduto, produtos[i].nome);
                    quantidadeMinima = produtos[i].quantidadeMinima;
                } else if (quantidadeMinima == 0 && produtos[i].quantidadeMinima > 0) {
                    quantidadeMinima = produtos[i].quantidadeMinima;
                }
            }
        }

        if (!encontrado) {
            printf("\nProduto nao encontrado.\n");
            char resposta[10];
            lerTexto("\nDeseja tentar registrar outra saida? (s/n): ", resposta, sizeof(resposta));
            if (resposta[0] == 's' || resposta[0] == 'S') {
                continue;
            } else {
                break;
            }
        }

        if (totalDisponivel == 0) {
            printf("\nEste produto esta sem estoque disponivel (saldo 0).\n");
            char resposta[10];
            lerTexto("\nDeseja tentar registrar outra saida? (s/n): ", resposta, sizeof(resposta));
            if (resposta[0] == 's' || resposta[0] == 'S') {
                continue;
            } else {
                break;
            }
        }

        printf("\nEstoque atual (todos os lotes): %d\n", totalDisponivel);
        quantidadeSaida = lerInteiro("Quantidade a retirar: ", 1);

        if (quantidadeSaida > totalDisponivel) {
            printf("\nQuantidade invalida (maior que o estoque em loja).\n");
            char resposta[10];
            lerTexto("\nDeseja tentar registrar outra saida? (s/n): ", resposta, sizeof(resposta));
            if (resposta[0] == 's' || resposta[0] == 'S') {
                continue;
            } else {
                break;
            }
        }

        int restante = quantidadeSaida;
        printf("\n");
        while (restante > 0) {
            int indice = indiceLotePrioritario(codigo);
            int retirada = restante < produtos[indice].quantidade ? restante : produtos[indice].quantidade;

            produtos[indice].quantidade = produtos[indice].quantidade - retirada;
            estoqueMensal[indice][mesAtual()] = produtos[indice].quantidade;
            restante = restante - retirada;

            printf("Retirado %d un. do lote %s (validade %s). Saldo do lote: %d\n",
                   retirada, produtos[indice].lote, produtos[indice].validade, produtos[indice].quantidade);
        }

        printf("\nSaida registrada com sucesso!\n");

        int saldoRestante = totalDisponivel - quantidadeSaida;
        printf("Saldo restante em estoque: %d un.\n", saldoRestante);

        if (saldoRestante < quantidadeMinima) {
            printf("\n=========================================\n");
            printf(" [ALERTA] ESTOQUE BAIXO!\n");
            printf(" Necessidade de reposicao para o produto \"%s\".\n", nomeProduto);
            printf(" Saldo atual: %d | Quantidade minima: %d\n", saldoRestante, quantidadeMinima);
            printf("=========================================\n");
        }

        char resposta[10];
        lerTexto("\nDeseja registrar outra saida de estoque? (s/n): ", resposta, sizeof(resposta));
        if (resposta[0] == 's' || resposta[0] == 'S') {
            continue;
        } else {
            break;
        }
    }
}

void exibirEstoqueMensal(void) {
    limparTela();
    printf("===========================================================================================================================\n");
    printf("                                                 MATRIZ DE ESTOQUE MENSAL                                                  \n");
    printf("===========================================================================================================================\n\n");

    if (totalProdutos == 0) {
        printf("Nenhum produto cadastrado no momento.\n");
        pausar();
        return;
    }

    const char *mesesNomes[12] = {
        "Janeiro", "Fevereiro", "Marco", "Abril", "Maio", "Junho",
        "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro"
    };
    const char *mesesSiglas[12] = {
        "JAN", "FEV", "MAR", "ABR", "MAI", "JUN",
        "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"
    };
    int mesAtualIdx = mesAtual();

    printf(" Referencia: Ano Vigente | Mes Atual: %s (%s) | Total de Lotes Cadastrados: %d\n\n",
           mesesNomes[mesAtualIdx], mesesSiglas[mesAtualIdx], totalProdutos);

    printf("+-----+----------------------+------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n");
    printf("| ID  | PRODUTO              | LOTE       | JAN | FEV | MAR | ABR | MAI | JUN | JUL | AGO | SET | OUT | NOV | DEZ | STATUS    |\n");
    printf("+-----+----------------------+------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n");

    int totalEstoqueBaixo = 0;

    for (int i = 0; i < totalProdutos; i++) {
        int estoqueBaixo = (produtos[i].quantidade <= produtos[i].quantidadeMinima);
        if (estoqueBaixo) {
            totalEstoqueBaixo++;
        }

        printf("| %3d | %-20.20s | %-10.10s |", produtos[i].id, produtos[i].nome, produtos[i].lote);
        for (int mes = 0; mes < 12; mes++) {
            printf("%4d |", estoqueMensal[i][mes]);
        }
        if (estoqueBaixo) {
            printf(" [!] BAIXO |\n");
        } else {
            printf(" OK        |\n");
        }
    }

    printf("+-----+----------------------+------------+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----------+\n");
    printf(" Legenda: As colunas indicam o saldo de cada mes. STATUS: [!] BAIXO = estoque atual <= minimo | OK = nivel adequado.\n");
    printf("===========================================================================================================================\n");

    if (totalEstoqueBaixo > 0) {
        printf("\n [ALERTA] Foi(ram) identificado(s) %d produto(s) com estoque baixo ou no limite minimo!\n", totalEstoqueBaixo);
    } else {
        printf("\n [INFO] Todos os produtos estao com estoque em nivel adequado.\n");
    }

    char resposta[10];
    lerTexto("\nDeseja imprimir lista de produtos com estoque baixo para compra? (s/n): ", resposta, sizeof(resposta));

    if (resposta[0] == 's' || resposta[0] == 'S') {
        if (totalEstoqueBaixo == 0) {
            printf("\nNao ha produtos com estoque baixo para compra no momento.\n");
        } else {
            printf("\n===================================================================================================\n");
            printf("                        LISTA DE PRODUTOS PARA COMPRA (ESTOQUE BAIXO)                             \n");
            printf("===================================================================================================\n");
            printf("+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");
            printf("| ID  | PRODUTO              | MARCA      | LOTE       | ESTOQUE | MINIMO | A COMPRAR | PRECO UN. |\n");
            printf("+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");

            float totalEstimado = 0.0f;
            int totalItensComprar = 0;

            for (int i = 0; i < totalProdutos; i++) {
                if (produtos[i].quantidade <= produtos[i].quantidadeMinima) {
                    int aComprar = produtos[i].quantidadeMinima - produtos[i].quantidade;
                    if (aComprar <= 0) {
                        aComprar = 1;
                    }
                    float subtotal = aComprar * produtos[i].preco;
                    totalEstimado += subtotal;
                    totalItensComprar += aComprar;

                    printf("| %3d | %-20.20s | %-10.10s | %-10.10s | %7d | %6d | %9d | R$ %6.2f |\n",
                           produtos[i].id, produtos[i].nome, produtos[i].marca, produtos[i].lote,
                           produtos[i].quantidade, produtos[i].quantidadeMinima, aComprar, produtos[i].preco);
                }
            }

            printf("+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");
            printf(" Total de unidades sugeridas para compra: %d un.\n", totalItensComprar);
            printf(" Valor estimado total para aquisicao: R$ %.2f\n", totalEstimado);
            printf("===================================================================================================\n");

            FILE *arquivoCompras = fopen("lista_compras.txt", "w");
            if (arquivoCompras != NULL) {
                fprintf(arquivoCompras, "===================================================================================================\n");
                fprintf(arquivoCompras, "                        LISTA DE PRODUTOS PARA COMPRA (ESTOQUE BAIXO)                             \n");
                fprintf(arquivoCompras, "===================================================================================================\n");
                fprintf(arquivoCompras, "+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");
                fprintf(arquivoCompras, "| ID  | PRODUTO              | MARCA      | LOTE       | ESTOQUE | MINIMO | A COMPRAR | PRECO UN. |\n");
                fprintf(arquivoCompras, "+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");
                for (int i = 0; i < totalProdutos; i++) {
                    if (produtos[i].quantidade <= produtos[i].quantidadeMinima) {
                        int aComprar = produtos[i].quantidadeMinima - produtos[i].quantidade;
                        if (aComprar <= 0) {
                            aComprar = 1;
                        }
                        fprintf(arquivoCompras, "| %3d | %-20.20s | %-10.10s | %-10.10s | %7d | %6d | %9d | R$ %6.2f |\n",
                                produtos[i].id, produtos[i].nome, produtos[i].marca, produtos[i].lote,
                                produtos[i].quantidade, produtos[i].quantidadeMinima, aComprar, produtos[i].preco);
                    }
                }
                fprintf(arquivoCompras, "+-----+----------------------+------------+------------+---------+--------+-----------+-----------+\n");
                fprintf(arquivoCompras, " Total de unidades sugeridas para compra: %d un.\n", totalItensComprar);
                fprintf(arquivoCompras, " Valor estimado total para aquisicao: R$ %.2f\n", totalEstimado);
                fprintf(arquivoCompras, "===================================================================================================\n");
                fclose(arquivoCompras);
                printf("\n[OK] Relatorio de compras tambem salvo com sucesso no arquivo 'lista_compras.txt'.\n");
            }
        }
    }

    pausar();
}

void salvarDados(void) {
    FILE *arquivo = fopen("produtos.txt", "w");
    if (arquivo == NULL) {
        printf("Erro ao salvar os dados.\n");
        return;
    }

    for (int i = 0; i < totalProdutos; i++) {
        fprintf(arquivo, "%d;%s;%s;%.2f;%s;%s;%s;%s;%d;%d\n",
                produtos[i].id, produtos[i].nome, produtos[i].codigoBarras,
                produtos[i].preco, produtos[i].marca, produtos[i].categoria,
                produtos[i].lote, produtos[i].validade,
                produtos[i].quantidade, produtos[i].quantidadeMinima);
    }
    fclose(arquivo);
}

void carregarDados(void) {
    FILE *arquivo = fopen("produtos.txt", "r");
    if (arquivo == NULL) return;

    Produto p;

    while (fscanf(arquivo, "%d;%99[^;];%99[^;];%f;%99[^;];%99[^;];%99[^;];%99[^;];%d;%d\n",
                  &p.id, p.nome, p.codigoBarras, &p.preco, p.marca, p.categoria,
                  p.lote, p.validade, &p.quantidade, &p.quantidadeMinima) == 10) {
        if (!garantirCapacidade()) {
            printf("Memoria insuficiente para carregar todos os produtos.\n");
            break;
        }
        produtos[totalProdutos] = p;
        estoqueMensal[totalProdutos][mesAtual()] = p.quantidade;
        totalProdutos = totalProdutos + 1;
    }
    fclose(arquivo);
}

void exibirMenu(void) {
    printf("=========================================\n");
    printf("       GERENCIAMENTO DE ESTOQUE          \n");
    printf("=========================================\n");
    printf(" 1 - Cadastrar produto\n");
    printf(" 2 - Procurar produto pelo nome\n");
    printf(" 3 - Filtrar produtos por marca\n");
    printf(" 4 - Listar todos os produtos\n");
    printf(" 5 - Registrar saida de estoque\n");
    printf(" 6 - Exibir estoque mensal\n");
    printf(" 0 - Sair\n");
    printf("=========================================\n");
}

int main(void) {
    int opcao;

    carregarDados();

    do {
        limparTela();
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
                limparTela();
                salvarDados();
                printf("Dados salvos com sucesso. Encerrando...\n");
                break;
            default:
                printf("\nOpcao invalida.\n");
                pausar();
        }
    } while (opcao != 0);

    free(produtos);
    free(estoqueMensal);

    return 0;
}
