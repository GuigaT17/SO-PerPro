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
#include "scheduler.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuration Config;
//==============================================

struct response_p BProposta;  	// buffer empresa-cliente (response_proposal)
struct request_b BOrcamento; 	// buffer intermediario-empresa (request_bugget)
struct request_d BDescricao; 	// buffer cliente-intermediario (request_description)

//******************************************
// CRIAR ZONAS DE MEMORIA
//
void * memory_create(char * name, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_open
	// usar tambem: ftruncate e mmap
	return so_memory_create(name, size);
	//==============================================
}
void memory_create_capacidade_servicos() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA A CAPACIDADE DE SERVICOS
	//
	// utilizar a função genérica memory_create(char *,int)
	// para criar ponteiro que se guarda em Config.capacidade_servicos
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [SERVICOS] para inteiro
	so_memory_create_capacidade_servicos();
	//==============================================
}
void memory_create_buffers() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS BUFFERS: DESCRICAO, ORCAMENTO e PROPOSTAS
	//
	// utilizar a função genérica memory_create(char *,int)
	//
	// para criar ponteiro que se guarda em BProposta.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_PROPOSTA]
	// para criar ponteiro que se guarda em BProposta.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_PROPOSTA] para struct service
	//
	// para criar ponteiro que se guarda em BOrcamento.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_ORCAMENTO]
	// para criar ponteiro que se guarda em BOrcamento.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_ORCAMENTO] para struct service
	//
	// para criar ponteiro que se guarda em BDescricao.ptr
	// que deve ter capacidade para struct pointers
	// para criar ponteiro que se guarda em BDescricao.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_DESCRICAO] para struct service
	so_memory_create_buffers();
	//==============================================
}
void memory_create_scheduler() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O MAPA DE ESCALONAMENTO
	//
	// utilizar a função genérica memory_create(char *,int)
	// para criar ponteiro que se guarda em Schedule.ptr
	// que deve ter capacidade para um vetor bidimensional
	// com a dimensao [SERVICOS,EMPRESAS] para inteiro
	so_memory_create_scheduler();
	//==============================================
}

void memory_destroy(char * name, void * ptr, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_unlink
	// usar tambem: munmap
	so_memory_destroy(name, ptr, size);
	//==============================================
}

//******************************************
// MEMORIA_DESTRUIR
//
void memory_destroy_all() {
	//==============================================
	// DESTRUIR MAPEAMENTO E NOME DE PÁGINAS DE MEMÓRIA
	//
	// utilizar a função genérica memory_destroy(char *,void *,int)
	so_memory_destroy_all();
	//==============================================
}

//******************************************
// MEMORIA_REQUEST_D_ESCREVE
//
void memory_request_d_write(int id, struct service *pProduto) {
	prodcons_request_d_produce_begin();

	// registar hora do pedido de servico
	time_register(&pProduto->time_descricao);

	//==============================================
	// ESCREVER DESCRICAO DE PROJETO PESSOAL NO BUFFER DESCRICOES
	//
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BDescricoes.ptr->in do buffer BDescricoes
	// conteudo: cliente, id, time_descricao
	// colocar disponibilidade = 1 nessa posicao do BDescricoes
	// e atualizar BDescricoes.ptr->in
	so_memory_request_d_write(id, pProduto);
	//==============================================

	prodcons_request_d_produce_end();

	// informar INTERMEDIARIO de pedido de SERVICO
	control_cliente_submete_descricao(id);

	// log
	file_write_log_file(1, id);
}
//******************************************
// MEMORIA_REQUEST_D_LE
//
int memory_request_d_read(int id, struct service *pProduto) {
	// testar se existem clientes e se o SO_PERPRO esta open
	if (control_intermediario_esperapor_descricao(id) == 0)
		return 0;

	prodcons_request_d_consume_begin();

	//==============================================
	// LER DESCRICAO DO BUFFER DE DESCRICOES
	//
	// copiar conteudo relevante da posicao BDescricao.ptr->out
	// do buffer BDescricao para a estrutura pProduto
	// conteudo: cliente, id, time_descricao
	// colocar available = 0 nessa posicao do BDescricao
	// e atualizar BDescricao.ptr->out
	so_memory_request_d_read(id, pProduto);
	//==============================================

	// testar se existe capacidade de servico para servir cliente
	if (prodcons_update_capacidade_servico(pProduto->id) == 0) {
		pProduto->disponibilidade = 0;
		prodcons_request_d_consume_end();
		return 2;
	} else
		pProduto->disponibilidade = 1;

	prodcons_request_d_consume_end();

	// log
	file_write_log_file(2, id);

	return 1;
}

