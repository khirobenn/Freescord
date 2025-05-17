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
#include <signal.h>

#define PORT_FREESCORD 4321
#define BUFF_SIZE 4096
#define NAME_MAX 16

pthread_mutex_t mutex;
int tube[2];
struct list * users;
int serveur_socket_global; // On l'utilisera pour fermer la socket du serveur lors de CTRL + C

int is_names_ok(char nickname[], char pseudonyme[], char buf[], size_t buff_size);
/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);
/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);
void *repeat(void *arg);
void free_user (void * clt);
void ctrl_c_handler(int signal);


int main(int argc, char *argv[]){
	if(pipe(tube) < 0){
		perror("pipe erreur");
		exit(1);
	}
	users = list_create();

	int sock = create_listening_sock(PORT_FREESCORD);
	serveur_socket_global = sock;
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
	pthread_mutex_init(&mutex, NULL);

	pthread_t thread;
	pthread_create(&thread, NULL, repeat, NULL);
	pthread_detach(thread);

	// Pour appeler la fonction ctrl_c_handler lors du CTRL+C
	// Source : https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
	signal(SIGINT, ctrl_c_handler);

	for(;;){
		struct user *user = user_accept(sock);
		pthread_t th;
		pthread_create(&th, NULL, handle_client, (void *) user);
		pthread_detach(th);
	}

	return 0;
}

// Vérifiez la chaîne de caractères entrées par le client si elle est sous la forme "nickname %s %s".
// Retourne soit 0, 2 ou 3 (les cas du sujet)
// Pour le cas 1 on le testera dans une autre fonction
int is_names_ok(char nickname[], char pseudonyme[], char buf[], size_t buff_size){
	// Checker la chaine nickname au début de buf
	char nickname_check[9] = "init"; // Je l'initialise juste pour que valgrind ne donne pas de warning
	int i = 0;
	while(i < buff_size && buf[i] != ' '){
		if(i >= NAME_MAX || i >= 8) return 3;
		nickname_check[i] = buf[i];
		i++;
	}
	nickname[i] = '\0';
	if(strcmp(nickname_check, "nickname")) return 3;
	i++;
	int j = 0;
	
	while(i < buff_size && buf[i] != ' '){
		if(j >= NAME_MAX) return 2;
		else if(buf[i] == ':') return 2;
		nickname[j] = buf[i];
		i++;
		j++;
	}
	if(i == buff_size || buf[i] == '\n') return 2;

	nickname[j] = '\0';
	i++;
	j = 0;
	while(i < buff_size && buf[i] != ' '){
		if(j >= NAME_MAX) return 2;
		else if(buf[i] == ':') return 2;
		else if(buf[i] == '\n') {
			j++;
			break;
		}
		pseudonyme[j] = buf[i];
		i++;
		j++;
	}
	pseudonyme[j] = '\0';
	return 0;
}

// Vérifier si le nickname est déjà présent chez l'un des utilisateurs.
// Retourne 1 si le nickname est présent, 0 sinon.
int is_nickname_in_other_users(char nickname[], struct list * users){
	if(list_is_empty(users)) return 0;

	struct user *user;
	size_t i = 0;
	ssize_t length = list_length(users);
	do{
		user = (struct user *) list_get(users, i++);
		if(!strcmp(user->nickname, nickname)) return 1;
	}while(user != NULL && i < length);

	return 0;
}

