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
            return traduzir_I_para_S(maquinaOriginal);
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


MaquinaTuring* traduzir_I_para_S(const MaquinaTuring* maquinaOriginal) {
    // --- INÍCIO: Parte de inicialização e coleta de estados (do seu código original) ---

    // Renomeia o estado "0" original para "inicio" nas transições da máquina original antes de processá-las
    // Isso evita conflitos com o novo estado "0" de pré-processamento na MT de Sipser.
    // **IMPORTANTE**: Fazendo isso aqui, as transições da máquina original são MODIFICADAS.
    // Para uma abordagem mais robusta, deveríamos copiar as transições e renomear na cópia.
    // Por simplicidade e dada a natureza do exercício, vamos manter assim, mas é um ponto a notar.
    // Criar cópia das transições
    Transicao* transicoesCopiadas = (Transicao*) malloc(maquinaOriginal->numTransicoes * sizeof(Transicao));
    for(int i=0;i<maquinaOriginal->numTransicoes;i++){
        transicoesCopiadas[i].estadoAtual = strcmp(maquinaOriginal->transicoes[i].estadoAtual,"0")==0 ? strdup("inicio") : strdup(maquinaOriginal->transicoes[i].estadoAtual);
        transicoesCopiadas[i].simboloAtual = strdup(maquinaOriginal->transicoes[i].simboloAtual);
        transicoesCopiadas[i].novoSimbolo  = strdup(maquinaOriginal->transicoes[i].novoSimbolo);
        transicoesCopiadas[i].direcao      = maquinaOriginal->transicoes[i].direcao;
        transicoesCopiadas[i].novoEstado   = strcmp(maquinaOriginal->transicoes[i].novoEstado,"0")==0 ? strdup("inicio") : strdup(maquinaOriginal->transicoes[i].novoEstado);
    }

    
    for (int i = 0; i < maquinaOriginal->numTransicoes; ++i) {
        Transicao* t = &maquinaOriginal->transicoes[i];
        if (strcmp(t->estadoAtual, "0") == 0) {
            free(t->estadoAtual);
            t->estadoAtual = strdup("inicio");
        }
        if (strcmp(t->novoEstado, "0") == 0) {
            free(t->novoEstado);
            t->novoEstado = strdup("inicio");
        }       
    }

    MaquinaTuring* maquinaTraduzida = (MaquinaTuring*) malloc(sizeof(MaquinaTuring));
    if (!maquinaTraduzida) {
        fprintf(stderr, "ERRO: Falha na alocação de memória para a máquina traduzida.\n");
        exit(EXIT_FAILURE);
    }
    maquinaTraduzida->tipo = 'S';
    maquinaTraduzida->numTransicoes = 0;
    maquinaTraduzida->capacidadeTransicoes = 2000; // suficiente para muitas regras
    maquinaTraduzida->transicoes = (Transicao*) malloc(maquinaTraduzida->capacidadeTransicoes * sizeof(Transicao));
    if (!maquinaTraduzida->transicoes) {
        fprintf(stderr, "ERRO: Falha na alocação de memória para as transições traduzidas.\n");
        exit(EXIT_FAILURE);
    }

    printf("-> Traduzindo I -> S (lógica baseada no script Python)...\n");

    // === Recolher conjunto de estados da maquinaOriginal, garantindo unicidade ===
    int capEstados = 128;
    int numEstados = 0;
    char** estados = (char**) malloc(capEstados * sizeof(char*));
    if (!estados) { fprintf(stderr, "ERRO: malloc estados\n"); exit(EXIT_FAILURE); }

    // Primeiro, adicione todos os estados, lidando com a renomeação de '0' para 'inicio'
    for (int i = 0; i < maquinaOriginal->numTransicoes; ++i) {
        const Transicao* t = &maquinaOriginal->transicoes[i];
        const char* current_state_name = t->estadoAtual;
        const char* next_state_name = t->novoEstado;

        // Adicionar estado atual se não existe
        int found_current = 0;
        for (int j = 0; j < numEstados; ++j) {
            if (estados[j] && strcmp(estados[j], current_state_name) == 0) { found_current = 1; break; }
        }
        if (!found_current) {
            if (numEstados >= capEstados) {
                capEstados *= 2;
                estados = (char**) realloc(estados, capEstados * sizeof(char*));
                if (!estados) { fprintf(stderr,"ERRO realloc estados\n"); exit(EXIT_FAILURE); }
            }
            estados[numEstados++] = strdup(current_state_name);
        }

        // Adicionar próximo estado se não existe
        int found_next = 0;
        for (int j = 0; j < numEstados; ++j) {
            if (estados[j] && strcmp(estados[j], next_state_name) == 0) { found_next = 1; break; }
        }
        if (!found_next) {
            if (numEstados >= capEstados) {
                capEstados *= 2;
                estados = (char**) realloc(estados, capEstados * sizeof(char*));
                if (!estados) { fprintf(stderr,"ERRO realloc estados\n"); exit(EXIT_FAILURE); }
            }
            estados[numEstados++] = strdup(next_state_name);
        }
    }
    // Agora 'estados' contém uma lista única de todos os estados (onde '0' já foi renomeado para 'inicio')
    // --- FIM: Parte de inicialização e coleta de estados ---


    // === Agora construir as transicoes pre-processamento ===
    printf("-> Adicionando transições de pré-processamento...\n");
    // O estado '0' é o estado inicial real da MT de Sipser.
    // Ele vai varrer a entrada, marcando-a e configurando a fita para o formato '# [neg] * [pos] &'
    
    // As transições de setup que você tinha originalmente para '0', '1_0', '1_1', etc.
    // Esta parte do setup é complexa. Vou reintroduzir o setup que você tinha,
    // mas com o cuidado de não usar 's' nas transições intermediárias.

    adicionar_transicao(maquinaTraduzida, "0", "0", "#", 'r', "1_0");
    adicionar_transicao(maquinaTraduzida, "0", "1", "#", 'r', "1_1");
    adicionar_transicao(maquinaTraduzida, "0", "_", "*", 'r', "inicio"); // Se a fita é vazia, coloca '*' e vai para 'inicio'

    adicionar_transicao(maquinaTraduzida, "1_0", "0", "0", 'r', "1_0");
    adicionar_transicao(maquinaTraduzida, "1_0", "1", "0", 'r', "1_1");
    adicionar_transicao(maquinaTraduzida, "1_0", "_", "0", 'r', "1__");

    adicionar_transicao(maquinaTraduzida, "1_1", "0", "1", 'r', "1_0");
    adicionar_transicao(maquinaTraduzida, "1_1", "1", "1", 'r', "1_1");
    adicionar_transicao(maquinaTraduzida, "1_1", "_", "1", 'r', "1__");

    adicionar_transicao(maquinaTraduzida, "1__", "_", "&", 'l', "2_");

    adicionar_transicao(maquinaTraduzida, "2_", "#", "#", 'r', "inicio");
    adicionar_transicao(maquinaTraduzida, "2_", "*", "*", 'l', "2_"); // Move para a esquerda até '#'

    // === FASE 1: Traduzir cada transição original da maquinaOriginal ===
    printf("-> Criando regras para cada transição da máquina Duplamente Infinita original...\n");
    for (int i = 0; i < maquinaOriginal->numTransicoes; ++i) {
        const Transicao* t = &maquinaOriginal->transicoes[i];

        const char* estadoAtualMapeado = t->estadoAtual; 
        const char* novoEstadoMapeado = t->novoEstado;
        
        char estadoAuxMovimento[512]; // Estado auxiliar para simular movimento e escrita

        // Transição: (estadoAtualMapeado, simboloAtual) -> (novoSimbolo, direcao, novoEstadoMapeado)

        if (t->direcao == 's') { // Simular direção 's' (stay/parar)
            // Passo 1: Escreve o novo símbolo e move para a direita (temporariamente)
            snprintf(estadoAuxMovimento, sizeof(estadoAuxMovimento), "%s_stay_aux_R_%s_sym_%s_new_sym_%s",
                     estadoAtualMapeado, novoEstadoMapeado, t->simboloAtual, t->novoSimbolo);
            adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, t->simboloAtual, t->novoSimbolo, 'r', estadoAuxMovimento);

            // Passo 2: Do estado auxiliar, move para a esquerda para o novo estado (completando o "stay")
            // Precisa de regras para todos os símbolos possíveis na fita
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "0", "0", 'l', novoEstadoMapeado);
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "1", "1", 'l', novoEstadoMapeado);
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "_", "_", 'l', novoEstadoMapeado);
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "#", "#", 'l', novoEstadoMapeado);
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "&", "&", 'l', novoEstadoMapeado);
            adicionar_transicao(maquinaTraduzida, estadoAuxMovimento, "*", "*", 'l', novoEstadoMapeado);

        } else if (t->direcao == 'r') { // Mover para a direita
            // Regra principal para mover para a direita: escreve novo símbolo e move 'r'
            adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, t->simboloAtual, t->novoSimbolo, 'r', novoEstadoMapeado);
            
            // Regra especial: se o movimento para a direita encontra o marcador '&', a fita precisa ser estendida (shift)
            char estadoShiftAmpWriteSym[512]; // Estado para escrever o novo símbolo no lugar de '&'
            char estadoReturnAfterAmpShift[512]; // Estado para o retorno após o shift (cabeçote está em '_', ex-&, antes de ir para o novo estado)

            snprintf(estadoShiftAmpWriteSym, sizeof(estadoShiftAmpWriteSym), "%s_shift_amp_write_%s", 
                     estadoAtualMapeado, t->novoSimbolo); 
            snprintf(estadoReturnAfterAmpShift, sizeof(estadoReturnAfterAmpShift), "%s_return_after_amp_shift", 
                     novoEstadoMapeado); 

            // 1. Se encontra '&', escreve o novo símbolo (t->novoSimbolo), e move para a direita para encontrar o blank
            adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, "&", t->novoSimbolo, 'r', estadoShiftAmpWriteSym); 
            
            // 2. No estado `estadoShiftAmpWriteSym`, quando encontra um blank `_`, escreve `&`, move para a esquerda e vai para o estado de retorno
            adicionar_transicao(maquinaTraduzida, estadoShiftAmpWriteSym, "_", "&", 'l', estadoReturnAfterAmpShift);

            // 3. No estado de retorno, o cabeçote está na posição onde antes estava o '&' (agora `_`).
            // Ele "permanece" para simular a chegada no estado `novoEstadoMapeado` lendo `_`.
            // Ou seja, a fita foi estendida, e o cabeçote está na célula que a MT original veria como a "próxima"
            adicionar_transicao(maquinaTraduzida, estadoReturnAfterAmpShift, "_", "_", 's', novoEstadoMapeado);


        } else if (t->direcao == 'l') { // Mover para a esquerda
            // Regra principal para mover para a esquerda: escreve novo símbolo e move 'l'
            adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, t->simboloAtual, t->novoSimbolo, 'l', novoEstadoMapeado);
            
            // Regra especial: se o movimento para a esquerda encontra o marcador '#', a fita precisa ser estendida (shift)
            char q1_hash_novoEstado[512];
            snprintf(q1_hash_novoEstado, sizeof(q1_hash_novoEstado), "q1___%s", novoEstadoMapeado); // Reutilizando a lógica de q1___e para o shift
            adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, "#", t->novoSimbolo, 'r', q1_hash_novoEstado); // Move 'r' para iniciar o shift para '#'
            
            // NOVO: Adicionar uma transição para quando uma regra de movimento 'l' tenta escrever '&' no lugar de '&'
            // Exemplo da sua entrada: `1 _ F l 2` se torna `1 _ & l 2`.
            // Se o `simboloAtual` é `&` e `novoSimbolo` também é `&`, e a direção é `l`, precisamos tratar isso.
            if (strcmp(t->simboloAtual, "&") == 0 && strcmp(t->novoSimbolo, "&") == 0) {
                // Esta é uma regra que simula uma "autotransição" no '&' com movimento 'l'
                // A ação é a mesma que quando a MT tenta mover para a esquerda e encontra '#':
                // a fita toda precisa ser "deslocada" para a direita, e o cabeçote retorna para a posição.
                // Reutilizamos a sub-rotina de shift (`q1___e`) para isso.
                adicionar_transicao(maquinaTraduzida, estadoAtualMapeado, "&", "&", 'r', q1_hash_novoEstado); // Move 'r' para iniciar o shift (com o novo estado 'q1_hash_novoEstado')
            }
        }
    }

    // === FASE 2: Gerar as sub-rotinas de shift ===
    printf("-> Gerando sub-rotinas de shift para cada estado...\n");
    for (int idx = 0; idx < numEstados; ++idx) {
        const char* e = estados[idx]; 

        // Declarações movedas para dentro do loop para garantir que existam para cada 'e'
        char nome_q1___e[512];
        char nome_q1_0_e[512], nome_q1_1_e[512], nome_q1_amp_e[512]; // Alterado F para amp
        char nome_q2e[512];
        char nome_q_amp_e[512]; // Alterado F para amp

        snprintf(nome_q1___e, sizeof(nome_q1___e), "q1___%s", e);
        snprintf(nome_q1_0_e, sizeof(nome_q1_0_e), "q1_0_%s", e);
        snprintf(nome_q1_1_e, sizeof(nome_q1_1_e), "q1_1_%s", e);
        snprintf(nome_q1_amp_e, sizeof(nome_q1_amp_e), "q1_amp_%s", e); // Renomeado
        snprintf(nome_q2e, sizeof(nome_q2e), "q2%s", e);
        snprintf(nome_q_amp_e, sizeof(nome_q_amp_e), "q_amp%s", e); // Renomeado


        // Estes estados (q1_0_e, q1_1_e, q1___e) são para o "shift right" da fita.
        adicionar_transicao(maquinaTraduzida, nome_q1_0_e, "0", "0", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, nome_q1_0_e, "1", "0", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, nome_q1_0_e, "_", "0", 'r', nome_q1___e); 
        adicionar_transicao(maquinaTraduzida, nome_q1_0_e, "&", "0", 'r', nome_q1_amp_e); // Alterado F para &
        adicionar_transicao(maquinaTraduzida, nome_q1_0_e, "*", "0", 'r', "q1_star_0"); 

        adicionar_transicao(maquinaTraduzida, nome_q1_1_e, "0", "1", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, nome_q1_1_e, "1", "1", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, nome_q1_1_e, "_", "1", 'r', nome_q1___e); 
        adicionar_transicao(maquinaTraduzida, nome_q1_1_e, "&", "1", 'r', nome_q1_amp_e); // Alterado F para &
        adicionar_transicao(maquinaTraduzida, nome_q1_1_e, "*", "1", 'r', "q1_star_1"); 

        adicionar_transicao(maquinaTraduzida, nome_q1___e, "0", "_", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, nome_q1___e, "1", "_", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, nome_q1___e, "_", "_", 'r', nome_q1___e); // Loop para mover blanks
        adicionar_transicao(maquinaTraduzida, nome_q1___e, "&", "_", 'r', nome_q1_amp_e); // Alterado F para &
        adicionar_transicao(maquinaTraduzida, nome_q1___e, "*", "_", 'r', "q1_star_blank"); 

        // q1_amp_e -> quando o shift atinge o '&' final.
        adicionar_transicao(maquinaTraduzida, nome_q1_amp_e, "*", "&", 'l', nome_q2e); // Se lê '*', escreve '&', volta para iniciar o re-posicionamento
        adicionar_transicao(maquinaTraduzida, nome_q1_amp_e, "_", "&", 'l', nome_q2e); // Se encontra blank, escreve '&', volta. 

        // q1_star_X estados: para lidar com o marcador '*'.
        // q1_star_0 (estava segurando '0', encontrou '*') -> escreve '0', move 'r', guarda '*'
        adicionar_transicao(maquinaTraduzida, "q1_star_0", "0", "*", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_0", "1", "*", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_0", "_", "*", 'r', nome_q1___e);
        adicionar_transicao(maquinaTraduzida, "q1_star_0", "&", "*", 'r', nome_q1_amp_e); // Alterado F para &

        adicionar_transicao(maquinaTraduzida, "q1_star_1", "0", "*", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_1", "1", "*", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_1", "_", "*", 'r', nome_q1___e);
        adicionar_transicao(maquinaTraduzida, "q1_star_1", "&", "*", 'r', nome_q1_amp_e); // Alterado F para &

        adicionar_transicao(maquinaTraduzida, "q1_star_blank", "0", "*", 'r', nome_q1_0_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_blank", "1", "*", 'r', nome_q1_1_e);
        adicionar_transicao(maquinaTraduzida, "q1_star_blank", "_", "*", 'r', nome_q1___e);
        adicionar_transicao(maquinaTraduzida, "q1_star_blank", "&", "*", 'r', nome_q1_amp_e); // Alterado F para &


        // q2e: loop left on '*', then on '#' go right to e (Retorna para o estado original 'e' após o shift)
        adicionar_transicao(maquinaTraduzida, nome_q2e, "*", "*", 'l', nome_q2e);
        adicionar_transicao(maquinaTraduzida, nome_q2e, "#", "#", 'r', e); // Alterado I para #
        
        // q_amp_e: '*', write '&', l, e (Reposiciona '&' e retorna para 'e') - Este estado é para o shift da direita
        adicionar_transicao(maquinaTraduzida, nome_q_amp_e, "*", "&", 'l', e); // Se encontra '*', escreve '&', move para a esquerda para 'e'
        adicionar_transicao(maquinaTraduzida, nome_q_amp_e, "0", "0", 'l', nome_q_amp_e); // Continua movendo esquerda
        adicionar_transicao(maquinaTraduzida, nome_q_amp_e, "1", "1", 'l', nome_q_amp_e); // Continua movendo esquerda
        adicionar_transicao(maquinaTraduzida, nome_q_amp_e, "_", "_", 'l', nome_q_amp_e); // Continua movendo esquerda
        adicionar_transicao(maquinaTraduzida, nome_q_amp_e, "#", "#", 'r', e); // Se encontra '#', move para direita e vai para 'e' // Alterado I para #
    }


    // === Liberar lista temporaria de estados ===
    for (int i = 0; i < numEstados; ++i) {
        if (estados[i]) free(estados[i]);
    }
    free(estados);

    return maquinaTraduzida;
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