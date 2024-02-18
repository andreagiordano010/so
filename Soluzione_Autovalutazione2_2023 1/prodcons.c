#include "prodcons.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>


void inizializza(MonitorStreaming * p) {

    init_monitor(&p->m, 2);

    p->buffer_liberi = DIM;
    p->buffer_occupati = 0;

    for(int i=0; i<DIM; i++) {

        p->vettore[i].stato = LIBERO;
    }
}


void produci(MonitorStreaming * p, char * stringa, size_t lunghezza, key_t chiave) {

    enter_monitor(&p->m);

    if( p->buffer_liberi == 0 ) {

        wait_condition(&p->m, CV_PROD);
    }

    int i = 0;

    while(i<DIM && p->vettore[i].stato != LIBERO) {
        i++;
    }

    p->vettore[i].stato = IN_USO;
    p->buffer_liberi--;

    leave_monitor(&p->m);



    printf("Avvio produzione...\n");

    sleep(1);

    int id_shm = shmget(chiave, lunghezza, IPC_CREAT | 0664);

    if(id_shm < 0) {
        perror("Errore shmget produttore");
        exit(1);
    }

    char * stringa_shm = shmat(id_shm, NULL, 0);

    if(stringa_shm == (void *)-1) {
        perror("Errore shmat produttore");
        exit(1);
    }

    p->vettore[i].dimensione = lunghezza;
    p->vettore[i].chiave = chiave;
    p->vettore[i].produttore = getpid();

    strcpy(stringa_shm, stringa);

    printf("Produzione completata: %s (%zu char, key=%x)\n", stringa_shm, lunghezza, chiave);

    shmdt(stringa_shm);

    

    enter_monitor(&p->m);

    p->vettore[i].stato = OCCUPATO;
    p->buffer_occupati++;

    signal_condition(&p->m, CV_CONS);

    leave_monitor(&p->m);
}

void consuma(MonitorStreaming * p, char * stringa, size_t * lunghezza) {

    enter_monitor(&p->m);

    if( p->buffer_occupati == 0 ) {

        wait_condition(&p->m, CV_CONS);
    }

    int i = 0;
    while(i<DIM && p->vettore[i].stato != OCCUPATO) {
        i++;
    }

    p->vettore[i].stato = IN_USO;
    p->buffer_occupati--;

    leave_monitor(&p->m);



    printf("Avvio consumazione...\n");

    sleep(1);

    key_t chiave = p->vettore[i].chiave;
    pid_t produttore = p->vettore[i].produttore;
    *lunghezza = p->vettore[i].dimensione;



    int id_shm = shmget(chiave, *lunghezza, 0664);

    if(id_shm < 0) {
        perror("Errore shmget consumatore");
        exit(1);
    }

    char * stringa_shm = shmat(id_shm, NULL, 0);

    if(stringa_shm == (void *)-1) {
        perror("Errore shmat consumatore");
        exit(1);
    }

    strcpy(stringa, stringa_shm);


    shmdt(stringa_shm);
    shmctl(id_shm, IPC_RMID, 0);


    printf("Consumazione completata: %s (%zu char, key=%x)\n", stringa, *lunghezza, chiave);




    enter_monitor(&p->m);

    p->vettore[i].stato = LIBERO;
    p->buffer_liberi++;

    signal_condition(&p->m, CV_CONS);

    leave_monitor(&p->m);
}

void distruggi(MonitorStreaming * p) {

    remove_monitor(&p->m);
}