/*
 * Instituição: Universidade Federal da Paraíba
 * Disciplina: Sistemas Operacionais I
 * Unidade 1 - Atividade 2
 *
 * Autor: Rodrigo Monteiro Fortes de Oliveira, 20240097664
 *
 * Nome do Programa: threads.c
 *
 * Descrição:
 *     Gera 10.000 números aleatórios entre 0 e 100 e calcula média,
 *     mediana e desvio padrão usando três threads paralelas.
 *     Cada thread faz um cálculo diferente e salva o resultado em
 *     variáveis globais. A thread principal mostra os resultados
 *     depois que todas terminam. Mede tempo de criação e execução.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define N_ENTRADAS 10000
#define MIN_VALOR 0
#define MAX_VALOR 100

// Variáveis globais para armazenar os resultados
double resultado_media = 0.0;
double resultado_mediana = 0.0;
double resultado_desvio = 0.0;

// Vetor global compartilhado entre as threads
int valores[N_ENTRADAS];

// Compara dois inteiros (usado pelo qsort)
int comparar(const void *a, const void *b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);
    
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// Thread que calcula a média
void* thread_media(void *arg) {
    long long soma = 0;
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        soma += valores[i];
    }
    
    resultado_media = (double)soma / N_ENTRADAS;
    
    pthread_exit(NULL);
}

// Thread que calcula a mediana
void* thread_mediana(void *arg) {
    // Copia o vetor para não alterar o original
    int *copia = (int*)malloc(N_ENTRADAS * sizeof(int));
    if (copia == NULL) {
        fprintf(stderr, "Erro ao alocar memória para cópia do vetor\n");
        pthread_exit(NULL);
    }
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        copia[i] = valores[i];
    }
    
    // Ordena o vetor
    qsort(copia, N_ENTRADAS, sizeof(int), comparar);
    
    if (N_ENTRADAS % 2 == 0) {
        // Par: média dos dois do meio
        resultado_mediana = (copia[N_ENTRADAS/2] + copia[N_ENTRADAS/2 - 1]) / 2.0;
    } else {
        // Ímpar: elemento do meio
        resultado_mediana = copia[N_ENTRADAS/2];
    }
    
    free(copia);
    pthread_exit(NULL);
}

// Thread que calcula o desvio padrão
void* thread_desvio(void *arg) {
    // Calcula a média primeiro (precisa para o desvio)
    long long soma = 0;
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        soma += valores[i];
    }
    
    double media_local = (double)soma / N_ENTRADAS;
    
    // Soma dos quadrados das diferenças
    double soma_quadrados = 0.0;
    
    for (int i = 0; i < N_ENTRADAS; i++) {
        double diferenca = valores[i] - media_local;
        soma_quadrados += diferenca * diferenca;
    }
    
    double variancia = soma_quadrados / N_ENTRADAS;
    resultado_desvio = sqrt(variancia);
    
    pthread_exit(NULL);
}

int main() {
    // Inicializa gerador aleatório
    srand(time(NULL));
    
    // Preenche o vetor com números aleatórios de 0 a 100
    for (int i = 0; i < N_ENTRADAS; i++) {
        valores[i] = rand() % (MAX_VALOR - MIN_VALOR + 1) + MIN_VALOR;
    }
    
    printf("========================================\n");
    printf("  EXECUÇÃO COM TRÊS THREADS\n");
    printf("========================================\n\n");
    
    // Começa a medir o tempo total
    struct timeval inicio_total, fim_total;
    gettimeofday(&inicio_total, NULL);
    
    // Começa a medir o tempo de criação
    struct timeval inicio_criacao, fim_criacao;
    gettimeofday(&inicio_criacao, NULL);
    
    // IDs das threads
    pthread_t thread1, thread2, thread3;
    int ret1, ret2, ret3;
    
    // Cria thread da média
    ret1 = pthread_create(&thread1, NULL, thread_media, NULL);
    if (ret1 != 0) {
        fprintf(stderr, "Erro ao criar thread de média: %d\n", ret1);
        return EXIT_FAILURE;
    }
    
    // Cria thread da mediana
    ret2 = pthread_create(&thread2, NULL, thread_mediana, NULL);
    if (ret2 != 0) {
        fprintf(stderr, "Erro ao criar thread de mediana: %d\n", ret2);
        return EXIT_FAILURE;
    }
    
    // Cria thread do desvio padrão
    ret3 = pthread_create(&thread3, NULL, thread_desvio, NULL);
    if (ret3 != 0) {
        fprintf(stderr, "Erro ao criar thread de desvio padrão: %d\n", ret3);
        return EXIT_FAILURE;
    }
    
    // Para de medir o tempo de criação
    gettimeofday(&fim_criacao, NULL);
    
    // Espera todas as threads terminarem
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    
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
    printf("Tempo de criação das threads: %.3f ms\n", tempo_criacao);
    
    printf("\n========================================\n");
    printf("  Execução finalizada com sucesso\n");
    printf("========================================\n");
    
    return EXIT_SUCCESS;
}

