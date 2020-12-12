/*
 @author Guilherme Teixeira 49021, Ines Goncalves 49493
*/
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
#include <ctype.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "memory.h"
#include "prodcons.h"
#include "file.h"
#include "sotime.h"
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
extern struct request_d BDescricao;
extern struct request_b BOrcamento;
extern struct response_p BProposta;
//==============================================

struct file Ficheiros; // informação sobre nomes e handles de ficheiros

void file_begin(char *fic_in, char *fic_out, char *fic_log) {
	//==============================================
	// GUARDAR NOMES DOS FICHEIROS NA ESTRUTURA Ficheiros

	Ficheiros.entrada = fic_in;
	Ficheiros.saida = fic_out;
	Ficheiros.log = fic_log;

//	so_file_begin(fic_in, fic_out, fic_log);
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE ENTRADA
	// em modo de leitura
	Ficheiros.h_in = fopen(Ficheiros.entrada, "r");
	//so_file_open_file_in();
	//==============================================

	// parse do ficheiro de teste
	// esta funcao prepara os campos da estrutura Config (char *)
	// com os dados do ficheiro de entrada
	if (ini_parse_file(Ficheiros.h_in, handler, &Config) < 0) {
		printf("Erro a carregar o ficheiro de teste'\n");
		exit(1);
	}

	// agora e' preciso inicializar os restantes campos da estrutura Config

	//==============================================
	// CONTAR SERVICOS
	// usar strtok para percorrer Config.list_servicos
	// guardar resultado em Config.SERVICOS

	int c = 0;
	char* copy_list_services = calloc(strlen(Config.list_servicos) + 1, sizeof(char));
	strcpy(copy_list_services, Config.list_servicos);

	char * tokenServices = strtok(Config.list_servicos, " ");


	while (tokenServices != NULL){
		c ++;
		tokenServices = strtok(NULL, " ");
	}
	Config.SERVICOS = c;
	//

	//so_file_count_servicos();
	//==============================================

	// iniciar memoria para o vetor com o capacidade por servico e semaforo
	memory_create_capacidade_servicos();
	prodcons_create_capacidade_servico();

	//==============================================
	// LER CAPACIDADE DE CADA SERVICO
	// percorrer Config.list_servicos e
	// guardar cada valor no vetor Config.capacidade_servicos

	int index = 0;
	char *tokenC = strtok(copy_list_services, " ");

	while (tokenC != NULL){
		Config.capacidade_servico[index] = atoi(tokenC);

		index++;
		tokenC = strtok(NULL, " ");
	}

	free(copy_list_services);

	//so_file_read_capacidade_servicos();
	//==============================================

	//==============================================
	// CONTAR CLIENTES
	// usar strtok para percorrer Config.list_clientes
	// guardar resultado em Config.CLIENTES

	char * tokenClientes = strtok(Config.list_clientes, " ");
	c = 0;
	while (tokenClientes != NULL){
		 c++;
		tokenClientes = strtok(NULL, " ");
	}
	Config.CLIENTES = c;

	//so_file_count_clientes();
	//==============================================

	//==============================================
	// CONTAR INTERMEDIARIOS
	// usar strtok para percorrer Config.list_intermediarios
	// guardar resultado em Config.INTERMEDIARIOS

	char * tokenIntermediarios = strtok(Config.list_intermediarios, " ");
	c = 0;
	while (tokenIntermediarios != NULL){
		c++;
		tokenIntermediarios = strtok(NULL, " ");
	}
	Config.INTERMEDIARIO = c;

	//so_file_count_intermediarios();
	//==============================================

	//==============================================
	// CONTAR EMPRESAS
	// usar strtok para percorrer Config.list_empresas
	// guardar resultado em Config.EMPRESAS

	char * tokenEmpresas = strtok(Config.list_empresas, ",");
	c = 0;
	while (tokenEmpresas != NULL){
		c++;
		tokenEmpresas = strtok (NULL, ",");
	}
	Config.EMPRESAS = c;

	//so_file_count_empresas();
	//==============================================

	so_file_read_servicos();

	//==============================================
	// LER CAPACIDADES DOS BUFFERS
	// usar strtok para percorrer Config.list_buffers
	// guardar primeiro tamanho em Config.BUFFER_DESCRICAO
	// guardar segundo tamanho em Config.BUFFER_ORCAMENTO
	// guardar terceiro tamanho em Config.BUFFER_PROPOSTA

	char * tokenCapacity;
	tokenCapacity = strtok(Config.list_buffers, " ");
	Config.BUFFER_DESCRICAO = atoi(tokenCapacity);
	tokenCapacity = strtok(NULL, " ");
	Config.BUFFER_ORCAMENTO = atoi(tokenCapacity);
	tokenCapacity = strtok(NULL, " ");
	Config.BUFFER_PROPOSTA = atoi(tokenCapacity);

	//so_file_read_capacidade_buffer();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE SAIDA (se foi especificado)
	// em modo de escrita

	if(Ficheiros.saida != NULL)
		Ficheiros.h_out = fopen(Ficheiros.saida, "w");

//	so_file_open_file_out();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE LOG (se foi especificado)
	// em modo de escrita

	if (Ficheiros.log != NULL)
		Ficheiros.h_log = fopen(Ficheiros.log, "w");

	//so_file_open_file_log();
	//==============================================
}

