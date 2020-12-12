#ifndef CONTROL_H_GUARD
#define CONTROL_H_GUARD

// estrutura de condições para controlar o encerramento do SO_Currency
struct control {
	pthread_mutex_t intermediario_mutex;
	pthread_cond_t  intermediario_cond;
	pthread_mutex_t empresas_mutex;
	pthread_cond_t  empresas_cond;
	// semaforo para o cliente informar os intermediarios de uma descricao
	sem_t *intermediario_request;
	// semaforos para a intermediario informar a empresa sobre um orcamento
	sem_t **empresa_request;
	// semaforos para a empresa informar o cliente sobre uma proposta
	sem_t **cliente_response;
	int open;  // 0: se SO_Currency closed, 1: se SO_Currency open
};

void control_create();
void control_destroy();
void control_lock();
void control_unlock();
void control_waitfor_request();
void control_wakeup_empresas();
void control_open_soperpro();
void control_close_soperpro();
void cond_create(pthread_cond_t *cond, pthread_mutex_t *mutex);

void control_cliente_submete_descricao(int);
int  control_intermediario_esperapor_descricao(int);
void control_intermediario_submete_orcamento(int);
int  control_empresa_esperapor_orcamento(int);
void control_empresa_submete_proposta(int);
void control_cliente_esperapor_proposta(int);

#endif
