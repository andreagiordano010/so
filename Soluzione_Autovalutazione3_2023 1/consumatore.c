#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include "header.h"


int main() {
	int pid;
	key_t msg_req_key = ftok(".",'a');
	key_t msg_res_key = ftok(".",'b');

	int queue_req = msgget(msg_req_key,0);
	int queue_res = msgget(msg_res_key,0);
	int i;
	for (i=0; i < NUM_CONS; i++) {
		req msg;
		msg.type = 1;
		printf("CONSUMATORE %d: invio richiesta \n",getpid());
		msgsnd(queue_req,&msg,sizeof(req)-sizeof(long),0);
		res risp;
		msgrcv(queue_res,&risp,sizeof(res)-sizeof(long),0,0);
		printf("CONSUMATORE %d: letto valore: %d\n",getpid(),risp.value);
		sleep(1);
	}

	msgctl(queue_req,IPC_RMID,0);
	msgctl(queue_res,IPC_RMID,0);

	return 0;
}
