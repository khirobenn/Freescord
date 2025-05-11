#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(){
    buffer *b = buff_create(1, 20);
    int n;
    while((n = buff_getc(b) )!= -1){
        printf("%c", n);
    }
    buff_free(b);
    return 0;
}