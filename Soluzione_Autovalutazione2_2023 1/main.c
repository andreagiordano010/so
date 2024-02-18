#include "prodcons.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void Produttore(MonitorStreaming *p);
void Consumatore(MonitorStreaming *p);

int main() {

    int id_shm = shmget(IPC_PRIVATE, sizeof(MonitorStreaming), IPC_CREAT | 0664);

    if(id_shm < 0) {
        perror("Errore shmget");
        exit(1);
    }


    MonitorStreaming * p = shmat(id_shm, NULL, 0);

    if(p == (void*)-1) {
        perror("Errore shmat");
        exit(1);
    }

    inizializza(p);

    srand(time(NULL));


    pid_t pid = fork();

    if(pid == 0) {
        Produttore(p);
        exit(0);
    }


    pid = fork();

    if(pid == 0) {
        Consumatore(p);
        exit(0);
    }

    wait(NULL);
    wait(NULL);

    distruggi(p);

    shmctl(id_shm, IPC_RMID, 0);
}

void Produttore(MonitorStreaming *p) {

    char stringa[20];
    size_t lunghezza;

    char char_chiave = 'a';
    key_t chiave;

    for(int i=0; i<10; i++) {

        lunghezza = 1 + rand() % 20;

        for(int j=0; j<lunghezza-1; j++) {
            stringa[j] = 97 + (rand()%26);
        }

        stringa[lunghezza-1] = '\0';


        chiave = ftok(".", char_chiave);

        char_chiave = char_chiave + 1;


        produci(p, stringa, lunghezza, chiave);

        sleep(1);
    }
}

void Consumatore(MonitorStreaming *p) {

    char stringa[20];
    size_t lunghezza;

    for(int i=0; i<10; i++) {

        consuma(p, stringa, &lunghezza);

        sleep(1);
    }
}