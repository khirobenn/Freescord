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
	// Créer les structures du serveur
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
		struct user *user = user_accept(sock);
		handle_client((void *) user);
	}

	// struct sockaddr_in client_addr;
	return 0;
}

void *handle_client(void *clt)
{
	if(clt == NULL) return NULL;
	struct user * user = (struct user *) clt;
	for(;;){
		if(user->sock < 0){
			perror("client connexion erreur");
			exit(1);
		}
		char buff[256];
		ssize_t nb_read = 0;
		nb_read = read(user->sock, buff, 256);
		if(nb_read != 0){
			printf("%s", buff);
			write(user->sock, buff, nb_read);
		}
		else{break;}
	}
	close(user->sock);
	user_free(user);
	return NULL;
}

int create_listening_sock(uint16_t port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) return -1;

	return sock;
}
