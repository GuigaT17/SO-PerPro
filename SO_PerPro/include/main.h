#ifndef MAIN_H_GUARD
#define MAIN_H_GUARD

// estrutura que representa um service nas tres etapas: descricao de projeto, pedido de orcamento e proposta
struct service {
	int id;  		            		// identificador do tipo de servico solicitado
	int disponibilidade;           		// servico: 0 - indisponivel, 1 - disponivel
	int cliente;     	            	// id do cliente que solicitou o servico
	int intermediario;         			// id do intermediario que atendeu o cliente
	int empresa;  	            		// id da empresa que efetuou a servico
	struct timespec time_descricao; 	// hora a que a servico foi pedido ao intermediario (registada pelo cliente)
	struct timespec time_orcamento;  	// hora a que o orcamento foi pedida a empresa (registada pela intermediario)
	struct timespec time_proposta; 		// hora a que a proposta foi elaborada pela empresa (registada pela empresa)
};

// estrutura com valores dos parâmetros do ficheiro de teste
struct configuration {
	char* list_servicos;      		// linha com capacidade de servico
	char* list_clientes;    		// linha com produp pretendido por cada cliente
	char* list_intermediarios;		// linha com os intermediarios (nomes nao interessam)
	char* list_empresas; 			// linha com os servicos por empresa separadas por virgula
	char* list_buffers;     		// linha com capacidade dos tres buffers
	int SERVICOS;            		// nº de servicos disponiveis
	int CLIENTES;          			// nº de clientes
	int INTERMEDIARIO;        		// nº de intermediario
	int EMPRESAS;       			// nº de empresa
	int BUFFER_DESCRICAO; 			// capacidade do buffer de pedidos de servico
	int BUFFER_ORCAMENTO;  			// capacidade do buffer de pedidos de orcamento
	int BUFFER_PROPOSTA;     		// capacidade do buffer de propostas
	int* capacidade_servico;             		// vetor com capacidade por tipo de service
};

// estrutura de registo de indicadores de funcionamento do SO_PerPro
struct statistics {
	int *capacidade_inicial_servicos;
	int *pid_clientes;
	int *pid_intermediarios;
	int *pid_empresas;
	int *clientes_servidos_por_intermediarios;
	int *clientes_servidos_por_empresas;
	int *propostas_entregues_por_empresas;
	int *servicos_recebidos_por_clientes;
};

#endif
