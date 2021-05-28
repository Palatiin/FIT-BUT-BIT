# Makefile
# Riesenie: IOS - Projekt 2, 2.5.2021
# Autor: Matus Remen (xremen01), FIT
# Preklad: gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
# Popis: Makefile k druhemu projektu z predmetu IOS

CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic -pthread
OBJ=proj2

$(OBJ): $(OBJ).o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ).o: $(OBJ).c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm *.o $(OBJ)

rmout:
	rm proj2.out

pack:
	zip proj2.zip *.c *.h Makefile

