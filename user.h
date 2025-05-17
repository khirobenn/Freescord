#ifndef USER_H
#define USER_H
#include <sys/socket.h>
#include <netinet/in.h>

#define NAME_MAX 16

struct user {
	struct sockaddr *address;
	socklen_t addr_len;
	int sock;
	char nickname[NAME_MAX];
	char pseudonyme[NAME_MAX];
};

struct user *user_accept(int sl);
void user_free(struct user *user);

#endif /* ifndef USER_H */
