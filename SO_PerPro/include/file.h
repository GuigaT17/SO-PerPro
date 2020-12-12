#ifndef FILE_H_GUARD
#define FILE_H_GUARD

// estrutura de registo de nomes e handles para ficheiros
struct file {
	char *entrada;
	char *saida;
	char *log;
	FILE *h_in;
	FILE *h_out;
	FILE *h_log;
};

void file_begin(char *, char *, char *);
void file_destroy();
void file_write_log_file(int, int);
void file_write_line(char *);

int handler(void*, const char*, const char*, const char*);

#endif
