BIN_dir = bin
OBJ_dir = obj
TEST_IN_dir = tests/in
TEST_OUT_dir = tests/out
TEST_LOG_dir = tests/log
OBJETOS = cliente.o control.o empresa.o file.o intermediario.o main.o memory.o prodcons.o scheduler.o time.o
CFLAGS = -Wall -I
LIBS = -lrt -lpthread

vpath %.o $(OBJ_DIR)

%.o: src/%.c $($@)
	gcc $(CFLAGS) include -o $(OBJ_dir)/$@ -c $<

soperpro: $(OBJETOS)
	gcc $(addprefix $(OBJ_dir)/,$(OBJETOS)) obj/so.o -o bin/soperpro

clean:
	rm -f $(addprefix obj/,$(OBJETOS))
	rm -f bin/soperpro

test:
	./$(BIN_dir)/soperpro $(TEST_IN_dir)/scenario1 $(TEST_OUT_dir)/scenario1 -l $(TEST_LOG_dir)/scenario1 -t 10000
	./$(BIN_dir)/soperpro $(TEST_IN_dir)/scenario2 $(TEST_OUT_dir)/scenario2 -l $(TEST_LOG_dir)/scenario2 -t 10000
	./$(BIN_dir)/soperpro $(TEST_IN_dir)/scenario3 $(TEST_OUT_dir)/scenario3 -l $(TEST_LOG_dir)/scenario3 -t 10000
	./$(BIN_dir)/soperpro $(TEST_IN_dir)/scenario4 $(TEST_OUT_dir)/scenario4 -l $(TEST_LOG_dir)/scenario4 -t 10000
