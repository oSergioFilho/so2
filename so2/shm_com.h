/* Definindo o tamanho máximo do texto e o tamanho do buffer (10 mensagens) */

#define TEXT_SZ 2048
#define BUFFER_SIZE 10  // Número máximo de mensagens no buffer

struct shared_use_st {
    int written_by_you[BUFFER_SIZE];  // Marca se a posição está ocupada (1) ou vazia (0)
    char some_text[BUFFER_SIZE][TEXT_SZ];  // Buffer circular de mensagens
    int write_index;  // Índice para onde o próximo item será escrito
    int read_index;   // Índice para onde o próximo item será lido
};