//******************************************
// MEMORIA_REQUEST_B_ESCREVE
//
void memory_request_b_write(int id, struct service *pProduto) {
	int pos, empresa;

	prodcons_request_b_produce_begin();

	// decidir a que empresa se destina
	empresa = scheduler_get_empresa(id, pProduto->id);

	//==============================================
	// ESCREVER ORCAMENTO NO BUFFER DE PEDIDOS DE ORCAMENTOS
	//
	// procurar posicao vazia no buffer BOrcamento
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BOrcamento.ptr do buffer BOrcamento
	// conteudo: cliente, id, disponibilidade, intermediario, empresa, time_descricao
	// colocar BOrcamento.ptr[n] = 1 na posicao respetiva
	pos = so_memory_request_b_write(id, pProduto, empresa);
	//==============================================

	prodcons_request_b_produce_end();

	// informar empresa respetiva de pedido de orcamento
	control_intermediario_submete_orcamento(empresa);

	// registar hora pedido (orcamento)
	time_register(&BProposta.buffer[pos].time_orcamento);

	// log
	file_write_log_file(3, id);
}
//******************************************
// MEMORIA_REQUEST_B_LE
//
int memory_request_b_read(int id, struct service *pProduto) {
	// testar se existem pedidos e se o SO_PerPro esta open
	if (control_empresa_esperapor_orcamento(id) == 0)
		return 0;

	prodcons_request_b_consume_begin();

	//==============================================
	// LER PEDIDO DO BUFFER DE PEDIDOS DE ORCAMENTOS
	//
	// procurar pedido de orcamento para a empresa no buffer BOrcamento
	// copiar conteudo relevante da posicao encontrada
	// no buffer BOrcamento para pProduto
	// conteudo: cliente, id, disponibilidade, intermediario, time_descricao, time_orcamento
	// colocar BOrcamento.ptr[n] = 0 na posicao respetiva
	so_memory_request_b_read(id, pProduto);
	//==============================================

	prodcons_request_b_consume_end();

	// log
	file_write_log_file(4, id);

	return 1;
}

//******************************************
// MEMORIA_RESPONSE_P_ESCREVE
//
void memory_response_p_write(int id, struct service *pProduto) {
	int pos;

	prodcons_response_p_produce_begin();

	//==============================================
	// ESCREVER PROPOSTA NO BUFFER DE PROPOSTAS
	//
	// procurar posicao vazia no buffer BProposta
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BProposta.ptr do buffer BProposta
	// conteudo: cliente, id, disponibilidade, intermediario, empresa, time_descricao, time_orcamento
	// colocar BProposta.ptr[n] = 1 na posicao respetiva
	pos = so_memory_response_p_write(id, pProduto);
	//==============================================

	prodcons_response_p_produce_end();

	// informar cliente de que a proposta esta pronta
	control_empresa_submete_proposta(pProduto->cliente);

	// registar hora pronta (proposta)
	time_register(&BProposta.buffer[pos].time_proposta);

	// log
	file_write_log_file(5, id);
}
//******************************************
// MEMORIA_RESPONSE_P_LE
//
void memory_response_p_read(int id, struct service *pProduto) {
	// aguardar proposta
	control_cliente_esperapor_proposta(pProduto->cliente);

	prodcons_response_p_consume_begin();

	//==============================================
	// LER PROPOSTA DO BUFFER DE PROPOSTAS
	//
	// procurar proposta para o cliente no buffer BProposta
	// copiar conteudo relevante da posicao encontrada
	// no buffer BProposta para pProduto
	// conteudo: cliente, disponibilidade, intermediario, empresa, time_descricao, time_orcamento, time_proposta
	// colocar BProposta.ptr[n] = 0 na posicao respetiva
	so_memory_response_p_read(id, pProduto);
	//==============================================

	prodcons_response_p_consume_end();

	// log
	file_write_log_file(6, id);
}

//******************************************
// MEMORIA_CRIAR_INDICADORES
//
void memory_create_statistics() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS INDICADORES
	//
	// criação dinâmica de memória
	// para cada campo da estrutura indicadores
	so_memory_create_statistics();
	// iniciar indicadores relevantes:
	// - Ind.capacidade_inicial_servicos (c/ Config.capacidade_servico respetivo)
	// - Ind.clientes_servidos_por_intermediarios (c/ 0)
	// - Ind.clientes_servidos_por_empresas (c/ 0)
	// - Ind.serviços_recebidos_por_clientes (c/ 0)
	so_memory_prepare_statistics();
	//==============================================
}

