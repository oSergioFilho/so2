#ifndef SHM_COM_H
#define SHM_COM_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> // necessário para a union semun

// Tamanho máximo de cada mensagem
#define TEXT_SZ 2048

// Tamanho do buffer circular
#define BUFFER_SIZE 10

// Estrutura da memória compartilhada
struct shared_use_st {
    int written_by_you[BUFFER_SIZE];           // Flags de ocupação
    char some_text[BUFFER_SIZE][TEXT_SZ];      // Mensagens
    int write_index;                           // Índice do produtor
    int read_index;                            // Índice do consumidor
};

// Definição da union semun (necessária para semctl em sistemas POSIX)
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
    // A GNU já define a union semun
#else
    union semun {
        int val;                  // valor para SETVAL
        struct semid_ds *buf;     // buffer para IPC_STAT, IPC_SET
        unsigned short *array;    // array para GETALL, SETALL
        struct seminfo *__buf;    // buffer para IPC_INFO
    };
#endif

#endif
