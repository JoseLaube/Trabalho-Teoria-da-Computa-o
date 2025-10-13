// Para compilar:
// gcc tradutor.c -o tradutor
// Para executar:
// ./tradutor entrada_exemplo.in saida_exemplo.out

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// <estadoAtual> <simboloAtual> <novoSimbolo> <direcao> <novoEstado>
typedef struct {
    char* estadoAtual;
    char* simboloAtual;
    char* novoSimbolo;
    char direcao;
    char* novoEstado;
} Transicao;

// Maquina de Turing completa, lida de um arquivo
typedef struct {
    Transicao* transicoes;
    int numTransicoes;
    int capacidadeTransicoes;
    char tipo;  // 'S' para sipser, 'I' para duplamente_infinita
} MaquinaTuring;


MaquinaTuring* carregar_maquina(const char* nomeArquivo);
MaquinaTuring* traduzir_maquina(const MaquinaTuring* maquinaOriginal);
MaquinaTuring* traduzir_S_para_I(const MaquinaTuring* maquinaOriginal);
MaquinaTuring* traduzir_I_para_S(const MaquinaTuring* maquinaOriginal);
void adicionar_transicao(MaquinaTuring* maquina, const char* estA, const char* simA, const char* simN, char dir, const char* estN);
void salvar_maquina(const MaquinaTuring* maquina, const char* nomeArquivo);
void liberar_maquina(MaquinaTuring* maquina);



MaquinaTuring* carregar_maquina(const char* nomeArquivo) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "ERRO: Não foi possível abrir o arquivo '%s': %s\n", nomeArquivo, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Aloca a estrutura principal da máquina
    MaquinaTuring* maquina = (MaquinaTuring*) malloc(sizeof(MaquinaTuring));
    if (!maquina) {
        fprintf(stderr, "ERRO: Falha na alocação de memória para a máquina.\n");
        exit(EXIT_FAILURE);
    }

    maquina->numTransicoes = 0;
    maquina->capacidadeTransicoes = 10; // Capacidade inicial
    maquina->transicoes = (Transicao*) malloc(maquina->capacidadeTransicoes * sizeof(Transicao));
    if (!maquina->transicoes) {
        fprintf(stderr, "ERRO: Falha na alocação de memória para as transições.\n");
        exit(EXIT_FAILURE);
    }

    char linha[256]; // Buffer para ler cada linha do arquivo

    // 1. Ler a primeira linha para determinar o tipo da máquina
    if (fgets(linha, sizeof(linha), arquivo)) {
        if (linha[0] == ';' && (linha[1] == 'S' || linha[1] == 'I')) {
            maquina->tipo = linha[1];
        } else {
            fprintf(stderr, "ERRO: Primeira linha do arquivo inválida. Esperado ';S' ou ';I'.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "ERRO: Arquivo de entrada vazio ou corrompido.\n");
        exit(EXIT_FAILURE);
    }

    // 2. Ler as transições linha por linha
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Ignorar linhas vazias ou de comentário
        if (linha[0] == '\n' || linha[0] == ';') {
            continue;
        }

        // Checar se o array de transições está cheio e precisa ser realocado
        if (maquina->numTransicoes >= maquina->capacidadeTransicoes) {
            maquina->capacidadeTransicoes *= 2; // Dobrar a capacidade
            maquina->transicoes = (Transicao*) realloc(maquina->transicoes, maquina->capacidadeTransicoes * sizeof(Transicao));
            if (!maquina->transicoes) {
                fprintf(stderr, "ERRO: Falha na realocação de memória para as transições.\n");
                exit(EXIT_FAILURE);
            }
        }
        
        char* token;
        
        token = strtok(linha, " \t\n\r");
        if (!token) continue; // Linha mal formatada
        maquina->transicoes[maquina->numTransicoes].estadoAtual = strdup(token);

        token = strtok(NULL, " \t\n\r");
        if (!token) continue;
        maquina->transicoes[maquina->numTransicoes].simboloAtual = strdup(token);

        token = strtok(NULL, " \t\n\r");
        if (!token) continue;
        maquina->transicoes[maquina->numTransicoes].novoSimbolo = strdup(token);
        
        token = strtok(NULL, " \t\n\r");
        if (!token) continue;
        maquina->transicoes[maquina->numTransicoes].direcao = token[0];

        token = strtok(NULL, " \t\n\r");
        if (!token) continue;
        maquina->transicoes[maquina->numTransicoes].novoEstado = strdup(token);

        maquina->numTransicoes++;
    }

    fclose(arquivo);
    return maquina;
}

