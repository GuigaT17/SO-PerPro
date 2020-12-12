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

#include "cliente.h"
#include "main.h"
#include "memory.h"
#include "file.h"
#include "sotime.h"

extern struct configuration Config;

int cliente_executar(int id) {
	int n, count, ret;
	struct service Produto;
	int currency_id;
	char buf[100];
	char *result;

	setbuf(stdout, NULL);

	n = 0;
	count = 0;
	result = Config.list_clientes;
	while (n < id) {
		while (Config.list_clientes[count++] != '\0')
			;
		result = &Config.list_clientes[count];
		n++;
	}

	currency_id = atoi(result);
	Produto.id = currency_id;
	Produto.cliente = id;

	memory_request_d_write(id, &Produto);
	memory_response_p_read(id, &Produto);

	if (Produto.disponibilidade == 1) {
		printf(
				"     CLIENTE %03d solicitou %d e obteve %d (v:%02d c:%02d t:%.9fs)\n",
				id, currency_id, Produto.id, Produto.intermediario, Produto.empresa,
				time_difference(Produto.time_descricao, Produto.time_proposta));
		sprintf(buf, "     CLIENTE %03d solicitou %d e obteve %d\n", id,
				currency_id, Produto.id);
		ret = Produto.id;
	} else {
		printf("     CLIENTE %03d solicitou %d mas nao estava disponivel!\n",
				id, currency_id);
		sprintf(buf,
				"     CLIENTE %03d solicitou %d mas nao estava disponivel!\n",
				id, currency_id);
		ret = Config.SERVICOS;
	}

	file_write_line(buf);

	return ret;
}
