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
#include "intermediario.h"

int intermediario_executar(int id) {
	int ret;
	struct service Produto;
	int clientes_servidos_por_intermediarios = 0;

	setbuf(stdout, NULL);

	// Passar descricao de projeto pessoal para a empresa
	while (1) {
		ret = memory_request_d_read(id, &Produto);
		if (ret == 1) {
			// servico disponível
			memory_request_b_write(id, &Produto);
		} else if (ret == 2) {
			// servico indisponível
			memory_response_p_write(id, &Produto);
		} else
			// sem clientes
			break;
		clientes_servidos_por_intermediarios++;
	}

	return clientes_servidos_por_intermediarios;
}
