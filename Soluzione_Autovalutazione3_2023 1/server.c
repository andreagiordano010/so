#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "header.h"

typedef struct {
    int buffer[MAX_VALUES];
    int testa;
    int coda;
    int num_elem;
    pthread_mutex_t mutex;
    pthread_cond_t spazio_disp;
    pthread_cond_t msg_disp;
} data_str;

typedef struct {
    int somma;
    int num_elem_sommati;
    pthread_mutex_t mutex;
    pthread_cond_t num_cons;
} somma_valori;

// VARIABILI GLOBALI:
// la struttura
data_str shared_buf;
// le code
int queue_req;
int queue_res;

int consuma() {
    int value;
    pthread_mutex_lock(&shared_buf.mutex);
    while (shared_buf.num_elem == 0) {
        pthread_cond_wait(&shared_buf.msg_disp,&shared_buf.mutex);
    }
    value = shared_buf.buffer[shared_buf.coda];
    shared_buf.coda = (shared_buf.coda++)%MAX_VALUES;
    shared_buf.num_elem--;

    pthread_cond_signal(&shared_buf.spazio_disp);
    pthread_mutex_unlock(&shared_buf.mutex);

    return value;
}

void produci(int new_value) {
    pthread_mutex_lock(&shared_buf.mutex);
    while (shared_buf.num_elem == MAX_VALUES) {
        pthread_cond_wait(&shared_buf.spazio_disp,&shared_buf.mutex);
    }

    shared_buf.buffer[shared_buf.testa] = new_value;
    shared_buf.testa = (shared_buf.testa++)%MAX_VALUES;
    shared_buf.num_elem++;

	pthread_cond_signal(&shared_buf.msg_disp);   
    pthread_mutex_unlock(&shared_buf.mutex);
}


void * stampa_somma(void* par) {
    
    somma_valori * s = (somma_valori *) par;
    
    pthread_mutex_lock(&s->mutex);
    while (s->num_elem_sommati < NUM_CONS ) {
        pthread_cond_wait(&s->num_cons,&s->mutex);
    }

    printf("STAMPA_SOMMA: La somma degli elementi consumati è: %d\n",s->somma);
        
    pthread_mutex_unlock(&shared_buf.mutex);

    pthread_exit(NULL);
}

void * produttore(void* par) {
    int new_value;
    int i;
    for (i = 0; i < NUM_UPDATES*MAX_VALUES; i++) {
        new_value = rand()%10+1;
        printf("PRODUTTORE: inserimento nuovo dato: %d\n",new_value);
        produci(new_value);
        sleep(rand()%3+1);
    }
    pthread_exit(NULL);
}

void * consumatore(void* par) {
    somma_valori * s = (somma_valori *) par;
    int i;
    for (i = 0; i < NUM_CONS; i++) {
        req msg;
        msgrcv(queue_req,&msg,sizeof(req)-sizeof(long),0,0);
        printf("CONSUMATORE_SERV: ricevuta richiesta di consumo\n");

        res risp;
        risp.type = 1;
        risp.value=consuma();
        printf("CONSUMATORE_SERV: invio valore al consumatore client %d\n",risp.value);
        msgsnd(queue_res,&risp,sizeof(res)-sizeof(long),0);

        pthread_mutex_lock(&s->mutex);
        s->somma+=risp.value;
        s->num_elem_sommati++;
        if (s->num_elem_sommati == NUM_CONS)
            pthread_cond_signal(&s->num_cons);
        pthread_mutex_unlock(&s->mutex);
    }
    

    pthread_exit(NULL);
}

int main() {

    key_t msg_req_key = ftok(".",'a');
	key_t msg_res_key = ftok(".",'b');

	queue_req = msgget(msg_req_key,0);
	queue_res = msgget(msg_res_key,0);

    pthread_mutex_init(&shared_buf.mutex,NULL);
    pthread_cond_init(&shared_buf.spazio_disp,NULL);
    pthread_cond_init(&shared_buf.msg_disp,NULL);
    shared_buf.testa = shared_buf.coda = 0;
    shared_buf.num_elem = 0;

    pthread_t prod, cons, sum;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    srand(time(NULL));

    somma_valori* p = malloc(sizeof(somma_valori));
    p->somma = 0;
    p->num_elem_sommati = 0;
    
    pthread_create(&prod,&attr,produttore,NULL);

    pthread_create(&sum,&attr,stampa_somma,p);

    pthread_create(&cons,&attr,consumatore,p);

    
    pthread_join(prod,NULL);
    pthread_join(sum,NULL);
    pthread_join(cons,NULL);

    msgctl(queue_req,IPC_RMID,0);
	msgctl(queue_res,IPC_RMID,0);
    free(p);

    return 0;
}
