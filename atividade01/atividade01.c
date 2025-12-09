/*
 * Instituição: Universidade Federal da Paraíba
 * Disciplina: Sistemas Operacionais I
 * Unidade 1 - Atividade 1
 *
 * Autor: Rodrigo Monteiro Fortes de Oliveira, 20240097664
 * 
 * Descrição do Programa:
 * Este programa implementa uma hierarquia de processos totalizando 7 processos:
 * 1 Processo Pai (P1) -> 2 Processos Filhos (F1, F2) -> 4 Processos Netos (N1, N2, N3, N4).
 * 
 * Funcionalidade:
 * - Os processos Netos (N1, N2, N3, N4) utilizam a função execl() para substituir sua imagem 
 *   por comandos do terminal Linux (ls, pwd, date, whoami).
 * - Os processos Filhos (F1, F2) aguardam o término de seus respectivos netos antes 
 *   de imprimirem seus PIDs e o PID do pai (P1).
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <sys/wait.h>
 #include <sys/types.h>
 
 /**
  * Função: criarNeto
  * -----------------
  * Responsável por criar um processo folha (Neto) na árvore de processos.
  * Este processo utiliza execl para executar um comando do sistema.
  *
  * comando: string contendo o caminho do comando a ser executado (ex: "/bin/ls", "/bin/date").
  * arg1: primeiro argumento do comando (pode ser NULL ou o nome do comando).
  * arg2: segundo argumento do comando (pode ser NULL).
  */
 void criarNeto(const char* comando, const char* arg1, const char* arg2) {
     pid_t pid = fork(); // Chamada de sistema para duplicar o processo atual
     
     if (pid < 0) {
         // Erro crítico na criação do processo
         perror("Erro ao executar fork para o Neto");
         exit(EXIT_FAILURE);
     } 
     else if (pid == 0) {
         // --- Área do Processo Neto (N) ---
         
         // A função execl substitui a imagem do processo atual pelo comando especificado.
         // Formato: execl(caminho_do_comando, nome_do_comando, arg1, arg2, NULL)
         if (arg2 != NULL) {
             execl(comando, arg1, arg2, (char*)NULL);
         } else if (arg1 != NULL) {
             execl(comando, arg1, (char*)NULL);
         } else {
             execl(comando, comando, (char*)NULL);
         }
         
         // Se o execl funcionar, o código abaixo NUNCA será executado.
         // Se chegou aqui, houve erro na substituição da imagem.
         perror("Erro na execução do execl");
         exit(EXIT_FAILURE);
     }
     
     // Se pid > 0, estamos no processo Filho (pai do Neto), a função retorna para continuar o fluxo.
 }
 
 /**
  * Função: criarFilho
  * ------------------
  * Responsável por criar um processo intermediário (Filho) que gerencia dois Netos.
  *
  * cmdNetoA: Caminho do primeiro comando a ser executado por um dos netos.
  * arg1A: Primeiro argumento do primeiro comando.
  * arg2A: Segundo argumento do primeiro comando (pode ser NULL).
  * cmdNetoB: Caminho do segundo comando a ser executado pelo outro neto.
  * arg1B: Primeiro argumento do segundo comando.
  * arg2B: Segundo argumento do segundo comando (pode ser NULL).
  */
 void criarFilho(const char* cmdNetoA, const char* arg1A, const char* arg2A,
                 const char* cmdNetoB, const char* arg1B, const char* arg2B) {
     pid_t pid = fork(); // Criação do processo F1 ou F2
     
     if (pid < 0) {
         perror("Erro ao executar fork para o Filho");
         exit(EXIT_FAILURE);
     } 
     else if (pid == 0) {
         // --- Área do Processo Filho (F) ---
         
         // O Filho cria seus dois processos Netos (N)
         criarNeto(cmdNetoA, arg1A, arg2A);
         criarNeto(cmdNetoB, arg1B, arg2B);
         
         // Sincronização: O Filho aguarda o término dos seus dois Netos
         wait(NULL);
         wait(NULL);
         
         // Após os netos terminarem, o Filho imprime suas informações conforme solicitado
         printf("-> [Processo Filho] Finalizado. Meu PID: %d | PID do meu Pai (P1): %d\n", 
                getpid(), getppid());
         
         // O processo Filho encerra com sucesso
         exit(EXIT_SUCCESS);
     }
     
     // Se pid > 0, estamos no processo Pai (P1), a função retorna.
 }
 
 /**
  * Função: main
  * ------------
  * Ponto de entrada do Processo Pai (P1).
  */
 int main() {
     printf("Iniciando a árvore de processos...\n\n");
     
     // P1 cria o primeiro filho (F1), que gerenciará N1 (ls) e N2 (pwd)
     criarFilho("/bin/ls", "ls", "-l", "/bin/pwd", "pwd", NULL);
     
     // P1 cria o segundo filho (F2), que gerenciará N3 (date) e N4 (whoami)
     criarFilho("/bin/date", "date", NULL, "/usr/bin/whoami", "whoami", NULL);
     
     // Sincronização: O Pai (P1) deve esperar F1 e F2 terminarem
     wait(NULL);
     wait(NULL);
     
     // Mensagem final exigida pelo enunciado
     printf("\nSou o processo pai P1. Meu PID: %d\n", getpid());
     printf("Todos os processos filhos e netos terminaram. Encerrando o programa.\n");
     
     return 0;
 }
 
 /*
  * COMENTÁRIO SOBRE A ORDEM DE EXECUÇÃO DOS PROCESSOS N1-N4:
  * 
  * A ordem de execução dos processos N1-N4 não é determinística e depende do 
  * escalonador do sistema operacional. No entanto, podemos observar alguns padrões:
  * 
  * 1. N1 e N2 são filhos de F1, enquanto N3 e N4 são filhos de F2.
  *    Como F1 e F2 são criados sequencialmente, é provável que N1 e N2 sejam 
  *    criados antes de N3 e N4, mas não há garantia absoluta.
  * 
  * 2. Dentro de cada grupo (N1-N2 ou N3-N4), a ordem de execução também não é 
  *    garantida. O sistema operacional pode executar N1 antes de N2, ou vice-versa,
  *    dependendo da carga do sistema e do algoritmo de escalonamento.
  * 
  * 3. O uso de wait() garante apenas que:
  *    - F1 espera N1 e N2 terminarem antes de imprimir sua mensagem
  *    - F2 espera N3 e N4 terminarem antes de imprimir sua mensagem
  *    - P1 espera F1 e F2 terminarem antes de imprimir sua mensagem final
  * 
  * 4. A saída dos comandos (ls, pwd, date, whoami) pode aparecer em qualquer ordem,
  *    mas as mensagens de sincronização (dos processos F1, F2 e P1) sempre aparecerão
  *    na ordem correta devido ao uso de wait().
  * 
  * Exemplo de possível ordem de execução:
  *    N1 -> N2 -> N3 -> N4 (ou qualquer outra permutação)
  *    Mas a ordem de finalização das mensagens será sempre:
  *    F1 -> F2 -> P1 (devido ao wait())
  */
 
 