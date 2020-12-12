#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "memory.h"
#include "prodcons.h"
#include "control.h"
#include "file.h"
#include "sotime.h"
#include "intermediario.h"
#include "cliente.h"
#include "empresa.h"


struct statistics Ind;     		// indicadores do funcionamento do SO_PerPro
struct configuration Config; 	// configuração da execução do SO_PerPro

/* main_cliente recebe como parâmetro o nº de clientes a criar */
void main_cliente(int quant)
{
    //==============================================
    // CRIAR PROCESSOS
    //
    // criar um número de processos cliente igual a quant através de um ciclo com n=0,1,...
	// após a criação de cada processo, o processo filho realiza duas atividades:
	// - chama a função cliente_executar passando como parâmetro o valor da variável de controlo do ciclo n=0,1,...
	// - chama a função exit passando como parâmetro o valor devolvido pela função cliente_executar
	// o processo pai guarda o pid do filho no vetor Ind.pid_clientes[n], com n=0,1,... e termina normalmente a função
    so_main_cliente(quant);
    //==============================================
}

/* main_intermediario recebe como parâmetro o nº de intermediarios a criar */
void main_intermediario(int quant)
{
    //==============================================
    // CRIAR PROCESSOS
    //
    // criar um número de processos intermediarios igual a quant através de um ciclo com n=0,1,...
	// após a criação de cada processo, o processo filho realiza duas atividades:
	// - chama a função intermediarios_executar passando como parâmetro o valor da variável de controlo do ciclo n=0,1,...
	// - chama a função exit passando como parâmetro o valor devolvido pela função intermediario_executar
	// o processo pai guarda o pid do filho no vetor Ind.pid_intermediarios[n], com n=0,1,... e termina normalmente a função
    so_main_intermediario(quant);
    //==============================================
}

/* main_empresas recebe como parâmetro o nº de empresas a criar */
void main_empresas(int quant)
{
    //==============================================
    // CRIAR PROCESSOS
    //
    // criar um número de processos empresas igual a quant através de um ciclo com n=0,1,...
	// após a criação de cada processo, o processo filho realiza duas atividades:
	// - chama a função empresas_executar passando como parâmetro o valor da variável de controlo do ciclo n=0,1,...
	// - chama a função exit passando como parâmetro o valor devolvido pela função empresas_executar
	// o processo pai guarda o pid do filho no vetor Ind.pid_empresas[n], com n=0,1,... e termina normalmente a função
    so_main_empresas(quant);
    //==============================================
}

int main(int argc, char *argv[])
{
    char *ficEntrada = NULL;
    char *ficSaida = NULL;
    char *ficLog = NULL;
    long intervalo = 0;

    //==============================================
    // TRATAR PARÂMETROS DE ENTRADA
    // parâmetro obrigatório: file_configuracao
    // parâmetros opcionais:
    //   file_resultados
    //   -l file_log
    //   -t intervalo(us)    // us: microsegundos
    //
	// ignorar parâmetros desconhecidos
	// em caso de ausência de parâmetros escrever mensagem de como utilizar o programa e terminar
	// considerar que os parâmetros apenas são introduzidos na ordem indicada pela mensagem
	// considerar que são sempre introduzidos valores válidos para os parâmetros
    intervalo = so_main_args(argc, argv, &ficEntrada, &ficSaida, &ficLog);
    //==============================================

    printf(
	"\n------------------------------------------------------------------------");
    printf(
	"\n----------------------------- SO_PerPro ------------------------------");
    printf(
	"\n------------------------------------------------------------------------\n");

    // Ler dados de entrada
    file_begin(ficEntrada, ficSaida, ficLog);
    // criar zonas de memória e semáforos
    memory_create_buffers();
    prodcons_create_buffers();
    control_create();

    // Criar estruturas para indicadores e guardar capacidade inicial de servicos
    memory_create_statistics();

    printf("\n\t\t\t*** Open SO_PerPro ***\n\n");
    control_open_soperpro();

    // Registar início de operação e armar alarme
    time_begin(intervalo);

    //
    // Iniciar sistema
    //

    // Criar INTERMEDIARIOS
    main_intermediario(Config.INTERMEDIARIO);
    // Criar EMPRESAS
    main_empresas(Config.EMPRESAS);
    // Criar CLIENTES
    main_cliente(Config.CLIENTES);

    //==============================================
    // ESPERAR PELA TERMINAÇÃO DOS CLIENTES E ATUALIZAR ESTATISTICAS
    //
    // esperar por cada cliente individualmente
	// se o cliente terminou normalmente
	// então testar se o valor do status é igual a SERVICOS
    //   se for igual entao nao atualizar as estatisticas 
    //   se for inferior entao incrementar o indicador da respetiva servico obtida
    // Ind.serviços_recebidos_por_clientes[SERVICO], n=0,1,...
    so_main_wait_clientes();
    //==============================================

    // Desarmar alarme
    time_destroy(intervalo);

    printf("\n\t\t\t*** Close SO_PerPro ***\n\n");
    control_close_soperpro();

    //==============================================
    // ESPERAR PELA TERMINAÇÃO DOS INTERMEDIARIOS E ATUALIZAR INDICADORES
    //
    // esperar por cada intermediario individualmente
	// se a intermediario terminou normalmente
	// então atualizar o indicador de investors atendidos
    // Ind.clientes_servidos_por_intermediarios[n], n=0,1,...
    // guardando nele o estado devolvido pela terminação do processo
    so_main_wait_intermediarios();
    //==============================================

    //==============================================
    // ESPERAR PELA TERMINAÇÃO DAS EMPRESAS E ATUALIZAR INDICADORES
    //
    // esperar por cada empresa individualmente
	// se a empresa terminou normalmente
	// então atualizar o indicador de investors atendidos
    // Ind.clientes_servidos_por_empresas[n], n=0,1,...
    // guardando nele o estado devolvido pela terminação do processo
    so_main_wait_empresas();
    //==============================================

    printf(
	"------------------------------------------------------------------------\n\n");
    printf("\t\t\t*** Statistics ***\n\n");
    so_write_statistics();
    printf("\t\t\t*******************\n");

    // destruir zonas de memória e semáforos
    file_destroy();
    control_destroy();
    prodcons_destroy();
    //memory_destroy_all();

    return 0;
}
