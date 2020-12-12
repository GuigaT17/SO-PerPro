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
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "prodcons.h"
#include "control.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct prodcons ProdCons;

//******************************************
// SEMAFORO_CRIAR
//
sem_t * semaphore_create(char * name, int value) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE UM SEMÁFORO
	int i = getuid();
	char name_uid[120];
	sprintf(name_uid,"/%s_%d", name, i);

	sem_unlink(name_uid);

	sem_t *sem_full = sem_open(name_uid, O_CREAT, 0xFFFFFFFF, value);
	if (sem_full == SEM_FAILED){
    perror("Error: criacao sem_full");
		exit(1);
	}
	return sem_full;
	//return so_semaphore_create(name, value);
	//==============================================
}

void prodcons_create_capacidade_servico() {
	//==============================================
	// CRIAR MUTEX PARA CONTROLAR O ACESSO A CAPACIDADE_SERVICOO
	ProdCons.stock_mutex = semaphore_create(STR_SEM_CAPACIDADE_SERVICO_MUTEX, 1);
	//so_prodcons_create_capacidade_servicos();
	//==============================================
}

void prodcons_create_buffers() {
	//==============================================
	// CRIAR SEMAFOROS PARA CONTROLAR O ACESSO AOS 3 BUFFERS

	ProdCons.request_d_mutex = semaphore_create(STR_SEM_DESCRICAO_MUTEX, 1);
	ProdCons.request_d_full = semaphore_create(STR_SEM_DESCRICAO_FULL, 0);
	ProdCons.request_d_empty = semaphore_create(STR_SEM_DESCRICAO_EMPTY, Config.BUFFER_DESCRICAO);

	ProdCons.request_b_mutex = semaphore_create(STR_SEM_ORCAMENTO_MUTEX, 1);
	ProdCons.request_b_full = semaphore_create(STR_SEM_ORCAMENTO_FULL, 0);
	ProdCons.request_b_empty = semaphore_create(STR_SEM_ORCAMENTO_EMPTY, Config.BUFFER_ORCAMENTO);

	ProdCons.response_p_mutex = semaphore_create(STR_SEM_PROPOSTA_MUTEX, 1);
	ProdCons.response_p_full = semaphore_create(STR_SEM_PROPOSTA_FULL, 0);
	ProdCons.response_p_empty = semaphore_create(STR_SEM_PROPOSTA_EMPTY, Config.BUFFER_PROPOSTA);
//	so_prodcons_create_buffers();
	//==============================================
}

void semaphore_destroy(char * name, void * ptr) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE UM SEMÁFORO
	int i = getuid();
	char name_uid[120];
	sprintf(name_uid,"/%s_%d", name, i);

	sem_close(ptr);
	sem_unlink(name_uid);
	//so_semaphore_destroy(name, ptr);
	//==============================================
}

void prodcons_destroy() {
	//==============================================
	// DESTRUIR SEMÁFORO E RESPETIVO NOME
	semaphore_destroy(STR_SEM_CAPACIDADE_SERVICO_MUTEX, ProdCons.stock_mutex);

	semaphore_destroy(STR_SEM_DESCRICAO_MUTEX, ProdCons.request_d_mutex);
	semaphore_destroy(STR_SEM_DESCRICAO_FULL, ProdCons.request_d_full);
	semaphore_destroy(STR_SEM_DESCRICAO_EMPTY, ProdCons.request_d_empty);

	semaphore_destroy(STR_SEM_PROPOSTA_MUTEX, ProdCons.request_b_mutex);
	semaphore_destroy(STR_SEM_PROPOSTA_FULL, ProdCons.request_b_full);
	semaphore_destroy(STR_SEM_PROPOSTA_EMPTY, ProdCons.request_b_empty);

	semaphore_destroy(STR_SEM_ORCAMENTO_MUTEX, ProdCons.response_p_mutex);
	semaphore_destroy(STR_SEM_ORCAMENTO_FULL, ProdCons.response_p_full);
	semaphore_destroy(STR_SEM_ORCAMENTO_EMPTY, ProdCons.response_p_empty);

	//so_prodcons_destroy();
	//==============================================
}

//******************************************
void prodcons_request_d_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	if (sem_wait(ProdCons.request_d_empty) == -1){
		perror("Error: wait request_d_empty");
		exit(1);
	}

	if (sem_wait(ProdCons.request_d_mutex) == -1){
		perror("Error: wait request_d_mutex");
		exit(2);
	}
	//so_prodcons_request_p_produce_begin();
	//==============================================
}

//******************************************
void prodcons_request_d_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER DESCRICAO
	if (sem_post(ProdCons.request_d_full) == -1){
		perror("Error: post request_d_full");
		exit(1);
	}

	if (sem_post(ProdCons.request_d_mutex) == -1){
		perror("Error: post request_d_mutex");
		exit(2);
	}
	//so_prodcons_request_p_produce_end();
	//==============================================
}

//******************************************
void prodcons_request_d_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	if (sem_wait(ProdCons.request_d_full) == -1){
		perror("Error: wait request_d_full");
		exit(1);
	}

	if (sem_wait(ProdCons.request_d_mutex) == -1){
		perror("Error: wait request_d_mutex");
		exit(2);
	}
  //so_prodcons_request_p_consume_begin();
	//==============================================
}

//******************************************
void prodcons_request_d_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER DESCRICAO
	if (sem_post(ProdCons.request_d_mutex) == -1){
		perror("Error: post request_d_mutex");
		exit(1);
	}

	if (sem_post(ProdCons.request_d_empty) == -1){
		perror("Error: post request_d_empty");
		exit(2);
	}
	//so_prodcons_request_p_consume_end();
	//==============================================
}

