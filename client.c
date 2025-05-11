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
#define BUFF_SIZE 512

/** se connecter au serveur TCP d'adresse donnée en argument sous forme de
 * chaîne de caractère et au port donné en argument
 * retourne le descripteur de fichier de la socket obtenue ou -1 en cas
 * d'erreur. */
int connect_serveur_tcp(char *adresse, uint16_t port);

int main(int argc, char *argv[])
{
	int j = connect_serveur_tcp("127.0.0.1", PORT_FREESCORD);
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

	if(inet_pton(AF_INET, adresse, &client_addr.sin_addr) == -1 ){
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

	for(;;){
		char buff[BUFF_SIZE];
		memset(buff, 0, BUFF_SIZE); // Nettoyer le buffer
		poll(mes_poll, 2, -1);
		if(mes_poll[0].revents & (POLLIN | POLLHUP)){
			ssize_t n = read(0, buff, BUFF_SIZE);
			write(sock, buff, n);
		}
		else if(mes_poll[1].revents & (POLLIN | POLLHUP)){
			ssize_t n = read(sock, buff, BUFF_SIZE);
			write(1, buff, n);
		}	
	}
	/* pour éviter les warnings de variable non utilisée */
	return sock;
}
