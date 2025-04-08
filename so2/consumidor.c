#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm_com.h"
#include "semun.h"  // Incluindo semun.h para a definição de union semun

// Identificadores dos semáforos globais
int vazio_id, cheio_id;

// Funções de semáforo
static int up_vazio(void);
static int down_vazio(void);
static int up_cheio(void);
static int down_cheio(void);

int main() {
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;

    // Criando a memória compartilhada
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    printf("Memory attached at %p\n", shared_memory);
    shared_stuff = (struct shared_use_st *)shared_memory;

    // Criando semáforos
    vazio_id = semget((key_t)12345, 1, 0666 | IPC_CREAT); // Semáforo para o espaço vazio
    cheio_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);  // Semáforo para o espaço cheio

    // Inicializando os semáforos (somente se necessário – não sobrescreve)
    union semun sem_union;

    sem_union.val = 0;
    semctl(cheio_id, 0, SETVAL, sem_union);

    sem_union.val = BUFFER_SIZE;
    semctl(vazio_id, 0, SETVAL, sem_union);

    // Inicializando o buffer
    shared_stuff->write_index = 0;
    shared_stuff->read_index = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        shared_stuff->written_by_you[i] = 0;
    }

    while (running) {
        down_cheio();  // Espera até que haja algo para consumir

        // Lê a mensagem atual
        char *msg = shared_stuff->some_text[shared_stuff->read_index];
        printf("Received text: %s", msg);

        // Se for "end", finaliza o loop
        if (strncmp(msg, "end", 3) == 0) {
            running = 0;
        }

        // Marca a posição como vazia
        shared_stuff->written_by_you[shared_stuff->read_index] = 0;

        // Avança o índice circular
        shared_stuff->read_index = (shared_stuff->read_index + 1) % BUFFER_SIZE;

        // Sinaliza que há espaço disponível para o produtor
        up_vazio();
    }

    shmdt(shared_memory);
    exit(EXIT_SUCCESS);
}

// Funções de semáforo
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
