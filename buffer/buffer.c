#include "buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

buffer *buff_create(int fd, size_t buffsz)
{	
	buffer * buffer = malloc(sizeof(buffer));
	buffer->buff = malloc(buffsz*sizeof(char));
	buffer->fd = fd;
	buffer->index = 1;
	buffer->size_read = 1;
	buffer->size = buffsz;
	return buffer;
}

int buff_getc(buffer *b)
{
	if(b->index < b->size_read){
		int a = b->buff[b->index];
		(b->index) ++;
		return a;
	}
	// càd on a lu tout les caractères du tableau, donc faut appeler read sur fd
	b->size_read = read(b->fd, b->buff, b->size);
	b->index = 1;
	if(b->size_read == 0){
		return -1;
	}
	return b->buff[0];	
}

int buff_ungetc(buffer *b, int c)
{
	if(b->index != 0){
		(b->index)--;
		b->buff[b->index] = c;
	}
	
	return c;
}

void buff_free(buffer *b)
{
	free(b->buff);
	free(b);
}

int buff_eof(const buffer *buff)
{
	char c;
	if(!read(buff->fd, &c, 1)){
		return 1;
	}
	return 0;
}

int buff_ready(const buffer *buff)
{
	if(buff->index < buff->size_read) return 1;
	return 0;
}

char *buff_fgets(buffer *b, char *dest, size_t size)
{
	int n = 0;
	for(int i = 0; i<size-1; i++){
		int a = buff_getc(b);
		if(a == -1) break;
		dest[i] = a;
		n++;
		if(a == '\n') break;
	}
	if(n == 0) return NULL;
	return b->buff;
}

char *buff_fgets_crlf(buffer *b, char *dest, size_t size)
{
	int n = 0;
	for(int i = 0; i<size-1; i++){
		int a = buff_getc(b);
		if(a == -1) break;
		dest[i] = a;
		n++;
		if(a == '\r'){
			a = buff_getc(b);
			if(a == '\n'){
				dest[i+1] = '\n';
				n++;
				break;
			}
			else {
				return NULL;
			}
		}
	}
	if(n == 0) return NULL;
	return b->buff;
}
