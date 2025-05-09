#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

/** accepter une connection TCP depuis la socket d'écoute sl et retourner un
 * pointeur vers un struct user, dynamiquement alloué et convenablement
 * initialisé */
struct user *user_accept(int sl)
{
	struct sockaddr_in * client_addr = malloc(sizeof(struct sockaddr_in));
	socklen_t client_addr_length = sizeof(client_addr);
	int sock;
	if((sock = accept(sl, (struct sockaddr *) client_addr, &client_addr_length)) < 0){
		return NULL;
	}
	struct user * user = malloc(sizeof(user));
	user->address = (struct sockaddr *) client_addr;
	user->addr_len = client_addr_length;
	user->sock = sock;
	printf("User connected successfully !\n");
	return user;
}

/** libérer toute la mémoire associée à user */
void user_free(struct user *user)
{
	free(user->address);
	free(user);
	printf("User disconnected !\n");
}