MaquinaTuring* traduzir_maquina(const MaquinaTuring* maquinaOriginal) {
    printf("-> Iniciando tradução do tipo '%c'...\n", maquinaOriginal->tipo);

    switch (maquinaOriginal->tipo) {
        case 'S':
            return traduzir_S_para_I(maquinaOriginal);
        case 'I':
            //return traduzir_I_para_S(maquinaOriginal);
        default:
            fprintf(stderr, "ERRO: Tipo de máquina desconhecido '%c'.\n", maquinaOriginal->tipo);
            return NULL;
    }
}

void adicionar_transicao(MaquinaTuring* maquina, const char* estA, const char* simA, const char* simN, char dir, const char* estN) {
    // Verifica se o array de transições precisa crescer
    if (maquina->numTransicoes >= maquina->capacidadeTransicoes) {
        maquina->capacidadeTransicoes *= 2;
        maquina->transicoes = (Transicao*) realloc(maquina->transicoes, maquina->capacidadeTransicoes * sizeof(Transicao));
    }
    
    // Aloca memória e copia os dados para a nova transição
    int i = maquina->numTransicoes;
    maquina->transicoes[i].estadoAtual  = strdup(estA);
    maquina->transicoes[i].simboloAtual = strdup(simA);
    maquina->transicoes[i].novoSimbolo  = strdup(simN);
    maquina->transicoes[i].direcao      = dir;
    maquina->transicoes[i].novoEstado   = strdup(estN);
    
    maquina->numTransicoes++;
}