void file_destroy() {
	//==============================================
	// LIBERTAR ZONAS DE MEMÓRIA RESERVADAS DINAMICAMENTE
	// que podem ser: Ficheiros.entrada, Ficheiros.saida, Ficheiros.log

	fclose(Ficheiros.h_in);
	fclose(Ficheiros.h_out);
	fclose(Ficheiros.h_log);

	//so_file_destroy();
	//==============================================
}

void file_write_log_file(int etapa, int id) {
	double t_diff;

	if (Ficheiros.h_log != NULL) {

		prodcons_buffers_begin();

		// guardar timestamp
		t_diff = time_untilnow();
		int sep = 0xFFFFFFFF;
		int zero = 0x00000000;
		//==============================================
		// ESCREVER DADOS NO FICHEIRO DE LOG
		//REGISTO
		for(int i = 0; i < 4; i++){
			if(fwrite(&sep, sizeof(int), 1, Ficheiros.h_log) == EOF){
				perror("Error: log write");
			}
		}
		//TIME
		if(fwrite(&t_diff, sizeof(double), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		//ETAPA
		if(fwrite(&etapa, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		//ID
		if(fwrite(&id, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		//BUFER DESCRICAO
		for(int i = 0; i < Config.BUFFER_DESCRICAO; i++){
			if(BDescricao.buffer[i].disponibilidade == 0){
				if(fwrite(&BDescricao.buffer[i].id, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BDescricao.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BDescricao.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BDescricao.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
			}
		}
		//sep
		if(fwrite(&sep, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&sep, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&zero, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&zero, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}

		//BUFFER ORCAMENTO
		for(int i = 0; i < Config.BUFFER_ORCAMENTO; i++){
			if(BOrcamento.buffer[i].disponibilidade == 0){
				if(fwrite(&BOrcamento.buffer[i].id, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}

				if(fwrite(&BOrcamento.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BOrcamento.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BOrcamento.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
			}
		}
		//sep
		if(fwrite(&sep, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&sep, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&zero, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		if(fwrite(&zero, sizeof(int), 1, Ficheiros.h_log) == EOF){
			perror("Error: log write");
		}
		//BUFFER PROPOSTA
		for(int i = 0; i < Config.BUFFER_PROPOSTA; i++){
			if(BProposta.buffer[i].disponibilidade == 0){
				if(fwrite(&BProposta.buffer[i].id, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}

				if(fwrite(&BProposta.buffer[i].cliente, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BProposta.buffer[i].intermediario, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
				if(fwrite(&BProposta.buffer[i].empresa, sizeof(int), 1, Ficheiros.h_log) == EOF){
					perror("Error: log write");
				}
			}
		}
		// o log deve seguir escrupulosamente o formato definido
		//so_file_write_log_file(etapa, id, t_diff);
		//==============================================

		prodcons_buffers_end();
	}
}

void file_write_line(char * linha) {
	//==============================================
	// ESCREVER UMA LINHA NO FICHEIRO DE SAIDA

	if(fputs (linha, Ficheiros.h_out) == EOF ){
		perror("Error: file_write_line");
	}

	//so_file_write_line(linha);
	//==============================================
}

int stricmp(const char *s1, const char *s2) {
	if (s1 == NULL)
		return s2 == NULL ? 0 : -(*s2);
	if (s2 == NULL)
		return *s1;

	char c1, c2;
	while ((c1 = tolower(*s1)) == (c2 = tolower(*s2))) {
		if (*s1 == '\0')
			break;
		++s1;
		++s2;
	}

	return c1 - c2;
}

int handler(void* user, const char* section, const char* name,
		const char* value) {
	struct configuration* pconfig = (struct configuration*) user;

#define MATCH(s, n) stricmp(section, s) == 0 && stricmp(name, n) == 0
	if (MATCH("servicos", "capacidade_servicos")) {
		pconfig->list_servicos = strdup(value);
	} else if (MATCH("clientes", "servico")) {
		pconfig->list_clientes = strdup(value);
	} else if (MATCH("intermediarios", "list")) {
		pconfig->list_intermediarios = strdup(value);
	} else if (MATCH("empresas", "servicos")) {
		pconfig->list_empresas = strdup(value);
	} else if (MATCH("buffers", "capacidade_buffer")) {
		pconfig->list_buffers = strdup(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}
