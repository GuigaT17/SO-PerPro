#ifndef SCHEDULER_H_GUARD
#define SCHEDULER_H_GUARD

// estrutura onde se encontram quais as empresas que podem fornecer cada servico (1 ou 0)
struct scheduler {
	int *ptr; // ponteiro para matriz [SERVICOS,EMPRESAS]
};

void scheduler_begin(int, int);
int scheduler_get_empresa(int, int);

#endif
