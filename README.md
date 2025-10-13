# Tradutor de Máquinas de Turing

Este projeto foi desenvolvido para a disciplina de Teoria da Computação do curso de Ciência da Computação da Universidade do Estado de Santa Catarina (UDESC). O objetivo é implementar um programa em C capaz de traduzir a representação de uma Máquina de Turing (MT) entre dois modelos distintos: o modelo de **Sipser (fita semi-infinita)** e o modelo de **fita duplamente infinita**.

A sintaxe de entrada e saída é compatível com o simulador online **Morphett's Turing Machine Simulator**, disponível em [http://morphett.info/turing/turing.html](http://morphett.info/turing/turing.html).

## Conceitos

O trabalho explora a equivalência computacional entre diferentes modelos de Máquinas de Turing.

### Máquina de Sipser (Fita Semi-infinita)
Caracteriza-se por uma fita que possui um início definido à esquerda e se estende infinitamente para a direita. O cabeçote de leitura/escrita não pode se mover para a esquerda da primeira célula, agindo como se houvesse uma "parede" intransponível.

### Máquina de Fita Duplamente Infinita
Possui uma fita que se estende infinitamente em ambas as direções (esquerda e direita).

## Funcionalidades

O programa `tradutor` realiza as seguintes conversões:

*   **Tradução `S -> I` (Sipser para Infinita):** Simula a "parede" da fita de Sipser adicionando um marcador especial (`#`) na fita infinita. Regras são geradas para que, ao encontrar este marcador, a máquina "ricocheteie" para a direita, imitando o comportamento da parede.

*   **Tradução `I -> S` (Infinita para Sipser):** Simula a fita infinita da máquina original em uma fita semi-infinita. Isso é feito através de uma sub-rotina de *setup* que prepara a fita com um marcador `#` no início e desloca a palavra de entrada para a direita. Para simular a fita infinita, a máquina de saída trata o marcador `#` como uma parede, impedindo o movimento para a esquerda.

## Tecnologias Utilizadas

*   **Linguagem:** C
*   **Compilador:** GCC (padrão no Ubuntu 22.04)

## Como Usar

### Pré-requisitos
É necessário ter um ambiente Linux (como o Ubuntu 22.04) com o compilador `gcc` instalado.

```bash
# Para instalar o GCC no Ubuntu/Debian, caso não o tenha
sudo apt update
sudo apt install build-essential
```

### 1. Compilação
Clone o repositório e navegue até a pasta do projeto. Para compilar o programa, execute o seguinte comando no terminal:

```bash
gcc tradutor.c -o tradutor
```
Isso criará um arquivo executável chamado `tradutor`.

### 2. Execução
Para executar o programa, utilize a seguinte sintaxe:

```bash
./tradutor <arquivo_de_entrada.in> <arquivo_de_saida.out>
```
*   `<arquivo_de_entrada.in>`: O caminho para o arquivo contendo a descrição da Máquina de Turing original.
*   `<arquivo_de_saida.out>`: O caminho onde o arquivo com a Máquina de Turing traduzida será salvo.

## Formato dos Arquivos

### Arquivo de Entrada (`.in`)
*   A **primeira linha** deve indicar o tipo da máquina:
    *   `;S` para o modelo de Sipser.
    *   `;I` para o modelo de fita duplamente infinita.
*   As **linhas seguintes** devem conter as transições, com 5 colunas separadas por espaço, no formato:
    `<estado_atual> <simbolo_atual> <novo_simbolo> <direcao> <novo_estado>`

**Exemplo de Entrada (`exemplo.in`):**
```
;S
0 1 x r 1
0 0 x r 1
1 1 _ r 1
1 0 _ r 1
1 _ F l 2
2 _ * l 2
2 x * l 3
3 x * l 4
4 x * l 5
5 x _ * halt-accept
```

### Arquivo de Saída (`.out`)
O arquivo de saída segue o mesmo formato, mas conterá a máquina traduzida e a primeira linha com o tipo oposto ao da entrada.

**Exemplo de Saída para `exemplo.in` (`saida.out`):**
```
;I
0 * * l marcar_#
marcar_# * # r 0_dir
0_dir 1 x r 1_dir
0_dir 0 x r 1_dir
1_dir 1 _ r 1_dir
1_dir 0 _ r 1_dir
1_dir _ F l 2_dir
2_dir # # r 2_dir
2_dir _ * l 2_dir
2_dir x * l 3_dir
3_dir # # r 3_dir
3_dir x * l 4_dir
4_dir # # r 4_dir
4_dir x * l 5_dir
5_dir # # r 5_dir
5_dir x _ * halt-accept
```

## Professora

**Karina Girardi Roggia**

## Autores

**Gustavo de Souza e José Augusto Laube**

---
