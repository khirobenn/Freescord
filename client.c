#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include "buffer/buffer.h"
#include "utils.h"
#include <string.h>

#define PORT_FREESCORD 4321
#define BUFF_SIZE 4096

/** se connecter au serveur TCP d'adresse donnée en argument sous forme de
 * chaîne de caractère et au port donné en argument
 * retourne le descripteur de fichier de la socket obtenue ou -1 en cas
 * d'erreur. */
int connect_serveur_tcp(char *adresse, uint16_t port);

int main(int argc, char *argv[])
{
	if(argc < 2){
		printf("Usage : ./clt adresse_ip");
		exit(1);
	}
	int j = connect_serveur_tcp(argv[1], PORT_FREESCORD);
	if(j == -1){
		perror("socket erreur");
		exit(1);
	}
	return 0;
}

int connect_serveur_tcp(char *adresse, uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		return -1;
	}

	struct sockaddr_in client_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
	};

	if(inet_pton(AF_INET, adresse, &client_addr.sin_addr) < 0 ){
		return -1;
	}
	socklen_t client_size = sizeof(client_addr);

	if(connect(sock, (struct sockaddr *) &client_addr, client_size) < 0){
		return -1;
	}

	struct pollfd mes_poll[2] = {
		{.fd = 0, .events = POLLIN},
		{.fd = sock, .events = POLLIN}
	};

	buffer * buf = buff_create(0, BUFF_SIZE);
	buffer * buf2 = buff_create(sock, BUFF_SIZE);
	char dest[BUFF_SIZE];

	// Pour écrire le code ascii
	size_t n = read(sock, dest, BUFF_SIZE);
	write(1, dest, n);

	
	do{
		memset(dest, 0, BUFF_SIZE); // Nettoyer le buffer
		// Saisir le nickname et pseudonyme
		n = read(sock, dest, BUFF_SIZE);
		write(1, dest, n);
		memset(dest, 0, BUFF_SIZE); // Nettoyer le buffer

		// saisir le nickname et pseudonyme
		n = read(0, dest, BUFF_SIZE);
		write(sock, dest, n);
		memset(dest, 0, BUFF_SIZE); // Nettoyer le buffer

		// Lire la réponse du serveur
		n = read(sock, dest, 1);
		switch (dest[0]){
			case '0':
				printf("Nickname et pseudonyme validés.\n");
				printf("Vous pouvez commencer à chatter!\n");
				break;
			case '1':
				printf("Nickname déjà pris par un autre utilisateur, veuillez choisir un autre.\n");
				break;
			case '2':
				printf("Nickname ou pseudonyme inavalide, veuillez resaisir.\n");
				break;
			case '3':
				printf("Erreur syntaxe, veuillez saisir vos identifians comme suit : nickname ton_nickname ton_pseudonyme\n");
				break;
		}
	}while(dest[0] != '0');
	memset(dest, 0, BUFF_SIZE); // Nettoyer le buffer
	char *tmp;
	for(;;){
		poll(mes_poll, 2, -1);
		if(mes_poll[0].revents & (POLLIN | POLLHUP)){
			tmp = buff_fgets(buf, dest, BUFF_SIZE-1);
			if(tmp != NULL){
				size_t i = 0;
				while(dest[i] != '\n'){
					i++;
				}
				write(sock, dest, i+1);
			}
			else{
				break;
			}
		}
		else if(mes_poll[1].revents & (POLLIN | POLLHUP)){
			tmp = buff_fgets(buf2, dest, BUFF_SIZE-1);
			if(tmp != NULL){
				size_t i = 0;
				while(dest[i] != '\n'){
					i++;
				}
				write(1, dest, i+1);
			}
			else{
				printf("Serveur fermé :( \n");
				break;
			}
		}	
	}

	buff_free(buf);
	buff_free(buf2);
	return sock;
}
