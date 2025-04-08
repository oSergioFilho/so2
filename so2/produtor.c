#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm_com.h"
#include "semun.h"  // Certifique-se de incluir o semun.h

// Funções de semáforo
static int up_vazio(void);
static int down_vazio(void);
static int up_cheio(void);
static int down_cheio(void);

// Identificadores dos semáforos
int vazio_id, cheio_id;

int main() {
    int running = 1;
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    char buffer[BUFSIZ];
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

    // Inicializando semáforos
    union semun sem_union;
    sem_union.val = BUFFER_SIZE; // Começa com todos os espaços disponíveis
    semctl(vazio_id, 0, SETVAL, sem_union);

    sem_union.val = 0; // Nenhum espaço cheio no início
    semctl(cheio_id, 0, SETVAL, sem_union);

    // Inicializando o buffer
    shared_stuff->write_index = 0;
    shared_stuff->read_index = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        shared_stuff->written_by_you[i] = 0;
    }

    while (running) {
        down_vazio();  // Espera espaço disponível

        // Entrada do produtor
        printf("Enter some text: ");
        fgets(buffer, BUFSIZ, stdin);
        strncpy(shared_stuff->some_text[shared_stuff->write_index], buffer, TEXT_SZ);
        shared_stuff->written_by_you[shared_stuff->write_index] = 1;

        shared_stuff->write_index = (shared_stuff->write_index + 1) % BUFFER_SIZE;

        up_cheio();  // Sinaliza que há algo para consumir

        if (strncmp(buffer, "end", 3) == 0) {
            running = 0;
        }
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
