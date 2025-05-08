#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list/list.h"
#include "user.h"

#define PORT_FREESCORD 4321

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);
/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);

int main(int argc, char *argv[]){
	int sock = create_listening_sock(PORT_FREESCORD);
	if(sock == -1){
		perror("socket erreur");
		exit(1);
	}
	// Créer les structures
	struct sockaddr_in serveur_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT_FREESCORD),
		.sin_addr.s_addr = htonl(INADDR_ANY)
	};
	socklen_t serveur_addr_length = sizeof(serveur_addr);

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	
	if(bind(sock, (struct sockaddr *)(&serveur_addr), serveur_addr_length) < 0){
		perror("bind erreur");
		exit(1);
	}

	if(listen(sock, 128) < 0){
		perror("listen erreur");
		exit(1);
	}

	for(;;){
		struct sockaddr_in client;
		socklen_t client_length = sizeof(client);
		int s = accept(sock, (struct sockaddr*) &client, &client_length);
		if(s < 0){
			perror("client connexion erreur");
			exit(1);
		}
		printf("Client connecté\n");
		char buff[256];
		ssize_t nb_read;
		nb_read = read(s, buff, 256);
		if(nb_read == 0){
			struct sockaddr_in client;
			socklen_t client_length = sizeof(client);
			s = accept(sock, (struct sockaddr*) &client, &client_length);
			continue;
		}
		buff[nb_read] = '\0';
		printf("%s\n", buff);
		write(s, buff, nb_read+1);
	}

	// struct sockaddr_in client_addr;
	return 0;
}

void *handle_client(void *clt)
{
	return clt;
}

int create_listening_sock(uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) return -1;

	return sock;
}