void *handle_client(void *clt)
{
	if(clt == NULL) return NULL;
	// close(tube[0]);
	struct user * user = (struct user *) clt;

	if(user->sock < 0){
		perror("connexion au client impossible");
		return NULL;
	}

	// afficher l'ascii art au client
	int a = rand()%9 + 1;
	char filename[30];
	sprintf(filename, "ascii_art/ascii%d", a);
	FILE *f = fopen(filename, "r");
	if(f == NULL){
		exit(1);
	}

	char ascii[BUFF_SIZE];
	ssize_t n;
	while((n = fread(ascii, 1, BUFF_SIZE, f)) > 0){
		write(user->sock, ascii, n);
	};
	fclose(f);
	// -----------------------------------

	// Nickname et pseudonyme
	char nickname[NAME_MAX];
	char pseudonyme[NAME_MAX];
	for(;;){
		char a[] = "\nEntrez un nickname et un pseudonyme :\n";
		write(user->sock, a, strlen(a));
		char b[BUFF_SIZE];
		size_t n = read(user->sock, b, BUFF_SIZE);
		// Le user a fermé 
		if(n <= 0){
			close(user->sock);
			user_free(user);
			return NULL;	
		}
		int i = is_names_ok(nickname, pseudonyme, b, n);
		if(i == 0){
			int is_in_others;
			pthread_mutex_lock(&mutex);
			is_in_others = is_nickname_in_other_users(nickname, users);
			pthread_mutex_unlock(&mutex);

			if(is_in_others){
				write(user->sock, "1", 1);
			}
			else{
				write(user->sock, "0", 1);
				break;
			}
		}
		else if(i == 2){
			write(user->sock, "2", 1);
		}
		else if(i == 3){
			write(user->sock, "3", 1);
		}
	}

	// Nickname et pseudonyme validés
	strcpy(user->nickname, nickname);
	strcpy(user->pseudonyme, pseudonyme);

	pthread_mutex_lock(&mutex);
	users = list_add(users, (void *) user);
	pthread_mutex_unlock(&mutex);

	printf("- %s connected successfully !\n", user->nickname);

	for(;;){
		char buff[BUFF_SIZE];
		ssize_t nb_read = 0;
		nb_read = read(user->sock, buff, BUFF_SIZE);
		
		// Si y'a 0 octets lus ça veut dire que le client a fermé.
		if(nb_read == 0){
			break;
		}else{
			// Tester si la chaine commence par 'msg'
			char commande[20];
			int i = strlen("msg");
			strncpy(commande, buff, i);
			commande[i] = '\0';
			if(!strcmp(commande, "msg")){
				write(tube[1], "msg ", i+1);
				// écrire le nickname
				write(tube[1], user->nickname, strlen(user->nickname));
				write(tube[1], " : ", 3);
				write(tube[1], buff+i+1, nb_read-i-1);
			}
			// Si la chaine ne commence pas 'msg', on teste alors pour 'list'
			else{
				i = strlen("list");
				strncpy(commande, buff, i);
				commande[i] = '\0';
				if(!strcmp(commande, "list")){
					pthread_mutex_lock(&mutex);
					struct user *u;
					size_t i = 0;
					ssize_t length = list_length(users);
					do{
						u = (struct user *) list_get(users, i++);
						if(u != user){
							write(user->sock, "- ", 2);
							write(user->sock, u->nickname, strlen(u->nickname));
							write(user->sock, " is connected\n", strlen(" is connected\n"));
						}
					}while(u != NULL && i < length);
					pthread_mutex_unlock(&mutex);
				}
				else{
					// La chaine ne commence ni par 'msg', ni par 'list' alors on envoie une erreur au client
					char warning[] = "Veuillez utiliser `msg` avant le message ou `list` pour lister les utilisateur connectés.\n";
					write(user->sock, warning, strlen(warning));
				}
			}
		}
		memset(buff, 0, BUFF_SIZE); // Nettoyer le buffer
	}
	pthread_mutex_lock(&mutex);
	list_remove_element(users, clt);
	pthread_mutex_unlock(&mutex);
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
		char buff[BUFF_SIZE];
		buff[0] = '\0';
		ssize_t n;
		if(!list_is_empty(users) && (n = read(tube[0], buff, BUFF_SIZE)) > 0){
			write(1, buff, n);
			pthread_mutex_lock(&mutex);
			struct user *user;
			size_t i = 0;
			ssize_t length = list_length(users);
			do{
				user = (struct user *) list_get(users, i++);
				write(user->sock, buff, n);
			}while(user != NULL && i < length);
			pthread_mutex_unlock(&mutex);
			memset(buff, 0, BUFF_SIZE); // Nettoyer le buffer
		}
		else{
			continue;
		}
	}
}

// Une fonction temporaire pour la passer dans list_free pour libérer chaque noeud.
void free_user (void * clt){
	struct user * u = (struct user *) clt;
	close(u->sock);
	user_free(u);
}

// Cette fonction va gérer l'événement du CTRL+C
// Source : https://www.geeksforgeeks.org/write-a-c-program-that-doesnt-terminate-when-ctrlc-is-pressed/
void ctrl_c_handler(int signal){
	// On ferme le serveur
	close(serveur_socket_global);
	printf("\n");
	// On libère la mémoire de la liste des clients
	pthread_mutex_lock(&mutex);
	list_free(users, free_user);
	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	exit(0);
}