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
#include "memory.h"
#include "sotime.h"
#include "empresa.h"

extern struct statistics Ind;
extern struct configuration Config;

int empresa_executar(int id) {
	struct service Produto;
	int clientes_servidos_por_empresas = 0;

	setbuf(stdout, NULL);

	// Preparar servicos
	while (1) {
		// aguardar pedido de intermediario
		if (memory_request_b_read(id, &Produto) == 0)
			break;
		// preparar servico e aguardar processamento de orcamento
		time_processing_order();
		// colocar proposta
		memory_response_p_write(id, &Produto);
		Ind.propostas_entregues_por_empresas[Produto.id * Config.EMPRESAS + id]++;
		clientes_servidos_por_empresas++;
	}
	//    printf("EMPRESA %02d terminou\n",id);
	return clientes_servidos_por_empresas;
}
