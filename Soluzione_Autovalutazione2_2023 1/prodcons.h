#ifndef _HEADER_H_
#define _HEADER_H_

#include "monitor_hoare.h"

#include <sys/types.h>

#define LIBERO 0
#define OCCUPATO 1
#define IN_USO 2

typedef struct {

    int stato;      // LIBERO, OCCUPATO, IN_USO
    pid_t produttore;
    size_t dimensione;
    key_t chiave;

} Buffer;

#define CV_PROD 0
#define CV_CONS 1

#define DIM 4

typedef struct {

    Monitor m;
    Buffer vettore[DIM];

    int buffer_liberi;
    int buffer_occupati;

} MonitorStreaming;

void inizializza(MonitorStreaming * p);
void produci(MonitorStreaming * p, char * stringa, size_t lunghezza, key_t chiave);
void consuma(MonitorStreaming * p, char * stringa, size_t * lunghezza);
void distruggi(MonitorStreaming * p);


#endif