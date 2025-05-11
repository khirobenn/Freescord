#include <string.h>
char *crlf_to_lf(char *line_with_crlf)
{	
	size_t i = 0;
	static char new_line[512];
	while(line_with_crlf[i] != '\r'){
		new_line[i] = line_with_crlf[i];
		i++;
	}
	new_line[i] = '\n';
	return new_line;
}

char *lf_to_crlf(char *line_with_lf)
{
	size_t i = 0;
	static char new_line[512];
	while(line_with_lf[i] != '\n'){
		new_line[i] = line_with_lf[i];
		i++;
	}
	// Ici faut faire attention car peut être le caractère \n est écrit à la 512ème case de la chaîne
	// et donc \r on va l'écrire à la 512ème case et \n à la 513ème case qui va causer un bug
	new_line[i] = '\r';
	i++;
	new_line[i] = '\n';
	return new_line;
}
