#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>     // Biblioteca para memória compartilhada
#include <sys/sem.h>     // Biblioteca para semáforos
#include "shm_com.h"     // Struct da memória compartilhada e tamanhos e Definição da união semun para uso no semctl

// Declaração das funções auxiliares de semáforo
static int up_vazio(void);
static int down_vazio(void);
static int up_cheio(void);
static int down_cheio(void);

// Identificadores globais dos semáforos
int vazio_id, cheio_id;

int main() {
    int running = 1;  // Controle do loop principal
    void *shared_memory = (void *)0;  // Ponteiro genérico para memória compartilhada
    struct shared_use_st *shared_stuff;  // Ponteiro para a struct da memória compartilhada
    char buffer[BUFSIZ];  // Buffer para ler a entrada do usuário
    int shmid;

    // 🔧 Criação (ou acesso) da memória compartilhada
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // 🔗 Mapeamento da memória compartilhada para o processo
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    printf("Memory attached at %p\n", shared_memory);

    // Cast da memória compartilhada para a struct definida
    shared_stuff = (struct shared_use_st *)shared_memory;

    // 📦 Criação dos semáforos (dois conjuntos, cada um com 1 semáforo)
    vazio_id = semget((key_t)12345, 1, 0666 | IPC_CREAT); // Conta espaços disponíveis
    cheio_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);  // Conta quantos espaços estão ocupados

    // 🎛️ Inicialização dos semáforos
    union semun sem_union;
    sem_union.val = BUFFER_SIZE; // BUFFER_SIZE = 10, ou seja, buffer vazio no início
    semctl(vazio_id, 0, SETVAL, sem_union); // Todos os slots estão livres

    sem_union.val = 0; // Nada foi produzido ainda
    semctl(cheio_id, 0, SETVAL, sem_union);

    // 🔁 Inicialização do buffer
    shared_stuff->write_index = 0;
    shared_stuff->read_index = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        shared_stuff->written_by_you[i] = 0;
    }

    // 🔁 Loop principal do produtor
    while (running) {
        down_vazio();  // Espera até que haja espaço livre no buffer

        // Leitura da mensagem do teclado
        printf("Enter some text: ");
        fgets(buffer, BUFSIZ, stdin);

        // Copia a mensagem para a posição atual do buffer circular
        strncpy(shared_stuff->some_text[shared_stuff->write_index], buffer, TEXT_SZ);
        shared_stuff->written_by_you[shared_stuff->write_index] = 1;

        // Atualiza o índice circular de escrita
        shared_stuff->write_index = (shared_stuff->write_index + 1) % BUFFER_SIZE;

        up_cheio();  // Sinaliza que há uma nova mensagem disponível para consumo

        // Verifica se o usuário digitou "end" para encerrar
        if (strncmp(buffer, "end", 3) == 0) {
            running = 0;
        }
    }

    // Desanexa a memória compartilhada e encerra o processo
    shmdt(shared_memory);
    exit(EXIT_SUCCESS);
}

// 🔽 Decrementa o semáforo vazio (espera até ter espaço livre)
static int down_vazio(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(vazio_id, &sem_b, 1) == -1) {
        perror("down_vazio failed");
        return 0;
    }
    return 1;
}

// 🔼 Incrementa o semáforo vazio (libera uma posição)
static int up_vazio(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(vazio_id, &sem_b, 1) == -1) {
        perror("up_vazio failed");
        return 0;
    }
    return 1;
}

// 🔼 Incrementa o semáforo cheio (sinaliza nova mensagem disponível)
static int up_cheio(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(cheio_id, &sem_b, 1) == -1) {
        perror("up_cheio failed");
        return 0;
    }
    return 1;
}

// 🔽 Decrementa o semáforo cheio (espera até haver mensagens disponíveis)
static int down_cheio(void) {
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(cheio_id, &sem_b, 1) == -1) {
        perror("down_cheio failed");
        return 0;
    }
    return 1;
}
