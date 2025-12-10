/*
 * Instituição: Universidade Federal da Paraíba
 * Disciplina: Sistemas Operacionais I
 * Unidade 1 - Atividade 2
 *
 * Autor: Rodrigo Monteiro Fortes de Oliveira, 20240097664
 *
 * Nome do Programa: single_process.c
 *
 * Descrição:
 *     Gera 10.000 números aleatórios entre 0 e 100 e calcula média,
 *     mediana e desvio padrão em um único processo (sem fork ou pipes).
 *     Versão sequencial usada para comparar com a versão multiprocessada
 *     e ver diferenças de desempenho.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define N_ENTRADAS 10000
#define MIN_VALOR 0
#define MAX_VALOR 100

// Compara dois inteiros (usado pelo qsort)
int comparar(const void *a, const void *b) {
    int int_a = *((int*)a);
    int int_b = *((int*)b);
    
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

// Calcula a média
double calcular_media(int *valores, int tamanho) {
    long long soma = 0;
    
    for (int i = 0; i < tamanho; i++) {
        soma += valores[i];
    }
    
    return (double)soma / tamanho;
}

// Calcula a mediana
double calcular_mediana(int *valores, int tamanho) {
    // Copia o vetor para não alterar o original
    int *copia = (int*)malloc(tamanho * sizeof(int));
    if (copia == NULL) {
        fprintf(stderr, "Erro ao alocar memória para cópia do vetor\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < tamanho; i++) {
        copia[i] = valores[i];
    }
    
    // Ordena o vetor
    qsort(copia, tamanho, sizeof(int), comparar);
    
    double mediana;
    if (tamanho % 2 == 0) {
        // Par: média dos dois do meio
        mediana = (copia[tamanho/2] + copia[tamanho/2 - 1]) / 2.0;
    } else {
        // Ímpar: elemento do meio
        mediana = copia[tamanho/2];
    }
    
    free(copia);
    return mediana;
}

// Calcula o desvio padrão
double calcular_desvio_padrao(int *valores, int tamanho, double media) {
    double soma_quadrados = 0.0;
    
    for (int i = 0; i < tamanho; i++) {
        double diferenca = valores[i] - media;
        soma_quadrados += diferenca * diferenca;
    }
    
    double variancia = soma_quadrados / tamanho;
    return sqrt(variancia);
}

int main() {
    // Aloca memória para o vetor de valores
    int *valores = (int*)malloc(N_ENTRADAS * sizeof(int));
    if (valores == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o vetor\n");
        return EXIT_FAILURE;
    }
    
    // Inicializa gerador de números aleatórios
    srand(time(NULL));
    
    // Preenche o vetor com valores aleatórios entre 0 e 100
    for (int i = 0; i < N_ENTRADAS; i++) {
        valores[i] = rand() % (MAX_VALOR - MIN_VALOR + 1) + MIN_VALOR;
    }
    
    printf("========================================\n");
    printf("  EXECUÇÃO EM UM ÚNICO PROCESSO\n");
    printf("========================================\n\n");
    printf("PID do processo principal: %d\n\n", getpid());
    
    // Inicia medição de tempo
    struct timeval inicio, fim;
    gettimeofday(&inicio, NULL);
    
    // Calcula média
    double media = calcular_media(valores, N_ENTRADAS);
    
    // Calcula mediana
    double mediana = calcular_mediana(valores, N_ENTRADAS);
    
    // Calcula desvio padrão
    double desvio = calcular_desvio_padrao(valores, N_ENTRADAS, media);
    
    // Finaliza medição de tempo
    gettimeofday(&fim, NULL);
    
    // Calcula tempo total em milissegundos
    long segundos = fim.tv_sec - inicio.tv_sec;
    long microsegundos = fim.tv_usec - inicio.tv_usec;
    double tempo_total = (segundos * 1000.0) + (microsegundos / 1000.0);
    
    // Exibe resultados estatísticos
    printf("--- RESULTADOS ESTATÍSTICOS ---\n");
    printf("Quantidade de valores processados: %d\n", N_ENTRADAS);
    printf("Média aritmética: %.6f\n", media);
    printf("Mediana: %.6f\n", mediana);
    printf("Desvio padrão populacional: %.6f\n\n", desvio);
    
    // Exibe métricas de tempo
    printf("--- MÉTRICAS DE TEMPO ---\n");
    printf("Tempo total de execução: %.3f ms\n", tempo_total);
    
    // Libera memória alocada
    free(valores);
    
    printf("\n========================================\n");
    printf("  Execução finalizada com sucesso\n");
    printf("========================================\n");
    
    return EXIT_SUCCESS;
}