void salvar_maquina(const MaquinaTuring* maquina, const char* nomeArquivo) {
    if (!maquina) {
        fprintf(stderr, "ERRO: Tentativa de salvar uma máquina nula.\n");
        return;
    }

    FILE* arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        fprintf(stderr, "ERRO: Não foi possível abrir o arquivo de saída '%s': %s\n", nomeArquivo, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Escrever a primeira linha com o tipo da máquina
    fprintf(arquivo, ";%c\n", maquina->tipo);

    // Fazer um loop e escrever cada transição
    for (int i = 0; i < maquina->numTransicoes; i++) {
        const Transicao* t = &maquina->transicoes[i]; // Ponteiro para a transição atual
        fprintf(arquivo, "%s %s %s %c %s\n",
                t->estadoAtual,
                t->simboloAtual,
                t->novoSimbolo,
                t->direcao,
                t->novoEstado);
    }

    fclose(arquivo);
}

void liberar_maquina(MaquinaTuring* maquina) {
    if (!maquina) return;

    for (int i = 0; i < maquina->numTransicoes; i++) {
        free(maquina->transicoes[i].estadoAtual);
        free(maquina->transicoes[i].simboloAtual);
        free(maquina->transicoes[i].novoSimbolo);
        free(maquina->transicoes[i].novoEstado);
    }
    
    free(maquina->transicoes);
    free(maquina);
}



//  Lógica usada: Para desenvolver uma MT Duplamente Infinita (MT_di) que simule uma MT Sipser, precisamos primeiramente na MT_di mover o cabeçote para a esquerda e marcar o inicio da fita com um simbolo especial (no caso o #). Apartir disso a MT_di reproduz as transições para direita e parado exatamente da mesma forma que a do Sipser, a unica diferença é a o movimento para a esquerda que pode ser que o mesmo leve para o simbolo #, nesse caso o cabeçote deve voltar para a primeira celula (simbolo 0 ou 1). E para fazer essa verificação e garantir a integridade da tradução adicionamos a transição ("#", "#", 'r') para cada estado que possua uma transição para esquerda 'l'.

MaquinaTuring* traduzir_S_para_I(const MaquinaTuring* maquinaOriginal) {
    MaquinaTuring* maquinaTraduzida = (MaquinaTuring*) malloc(sizeof(MaquinaTuring));
    maquinaTraduzida->tipo = 'I';
    maquinaTraduzida->numTransicoes = 0;
    maquinaTraduzida->capacidadeTransicoes = 20;
    maquinaTraduzida->transicoes = (Transicao*) malloc(maquinaTraduzida->capacidadeTransicoes * sizeof(Transicao));
    
    printf("-> Gerando sub-rotina de setup-shift para o alfabeto {0, 1} (versão final)...\n");

    // Parte 1: Move para esquerda e coloca a barreira #
    adicionar_transicao(maquinaTraduzida, "0", "*", "*", 'l', "marcar_#");
    adicionar_transicao(maquinaTraduzida, "marcar_#", "*", "#", 'r', "0_dir");

    // Estrutura para rastrear estados que já possuem regra de barreira
    int capacidadeBarreira = 10;
    int numEstadosBarreira = 0;
    char** estadosComBarreira = (char**) malloc(capacidadeBarreira * sizeof(char*));

    // Parte 2: Tratando as transições originais sem problemas (movimento para direita e Parado)
    printf("-> Traduzindo regras simples (movimento 'r', '*' e 'l')...\n");
    for (int i = 0; i < maquinaOriginal->numTransicoes; i++) {
        const Transicao* t = &maquinaOriginal->transicoes[i];

        // Traduz apenas as regras de movimento para a direita ou parado
        if (t->direcao == 'r' || t->direcao == '*') {
            char estadoAtualDir[100];
            char novoEstadoDir[100];

            // Adiciona o sufixo '_dir', a menos que seja um estado 'halt'
            if (strncmp(t->estadoAtual, "halt", 4) == 0) {
                strcpy(estadoAtualDir, t->estadoAtual);
            } else {
                sprintf(estadoAtualDir, "%s_dir", t->estadoAtual);
            }

            if (strncmp(t->novoEstado, "halt", 4) == 0) {
                strcpy(novoEstadoDir, t->novoEstado);
            } else {
                sprintf(novoEstadoDir, "%s_dir", t->novoEstado);
            }
            
            // Adiciona a transição traduzida à nova máquina
            adicionar_transicao(maquinaTraduzida,
                estadoAtualDir,
                t->simboloAtual,
                t->novoSimbolo,
                t->direcao,
                novoEstadoDir);
        }
        
        else if (t->direcao == 'l') {
            // A lógica aqui é identica a de cima, apenas para a direção 'l'
            char estadoAtualDir[100];
            char novoEstadoDir[100];

            if (strncmp(t->estadoAtual, "halt", 4) == 0) strcpy(estadoAtualDir, t->estadoAtual);
            else sprintf(estadoAtualDir, "%s_dir", t->estadoAtual);

            if (strncmp(t->novoEstado, "halt", 4) == 0) strcpy(novoEstadoDir, t->novoEstado);
            else sprintf(novoEstadoDir, "%s_dir", t->novoEstado);
        
             // Adiciona a transição 'l' normal (caso não seja na barreira)
            adicionar_transicao(maquinaTraduzida, estadoAtualDir, t->simboloAtual, t->novoSimbolo, 'l', novoEstadoDir);
            
            // Verifica se a regra da mola para 'novoEstadoDir' já foi criada
            int regraJaExiste = 0;
            for (int j = 0; j < numEstadosBarreira; j++) {
                if (strcmp(estadosComBarreira[j], novoEstadoDir) == 0) {
                    regraJaExiste = 1;
                    break;
                }
            }
            
            // Se a regra ainda não existe, crie ela e registre o estado
            if (!regraJaExiste) {
                // Adiciona a regra da mola: ao bater no '#', move para a direita 'r'
                adicionar_transicao(maquinaTraduzida, novoEstadoDir, "#", "#", 'r', novoEstadoDir);
                
                // Registra que já criamos a regra para este estado
                if (numEstadosBarreira >= capacidadeBarreira) {
                    capacidadeBarreira *= 2;
                    estadosComBarreira = (char**) realloc(estadosComBarreira, capacidadeBarreira * sizeof(char*));
                }
                estadosComBarreira[numEstadosBarreira++] = strdup(novoEstadoDir);
            }


        }

    }

    return maquinaTraduzida;
}


// Lógica que será usada:
MaquinaTuring* traduzir_I_para_S(const MaquinaTuring* maquinaOriginal) {
    MaquinaTuring* maquinaTraduzida = (MaquinaTuring*) malloc(sizeof(MaquinaTuring));
    maquinaTraduzida->tipo = 'I';
    maquinaTraduzida->numTransicoes = 0;
    maquinaTraduzida->capacidadeTransicoes = 20;
    maquinaTraduzida->transicoes = (Transicao*) malloc(maquinaTraduzida->capacidadeTransicoes * sizeof(Transicao));
    
    printf("-> Gerando sub-rotina de setup-shift para o alfabeto {0, 1} (versão final)...\n");

    // Parte 1: Início e busca pelo fim da palavra
    adicionar_transicao(maquinaTraduzida, "0", "_", "#", 'r', "0_dir");
    adicionar_transicao(maquinaTraduzida, "0", "0", "0", 'r', "setup_go_end");
    adicionar_transicao(maquinaTraduzida, "0", "1", "1", 'r', "setup_go_end");
    adicionar_transicao(maquinaTraduzida, "setup_go_end", "*", "*", 'r', "setup_go_end");
    adicionar_transicao(maquinaTraduzida, "setup_go_end", "_", "_", 'l', "setup_copy");

    // Parte 2: O loop de cópia com estado de retorno
    adicionar_transicao(maquinaTraduzida, "setup_copy", "0", "_", 'r', "setup_write_0");
    adicionar_transicao(maquinaTraduzida, "setup_copy", "1", "_", 'r', "setup_write_1");
    adicionar_transicao(maquinaTraduzida, "setup_write_0", "*", "0", 'l', "setup_retorno");
    adicionar_transicao(maquinaTraduzida, "setup_write_1", "*", "1", 'l', "setup_retorno");
    adicionar_transicao(maquinaTraduzida, "setup_retorno", "*", "*", 'l', "setup_copy");
    
    // Parte 3: Adicionando a barreira #
    adicionar_transicao(maquinaTraduzida, "setup_copy", "_", "*", 'r', "position_head");
    adicionar_transicao(maquinaTraduzida, "position_head", "_", "#", 'r', "0_dir");
}





int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo_de_entrada> <arquivo_de_saida>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* arquivoEntrada = argv[1];
    const char* arquivoSaida = argv[2];

    printf("Tradutor de Máquina de Turing\n");
    printf("----------------------------------\n");
    printf("Arquivo de entrada: %s\n", arquivoEntrada);
    printf("Arquivo de saída:   %s\n", arquivoSaida);
    printf("----------------------------------\n");

    MaquinaTuring* maquinaOriginal = carregar_maquina(arquivoEntrada);
    printf("-> Máquina original (tipo '%c') carregada com %d transições.\n", maquinaOriginal->tipo, maquinaOriginal->numTransicoes);


    MaquinaTuring* maquinaTraduzida = traduzir_maquina(maquinaOriginal);

    // Se a tradução foi bem-sucedida, salvar e liberar a máquina traduzida
    if (maquinaTraduzida) {
        salvar_maquina(maquinaTraduzida, arquivoSaida);
        printf("-> Máquina traduzida para o tipo '%c' e salva com sucesso.\n", maquinaTraduzida->tipo);

        liberar_maquina(maquinaTraduzida);
        printf("-> Memória da máquina traduzida liberada.\n");
    } else {
        printf("-> A tradução não pôde ser concluída.\n");
    }


    liberar_maquina(maquinaOriginal);
    printf("-> Memória da máquina original liberada com sucesso.\n");

    printf("\nEstrutura do programa concluída. As funções de lógica serão implementadas nos próximos passos.\n");

    return EXIT_SUCCESS;
}