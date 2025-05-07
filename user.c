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
	/* pour éviter les warnings de variable non utilisée */
	sl = 0;
	return NULL;
}

/** libérer toute la mémoire associée à user */
void user_free(struct user *user)
{
	/* pour éviter les warnings de variable non utilisée */
	user = NULL;
}
