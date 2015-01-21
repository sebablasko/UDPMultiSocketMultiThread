#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "ssocket.h"

//Definiciones
#define BUF_SIZE 10
#define FIRST_PORT 1820

//Variables
int first_pack = 0;
struct timeval dateInicio, dateFin;
pthread_mutex_t lock;
int mostrarInfo = 0;
int MAX_PACKS = 1;
int NTHREADS = 1;
int NSOCKETS = 1;
double segundos;


llamadaHilo(int socket_fd){
	char buf[BUF_SIZE];
	int lectura;

	if(mostrarInfo) printf("Socket Operativo: %d\n", socket_fd);

	int i;
	int paquetesParaAtender = MAX_PACKS/NTHREADS;

	for(i = 0; i < paquetesParaAtender; i++) {
		//lectura = recv(socket_fd, buf, BUF_SIZE, 0);
		lectura = read(socket_fd, buf, BUF_SIZE);
		if(lectura <= 0) {
			fprintf(stderr, "Error en el read del socket (%d)\n", lectura);
			exit(1);
		}
		if(first_pack==0) { 
			pthread_mutex_lock(&lock);
			if(first_pack == 0) {
				if(mostrarInfo)	printf("got first pack\n");
				first_pack = 1;
				//Medir Inicio
				gettimeofday(&dateInicio, NULL);
			}
			pthread_mutex_unlock(&lock);
		}
	}
}

int main(int argc, char **argv){
	//Verificar Parametros Entrada
	if(argc <4){
		fprintf(stderr,"Syntax Error: Esperado: ./mtServer MAX_PACKS NTHREADS NSOCKETS\n");
		exit(1);
	}

	//Recuperar total de paquetes a enviar
	MAX_PACKS = atoi(argv[1]);
	//Recuperar numero de Threads
	NTHREADS = atoi(argv[2]);
	//Recuperar numero de sockets a abrir
	NSOCKETS = atoi(argv[3]);

	if(NTHREADS < NSOCKETS){
		fprintf(stderr, "Error de diseño: No pueden haber menos Threads que sockets\n");
		exit(1);
	}
	if(mostrarInfo)	printf("Diseño: \n\t%d: Threads \n\t%d: Sockets\n\t%d: Paquetes a Enviar\n", NTHREADS, NSOCKETS, MAX_PACKS);

	//Definir Variables
	int sockets_fd[NSOCKETS];
	pthread_t pids[NTHREADS];
	int port = FIRST_PORT;
	char ports[10];
	//char thread_name[NTHREADS][NAMELEN];

	if(mostrarInfo)	printf("\n\tPuertos Activados:\n");
	int i;
	for(i = 0; i < NSOCKETS; i++) {
		sprintf(ports, "%d", port+i);
		if(mostrarInfo)	printf("\t\t %s\n", ports);
		sockets_fd[i] = udp_bind(ports);
		if(sockets_fd[i] < 0) {
			fprintf(stderr, "Error de bind al tomar el puerto\n");
			exit(1);
		}
	}

	pthread_mutex_init(&lock, NULL);

	//Lanzar Threads
	for(i=0; i < NTHREADS; i++) {
		int fd = sockets_fd[i%NSOCKETS];
		pthread_create(&pids[i], NULL, llamadaHilo, fd);
		//char thread_name[NAMELEN];
		//sprintf(thread_name, "serverThread_%d", i);
		//pthread_setname_np(pids[i], thread_name);
	}

	//Esperar Threads
	for(i=0; i < NTHREADS; i++) 
		pthread_join(pids[i], NULL);

	//Medir Fin
	gettimeofday(&dateFin, NULL);

	//Cerrar Sockets
	for(i=0; i < NSOCKETS; ++i)
		close(sockets_fd[i]);

	segundos=(dateFin.tv_sec*1.0+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
	if(mostrarInfo){
		printf("Tiempo Total = %g\n", segundos);
		printf("QPS = %g\n", MAX_PACKS*1.0/segundos);
	}else{
		printf("%g, \n", segundos);
	}
	exit(0);
}