CC=gcc
all: gestore individual_a individual_b
gestore: gestore.o population.o shm.o
	$(CC) -o gestore gestore.o population.o shm.o

individual_a: individual_a.o population.o shm.o
	$(CC) -o individual_a individual_a.o population.o shm.o

individual_b: individual_b.o population.o shm.o
	$(CC) -o individual_b individual_b.o population.o shm.o

individual_a.o: individual_a.c population.h shm.h
	$(CC) -c -o individual_a.o individual_a.c

individual_b.o: individual_b.c population.h shm.h
	$(CC) -c -o individual_b.o individual_b.c

gestore.o: gestore.c population.h shm.h
	$(CC) -c -o gestore.o gestore.c

population.o: population.c population.h
	$(CC) -c -o population.o population.c

shm.o: shm.c shm.h
	$(CC) -c -o shm.o shm.c
