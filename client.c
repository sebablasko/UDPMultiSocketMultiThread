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
char buf[BUF_SIZE];
char* IP_DEST;
int mostrarInfo = 0;
int NTHREADS = 1;
int MAX_PACKS = 1;
double segundos;

//Metodo para hilos
llamadaHilo(int socket_fd){
	int lectura;

	if(mostrarInfo)
		printf("Socket Operativo: %d\n", socket_fd);

	int i;
	int paquetesPorEnviar = MAX_PACKS/NTHREADS;

	for(i = 0; i < paquetesPorEnviar; i++){
		if(write(socket_fd, buf, BUF_SIZE) != BUF_SIZE) {
			gettimeofday(&dateFin, NULL);
			segundos = (dateFin.tv_sec+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
			fprintf(stderr, "Falla el write al servidor, envio %d paquetes\n", i);
			fprintf(stderr, "total time = %g\n", segundos);
			break;
		}
		if(first_pack==0) { 
			pthread_mutex_lock(&lock);
			if(first_pack == 0) {
				if(mostrarInfo) printf("got first pack\n");
				first_pack = 1;
				//Medir Inicio
				gettimeofday(&dateInicio, NULL);
			}
			pthread_mutex_unlock(&lock);
		}
	}
}

main(int argc, char **argv) {

	if(argc < 4){
		fprintf(stderr, "Syntax Error: Esperado: ./client MAX_PACKS NSOCKETS IP_DEST\n");
		exit(1);
	}

	//Recuperar total de paquetes a enviar
	MAX_PACKS = atoi(argv[1]);
	//Recuperar numero de Threads
	NTHREADS = atoi(argv[2]);
	//Recuperar IP destino
	IP_DEST = argv[3];

	//Variables
	int sockets_fd[NTHREADS];
	pthread_t pids[NTHREADS];
	int port = FIRST_PORT;
	char ports[10];

	/* Llenar de datos el buffer a enviar */
	int i;
	for(i=0; i < BUF_SIZE; i++)
		buf[i] = 'a'+i;

	if(mostrarInfo)	printf("Puertos Activados: \n");
	for(i = 0; i < NTHREADS; i++) {
		sprintf(ports, "%d", port+i);
		if(mostrarInfo)	printf("\t\t %s\n ", ports);
		sockets_fd[i] = udp_connect(IP_DEST, ports);
		if(sockets_fd[i] < 0) {
			fprintf(stderr, "connection refused\n");
			exit(1);
		}
	}

	pthread_mutex_init(&lock, NULL);
	if(mostrarInfo)	printf("Usando %d sockets, cada uno en un thread y enviando %d packs\n", NTHREADS, MAX_PACKS);

	//Lanzar Threads
	for(i=0; i < NTHREADS; i++) {
		int fd = sockets_fd[i];
		pthread_create(&pids[i], NULL, llamadaHilo, fd);
	}

	//Esperar Threads
	for(i=0; i < NTHREADS; i++) 
		pthread_join(pids[i], NULL);

	//Medir Fin
	gettimeofday(&dateFin, NULL);

	segundos=(dateFin.tv_sec*1.0+dateFin.tv_usec/1000000.)-(dateInicio.tv_sec*1.0+dateInicio.tv_usec/1000000.);
	if(mostrarInfo){
		printf("Tiempo Total = %g\n", segundos);
		printf("QPS = %g\n", MAX_PACKS*1.0/segundos);
	}else{
		printf("%i, %g \n", NTHREADS, segundos);
	}
	exit(0);
}