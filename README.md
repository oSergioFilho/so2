# Projeto Produtor-Consumidor com Memória Compartilhada e Semáforos (System V)

## 🎯 Objetivo

Este projeto tem como objetivo demonstrar a comunicação entre processos usando **memória compartilhada** e **semáforos** no estilo **System V IPC**. Trata-se de uma implementação do problema clássico **Produtor-Consumidor** com controle de concorrência via semáforos.

Dois processos distintos — um **produtor** e um **consumidor** — compartilham um buffer circular de tamanho fixo. O produtor insere mensagens nesse buffer, enquanto o consumidor as retira, sempre na mesma ordem.

---

## 🧱 Arquivos do Projeto

- `produtor.c` – Código do processo produtor.
- `consumidor.c` – Código do processo consumidor.
- `shm_com.h` – Define a estrutura da memória compartilhada, tamanhos do buffer e a `union semun` necessária para uso com semáforos.

> ✅ **Nota**: Todo o conteúdo necessário (inclusive `union semun`) foi centralizado em `shm_com.h`.

---

## 🛠️ Tecnologias e Conceitos Usados

### 📦 Memória Compartilhada (`shmget`, `shmat`, `shmdt`)
- Permite que os dois processos compartilhem um espaço comum de memória.
- Usada para armazenar um buffer circular de mensagens.

### 🎛️ Semáforos (`semget`, `semop`, `semctl`)
Dois semáforos são utilizados para controlar o acesso ao buffer:

- **`vazio_id`** – Indica quantas posições livres existem no buffer.
- **`cheio_id`** – Indica quantas mensagens estão disponíveis para leitura.

Eles garantem que:
- O produtor não sobrescreva posições ocupadas.
- O consumidor não leia posições vazias.

---

## 🔁 Funcionamento dos Processos

### 🟩 Produtor

1. Aguarda espaço livre (`down_vazio`).
2. Lê uma string do usuário.
3. Escreve no buffer circular.
4. Avança o índice de escrita (`write_index`).
5. Sinaliza o semáforo de cheio (`up_cheio`).
6. Se a entrada for `"end"`, finaliza.

### 🟦 Consumidor

1. Aguarda mensagens disponíveis (`down_cheio`).
2. Lê a próxima mensagem do buffer.
3. Imprime no terminal.
4. Marca a posição como livre.
5. Avança o índice de leitura (`read_index`).
6. Libera espaço (`up_vazio`).
7. Finaliza se a mensagem for `"end"`.

---

## 📐 Estrutura da Memória Compartilhada (`shm_com.h`)

```c
#define TEXT_SZ 2048
#define BUFFER_SIZE 10

struct shared_use_st {
    int written_by_you[BUFFER_SIZE];
    char some_text[BUFFER_SIZE][TEXT_SZ];
    int write_index;
    int read_index;
};
