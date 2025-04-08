#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>     // Biblioteca para memória compartilhada
#include <sys/sem.h>     // Biblioteca para semáforos
#include "shm_com.h"     // Struct da memória compartilhada e tamanhos
#include "semun.h"       // Definição da união semun para uso no semctl

// Identificadores globais dos semáforos
int vazio_id, cheio_id;

// Declaração das funções auxiliares de semáforo
static int up_vazio(void);
static int down_vazio(void);
static int up_cheio(void);
static int down_cheio(void);

int main() {
    int running = 1;  // Controle do loop principal
    void *shared_memory = (void *)0;  // Ponteiro genérico para a memória compartilhada
    struct shared_use_st *shared_stuff;  // Ponteiro para a struct da memória compartilhada
    int shmid;

    // 🔧 Acessa a memória compartilhada existente (ou cria se não existir)
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // 🔗 Mapeia a memória compartilhada para o espaço do processo
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    printf("Memory attached at %p\n", shared_memory);

    // Faz o cast da memória para a struct de uso compartilhado
    shared_stuff = (struct shared_use_st *)shared_memory;

    // 📦 Acessa (ou cria) os semáforos necessários
    vazio_id = semget((key_t)12345, 1, 0666 | IPC_CREAT); // controla posições vazias
    cheio_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);  // controla posições cheias

    // 🎛️ Inicialização dos semáforos (garante valores corretos)
    union semun sem_union;

    sem_union.val = 0; // no início, nenhum espaço está cheio
    semctl(cheio_id, 0, SETVAL, sem_union);

    sem_union.val = BUFFER_SIZE; // todos os espaços estão vazios
    semctl(vazio_id, 0, SETVAL, sem_union);

    // 🔁 Inicializa os índices e flags da memória compartilhada
    shared_stuff->write_index = 0;
    shared_stuff->read_index = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        shared_stuff->written_by_you[i] = 0;
    }

    // 🔁 Loop principal do consumidor
    while (running) {
        down_cheio();  // Espera até haver uma mensagem para consumir

        // Lê a mensagem atual do buffer circular
        char *msg = shared_stuff->some_text[shared_stuff->read_index];
        printf("Received text: %s", msg);

        // Se a mensagem for "end", encerra o loop
        if (strncmp(msg, "end", 3) == 0) {
            running = 0;
        }

        // Marca a posição como livre
        shared_stuff->written_by_you[shared_stuff->read_index] = 0;

        // Avança o índice circular de leitura
        shared_stuff->read_index = (shared_stuff->read_index + 1) % BUFFER_SIZE;

        // Libera um espaço no buffer (produtor pode usar novamente)
        up_vazio();
    }

    // Desanexa a memória compartilhada
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

// 🔼 Incrementa o semáforo cheio (sinaliza nova mensagem)
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

// 🔽 Decrementa o semáforo cheio (espera até haver mensagens)
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
