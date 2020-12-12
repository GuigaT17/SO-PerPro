#ifndef PRODCONS_H_GUARD
#define PRODCONS_H_GUARD

// Nomes usados na criacao dos semaforos
#define STR_SEM_CAPACIDADE_SERVICO_MUTEX	"sem_capacidade_servico_mutex"
#define STR_SEM_DESCRICAO_FULL 				"sem_descricao_full"
#define STR_SEM_DESCRICAO_EMPTY 			"sem_descricao_empty"
#define STR_SEM_DESCRICAO_MUTEX 			"sem_descricao_mutex"
#define STR_SEM_ORCAMENTO_FULL 				"sem_orcamento_full"
#define STR_SEM_ORCAMENTO_EMPTY 			"sem_orcamento_empty"
#define STR_SEM_ORCAMENTO_MUTEX 			"sem_orcamento_mutex"
#define STR_SEM_PROPOSTA_FULL 				"sem_proposta_full"
#define STR_SEM_PROPOSTA_EMPTY 				"sem_proposta_empty"
#define STR_SEM_PROPOSTA_MUTEX 				"sem_proposta_mutex"

struct prodcons {
	// semáforos de controlo do acesso ao buffer entre empresas e clientes
	sem_t *response_p_full, *response_p_empty, *response_p_mutex;
	// semáforos de controlo do acesso ao buffer entre intermediarios e empresas
	sem_t *request_b_full, *request_b_empty, *request_b_mutex;
	// semáforos de controlo do acesso ao buffer entre clientes e intermediarios
	sem_t *request_d_full, *request_d_empty, *request_d_mutex;
	// semaforo para exclusao mutua no acesso a capacidade de servicos
	sem_t *stock_mutex;
};

sem_t * semaphore_create(char*, int);
void prodcons_create_capacidade_servico();
void prodcons_create_buffers();
void prodcons_destroy();
void prodcons_request_d_produce_begin();
void prodcons_request_d_produce_end();
void prodcons_request_d_consume_begin();
void prodcons_request_d_consume_end();
void prodcons_request_b_produce_begin();
void prodcons_request_b_produce_end();
void prodcons_request_b_consume_begin();
void prodcons_request_b_consume_end();
void prodcons_response_p_produce_begin();
void prodcons_response_p_produce_end();
void prodcons_response_p_consume_begin();
void prodcons_response_p_consume_end();
void prodcons_buffers_begin();
void prodcons_buffers_end();
int prodcons_update_capacidade_servico(int);

#endif
