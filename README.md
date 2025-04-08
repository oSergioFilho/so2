# Projeto Produtor-Consumidor com Memória Compartilhada e Semáforos

## 🎯 Objetivo

Este projeto implementa a clássica solução do problema **Produtor-Consumidor** utilizando **memória compartilhada (System V)** e **semáforos** para comunicação e sincronização entre processos. O produtor insere mensagens em um buffer circular, enquanto o consumidor retira essas mensagens na mesma ordem.

A execução pode ser feita em sistemas Unix/Linux ou via **WSL (Windows Subsystem for Linux)**.

---

## 🧱 Estrutura do Projeto

- `produtor.c`: código-fonte do processo produtor.
- `consumidor.c`: código-fonte do processo consumidor.
- `shm_com.h`: define a estrutura de memória compartilhada e constantes como o tamanho do buffer.
- `semun.h`: define a `union semun`, necessária para o uso de `semctl()` em sistemas Linux.

---

## 🗂️ Conceitos Utilizados

### 🧠 Memória Compartilhada (`shmget`, `shmat`, `shmdt`)
Permite que dois processos (produtor e consumidor) compartilhem a mesma área de memória, acessando e modificando o mesmo buffer circular.

### 🎛️ Semáforos (`semget`, `semop`, `semctl`)
Dois semáforos são utilizados:

- **`vazio_id`**: controla o número de posições vazias no buffer.
- **`cheio_id`**: controla o número de posições ocupadas.

Isso evita **condições de corrida** e garante que:

- O produtor só escreva se houver espaço.
- O consumidor só leia se houver mensagens.

---

## 🔄 Funcionamento

### ✅ Produtor
- Aguarda espaço livre (`down_vazio`).
- Lê uma string do teclado.
- Armazena no buffer circular na posição atual de escrita.
- Avança o índice circular de escrita.
- Sinaliza que há uma nova mensagem disponível (`up_cheio`).
- Finaliza se a mensagem for `"end"`.

### ✅ Consumidor
- Aguarda até que haja uma mensagem (`down_cheio`).
- Lê a mensagem da posição atual de leitura.
- Imprime no terminal.
- Marca a posição como livre.
- Avança o índice de leitura.
- Libera uma posição do buffer (`up_vazio`).
- Finaliza se a mensagem for `"end"`.

---

## 📦 Estrutura da Memória Compartilhada (`shm_com.h`)

```c
#define TEXT_SZ 2048
#define BUFFER_SIZE 10

struct shared_use_st {
    int written_by_you[BUFFER_SIZE];
    char some_text[BUFFER_SIZE][TEXT_SZ];
    int write_index;
    int read_index;
};