//******************************************
void prodcons_request_b_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER ORCAMENTO
	if(sem_wait(ProdCons.request_b_empty) == -1){
		perror("Error: post request_b_empty");
		exit(1);
	}
	if(sem_wait(ProdCons.request_b_mutex) == -1){
		perror("Error: wait request_b_mutex");
		exit(2);
	}
	//so_prodcons_request_e_produce_begin();
	//==============================================
}

//******************************************
void prodcons_request_b_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER ORCAMENTO
	if(sem_post(ProdCons.request_b_mutex) == -1){
		perror("Error: post request_b mutex");
		exit(1);
	}
	if(sem_post(ProdCons.request_b_full) == -1){
		perror("Error: post request_b full");
		exit(2);
	}
	//so_prodcons_request_e_produce_end();
	//==============================================
}

//******************************************
void prodcons_request_b_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER ORCAMENTO
	if(sem_wait(ProdCons.request_b_full) == -1){
		perror("Error: wait request_b full");
		exit(1);
	}
	if(sem_wait(ProdCons.request_b_mutex) == -1){
		perror("Error: wait request_b mutex");
		exit(2);
	}
	//so_prodcons_request_e_consume_begin();
	//==============================================
}

//******************************************
void prodcons_request_b_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER ORCAMENTO

	if(sem_post(ProdCons.request_b_mutex) == -1){
		perror("Error: post request_b mutex");
		exit(1);
	}
	if(sem_post(ProdCons.request_b_empty) == -1){
		perror("Error: post request_b empty");
		exit(2);
	}

	//so_prodcons_request_b_consume_end();
	//==============================================
}

//******************************************
void prodcons_response_p_produce_begin() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PROPOSTA

	if(sem_wait(ProdCons.response_p_empty) == -1){
		perror("Error: post response_p_empty");
		exit(1);
	}
	if(sem_wait(ProdCons.response_p_mutex) == -1){
		perror("Error: wait response_p_mutex");
		exit(2);
	}

	//so_prodcons_response_p_produce_begin();
	//==============================================
}

//******************************************
void prodcons_response_p_produce_end() {
	//==============================================
	// CONTROLAR ACESSO AO BUFFER PROPOSTA

	if(sem_post(ProdCons.response_p_mutex) == -1){
		perror("Error: post response_p mutex");
		exit(1);
	}
	if(sem_post(ProdCons.response_p_full) == -1){
		perror("Error: post response_p full");
		exit(2);
	}

//	so_prodcons_response_p_produce_end();
	//==============================================
}

//******************************************
void prodcons_response_p_consume_begin() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PROPOSTA

	if(sem_wait(ProdCons.response_p_full) == -1){
		perror("Error: wait response_p full");
		exit(1);
	}
	if(sem_wait(ProdCons.response_p_mutex) == -1){
		perror("Error: wait response_p mutex");
		exit(2);
	}

	//so_prodcons_response_p_consume_begin();;
	//==============================================
}

//******************************************
void prodcons_response_p_consume_end() {
	//==============================================
	// CONTROLAR ACESSO DE CONSUMIDOR AO BUFFER PROPOSTA

	if(sem_post(ProdCons.response_p_mutex) == -1){
		perror("Error: post response_p mutex");
		exit(1);
	}
	if(sem_post(ProdCons.response_p_empty) == -1){
		perror("Error: post response_p empty");
		exit(2);
	}

	//so_prodcons_response_p_consume_end();
	//==============================================
}

//******************************************
void prodcons_buffers_begin() {
	//==============================================
	// GARANTIR EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS

	if(sem_wait(ProdCons.request_d_mutex) == -1){
		perror("Error: wait request_d_mutex");
		exit(1);
	}
	if(sem_wait(ProdCons.request_b_mutex) == -1){
		perror("Error: wait request_b_mutex");
		exit(2);
	}
	if(sem_wait(ProdCons.response_p_mutex) == -1){
		perror("Error: wait response_p_mutex");
		exit(3);
	}

//	so_prodcons_buffers_begin();
	//==============================================
}

//******************************************
void prodcons_buffers_end() {
	//==============================================
	// FIM DA ZONA DE EXCLUSÃO MÚTUA NO ACESSO AOS 3 BUFFERS

	if(sem_post(ProdCons.request_d_mutex) == -1){
		perror("Error: post request_d_mutex");
		exit(1);
	}
	if(sem_post(ProdCons.request_b_mutex) == -1){
		perror("Error: post request_b_mutex");
		exit(2);
	}
	if(sem_post(ProdCons.response_p_mutex) == -1){
		perror("Error: post response_p_mutex");
		exit(3);
	}

	//so_prodcons_buffers_end();
	//==============================================
}

//******************************************
int prodcons_update_capacidade_servico(int servico) {
	//==============================================
	// OBTER MUTEX DA CAPACIDADE SERVICO E ATUALIZAR CAPACIDADE SERVICO

	int result = 0;

	if(sem_wait(ProdCons.stock_mutex) == -1){
		perror("Error: wait stock_mutex");
		exit(1);
	}

	if(Config.capacidade_servico[servico] > 0){
		Config.capacidade_servico[servico]--;
		result = 1;
	}

	if(sem_post(ProdCons.stock_mutex) == -1){
		perror("Error: post stock_mutex");
		exit(2);
	}
	return result;

	//return so_prodcons_update_capacidade_servico(servico);
	//==============================================
}
