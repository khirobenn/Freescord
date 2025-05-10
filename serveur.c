/* Khireddine BENMEZIANE 12308874
Je déclare qu'il s'agit de mon propre travail.
Ce travail a été réalisé intégralement par un être humain. */

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
#include <time.h>

#define PORT_FREESCORD 4321

char *ascii_art = {};

int tube[2];
struct list * users;

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);
/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);
void *repeat(void *arg);

int main(int argc, char *argv[]){
	if(pipe(tube) < 0){
		perror("pipe erreur");
		exit(1);
	}
	users = list_create();

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

	srand(time(NULL));

	pthread_t thread;
	pthread_create(&thread, NULL, repeat, NULL);
	pthread_detach(thread);

	for(;;){
		struct user *user = user_accept(sock);
		int a = rand()%11 + 1;
		char filename[50];
		sprintf(filename, "ascii_art/ascii%d", a);
		FILE *f = fopen(filename, "r");
		if(f == NULL){
			exit(1);
		}
		char ascii[4096];
		ssize_t n = fread(ascii, 1, 4096, f);
		fclose(f);
		ascii[n] = '\n';
		write(user->sock, ascii, n+1);
		users = list_add(users, (void *) user);
		pthread_t th;
		pthread_create(&th, NULL, handle_client, (void *) user);
		pthread_detach(th);
	}

	return 0;
}

void *handle_client(void *clt)
{
	if(clt == NULL) return NULL;
	// close(tube[0]);
	struct user * user = (struct user *) clt;
	for(;;){
		if(user->sock < 0){
			perror("client connexion erreur");
			exit(1);
		}
		char buff[4096];
		ssize_t nb_read = 0;
		nb_read = read(user->sock, buff, 4096);
		
		// Si y'a 0 octets lus ça veut dire que le client a fermé.
		if(nb_read == 0){
			break;
		}else{
			write(tube[1], buff, nb_read);
		}
		memset(buff, 0, 4096); // Nettoyer le buffer
	}
	list_remove_element(users, clt);
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

void *repeat(void *arg){
	for(;;){
		char buff[4096];
		buff[0] = '\0';
		ssize_t n;
		if(!list_is_empty(users) && (n = read(tube[0], buff, 4096)) > 0){
			write(1, buff, n);
			struct user *user;
			size_t i = 0;
			ssize_t length = list_length(users);
			do{
				user = (struct user *) list_get(users, i++);
				write(user->sock, buff, n);
			}while(user != NULL && i < length);

			memset(buff, 0, 4096); // Nettoyer le buffer
		}
		else{
			continue;
		}
	}
}
