/*
 * Instituição: Universidade Federal da Paraíba
 * Disciplina: Sistemas Operacionais I
 * Unidade 1 - Atividade 2
 *
 * Autor: Rodrigo Monteiro Fortes de Oliveira, 20240097664
 *
 * Nome do Programa: processos.c
 *
 * Descrição:
 *     Gera 10.000 números aleatórios entre 0 e 100 e calcula média,
 *     mediana e desvio padrão usando três processos criados com fork().
 *     Cada processo faz um cálculo e envia o resultado para o processo
 *     pai através de pipes. Mede tempo de criação dos processos e
 *     tempo total de execução.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#define N_ENTRADAS 10000
#define MIN_VALOR 0
#define MAX_VALOR 100

// Identificadores dos tipos de resultados
typedef enum {
    RESULTADO_MEDIA = 1,
    RESULTADO_MEDIANA = 2,
    RESULTADO_DESVIO = 3
} TipoResultado;

// Vetor global compartilhado (herdado pelos processos filhos via fork)
int valores[N_ENTRADAS];

// Compara dois inteiros (usado pelo qsort)
int comparar(const void *a, const void *b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);
    
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// Processo filho que calcula a média
void calcular_media(int write_fd) {
    long long soma = 0;
    
    // Soma todos os valores
    for (int i = 0; i < N_ENTRADAS; i++) {
        soma += valores[i];
    }
    
    // Calcula a média
    double media = (double)soma / N_ENTRADAS;
    
    // Envia o resultado pelo pipe
    TipoResultado tipo = RESULTADO_MEDIA;
    if (write(write_fd, &tipo, sizeof(TipoResultado)) == -1) {
        perror("Erro ao escrever tipo no pipe");
        _exit(EXIT_FAILURE);
    }
    
    if (write(write_fd, &media, sizeof(double)) == -1) {
        perror("Erro ao escrever média no pipe");
        _exit(EXIT_FAILURE);
    }
    
    _exit(EXIT_SUCCESS);
}

// Processo filho que calcula a mediana
void calcular_mediana(int write_fd) {
    // Copia o vetor para não alterar o original
    int *copia = (int*)malloc(N_ENTRADAS * sizeof(int));
    if (copia == NULL) {
        fprintf(stderr, "Erro ao alocar memória para cópia do vetor\n");
        _exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        copia[i] = valores[i];
    }
    
    // Ordena o vetor
    qsort(copia, N_ENTRADAS, sizeof(int), comparar);
    
    double mediana;
    if (N_ENTRADAS % 2 == 0) {
        // Par: média dos dois do meio
        mediana = (copia[N_ENTRADAS/2] + copia[N_ENTRADAS/2 - 1]) / 2.0;
    } else {
        // Ímpar: elemento do meio
        mediana = copia[N_ENTRADAS/2];
    }
    
    free(copia);
    
    // Envia o resultado pelo pipe
    TipoResultado tipo = RESULTADO_MEDIANA;
    if (write(write_fd, &tipo, sizeof(TipoResultado)) == -1) {
        perror("Erro ao escrever tipo no pipe");
        _exit(EXIT_FAILURE);
    }
    
    if (write(write_fd, &mediana, sizeof(double)) == -1) {
        perror("Erro ao escrever mediana no pipe");
        _exit(EXIT_FAILURE);
    }
    
    _exit(EXIT_SUCCESS);
}

// Processo filho que calcula o desvio padrão
void calcular_desvio_padrao(int write_fd) {
    // Calcula a média primeiro (precisa para o desvio)
    long long soma = 0;
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        soma += valores[i];
    }
    
    double media = (double)soma / N_ENTRADAS;
    
    // Soma dos quadrados das diferenças
    double soma_quadrados = 0.0;
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        double diferenca = valores[i] - media;
        soma_quadrados += diferenca * diferenca;
    }
    
    // Calcula o desvio padrão
    double variancia = soma_quadrados / N_ENTRADAS;
    double desvio = sqrt(variancia);
    
    // Envia o resultado pelo pipe
    TipoResultado tipo = RESULTADO_DESVIO;
    if (write(write_fd, &tipo, sizeof(TipoResultado)) == -1) {
        perror("Erro ao escrever tipo no pipe");
        _exit(EXIT_FAILURE);
    }
    
    if (write(write_fd, &desvio, sizeof(double)) == -1) {
        perror("Erro ao escrever desvio padrão no pipe");
        _exit(EXIT_FAILURE);
    }
    
    _exit(EXIT_SUCCESS);
}

int main() {
    // Inicializa gerador aleatório
    srand(time(NULL));
    
    // Preenche o vetor com números aleatórios de 0 a 100
    for (int i = 0; i < N_ENTRADAS; i++) {
        valores[i] = rand() % (MAX_VALOR - MIN_VALOR + 1) + MIN_VALOR;
    }
    
    printf("========================================\n");
    printf("  EXECUÇÃO COM TRÊS PROCESSOS\n");
    printf("========================================\n\n");
    printf("PID do processo pai: %d\n\n", getpid());
    
    // Cria o pipe para comunicação
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("Erro ao criar pipe");
        return EXIT_FAILURE;
    }
    
    // Começa a medir o tempo total
    struct timeval inicio_total, fim_total;
    gettimeofday(&inicio_total, NULL);
    
    // Começa a medir o tempo de criação
    struct timeval inicio_criacao, fim_criacao;
    gettimeofday(&inicio_criacao, NULL);
    
    // PIDs dos processos filhos
    pid_t pids[3];
    
    // Cria processo da média
    pids[0] = fork();
    if (pids[0] == 0) {
        // Processo filho: fecha o lado de leitura do pipe
        close(pipe_fds[0]);
        calcular_media(pipe_fds[1]);
    } else if (pids[0] < 0) {
        perror("Erro ao criar fork para média");
        return EXIT_FAILURE;
    }
    
    // Cria processo da mediana
    pids[1] = fork();
    if (pids[1] == 0) {
        // Processo filho: fecha o lado de leitura do pipe
        close(pipe_fds[0]);
        calcular_mediana(pipe_fds[1]);
    } else if (pids[1] < 0) {
        perror("Erro ao criar fork para mediana");
        return EXIT_FAILURE;
    }
    
    // Cria processo do desvio padrão
    pids[2] = fork();
    if (pids[2] == 0) {
        // Processo filho: fecha o lado de leitura do pipe
        close(pipe_fds[0]);
        calcular_desvio_padrao(pipe_fds[1]);
    } else if (pids[2] < 0) {
        perror("Erro ao criar fork para desvio padrão");
        return EXIT_FAILURE;
    }
    
    // Para de medir o tempo de criação
    gettimeofday(&fim_criacao, NULL);
    
    // Pai: fecha escrita do pipe
    close(pipe_fds[1]);
    
    // Resultados recebidos
    double resultado_media = 0.0;
    double resultado_mediana = 0.0;
    double resultado_desvio = 0.0;
    
    // Lê todos os resultados dos filhos
    int resultados_recebidos = 0;
    while (resultados_recebidos < 3) {
        TipoResultado tipo;
        double valor;
        
        ssize_t bytes_lidos_tipo = read(pipe_fds[0], &tipo, sizeof(TipoResultado));
        if (bytes_lidos_tipo > 0) {
            ssize_t bytes_lidos_valor = read(pipe_fds[0], &valor, sizeof(double));
            if (bytes_lidos_valor > 0) {
                switch (tipo) {
                    case RESULTADO_MEDIA:
                        resultado_media = valor;
                        break;
                    case RESULTADO_MEDIANA:
                        resultado_mediana = valor;
                        break;
                    case RESULTADO_DESVIO:
                        resultado_desvio = valor;
                        break;
                }
                resultados_recebidos++;
            }
        }
    }
    
    // Fecha leitura do pipe
    close(pipe_fds[0]);
    
    // Espera todos os filhos terminarem
    for (int i = 0; i < 3; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    // Para de medir o tempo total
    gettimeofday(&fim_total, NULL);
    
    // Calcula tempo de criação (ms)
    long segundos_criacao = fim_criacao.tv_sec - inicio_criacao.tv_sec;
    long microsegundos_criacao = fim_criacao.tv_usec - inicio_criacao.tv_usec;
    double tempo_criacao = (segundos_criacao * 1000.0) + (microsegundos_criacao / 1000.0);
    
    // Calcula tempo total (ms)
    long segundos_total = fim_total.tv_sec - inicio_total.tv_sec;
    long microsegundos_total = fim_total.tv_usec - inicio_total.tv_usec;
    double tempo_total = (segundos_total * 1000.0) + (microsegundos_total / 1000.0);
    
    // Exibe resultados estatísticos
    printf("--- RESULTADOS ESTATÍSTICOS ---\n");
    printf("Quantidade de valores processados: %d\n", N_ENTRADAS);
    printf("Média aritmética: %.6f\n", resultado_media);
    printf("Mediana: %.6f\n", resultado_mediana);
    printf("Desvio padrão populacional: %.6f\n\n", resultado_desvio);
    
    // Exibe métricas de tempo
    printf("--- MÉTRICAS DE TEMPO ---\n");
    printf("Tempo total de execução: %.3f ms\n", tempo_total);
    printf("Tempo de criação dos processos: %.3f ms\n", tempo_criacao);
    
    printf("\n========================================\n");
    printf("  Execução finalizada com sucesso\n");
    printf("========================================\n");
    
    return EXIT_SUCCESS;
}

