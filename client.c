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

	for(;;){
		char buff[256];

		poll(mes_poll, 2, -1);
		if(mes_poll[0].revents & (POLLIN | POLLHUP)){
			ssize_t n = read(0, buff, 256);
			buff[n] = '\0';
			write(sock, buff, 256);
		}
		else if(mes_poll[1].revents & (POLLIN | POLLHUP)){
			read(sock, buff, 256);
			printf("%s", buff);
			// write(1, buff, 256);
		}	
	}
	/* pour éviter les warnings de variable non utilisée */
	return sock;
}
