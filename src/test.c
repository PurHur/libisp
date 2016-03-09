/*
 * libisp -- Lisp evaluator based on SICP
 * (C) 2013 Martin Wolters
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libisp.h"

#define INPUT_PROMPT		"HIBT> "
#define OUTPUT_PROMPT		"YHBT: "
#define GOODBYE				"GB2FIOC!"

void print_banner(void) {
	printf(" '-._                  ___.....___\n");
	printf("     `.__           ,-'        ,-.`-,\n");
	printf("         `''-------'          ( p )  `._\n");
	printf("                               `-'      (         Have you conjured the spirits\n");
	printf("                                         \\              of your computer today?\n");
	printf("                                .         \\\n");
	printf("                                 \\\\---..,--'\n");
	printf("         .............._           --...--,\n");
	printf("                        `-.._         _.-'\n");
	printf("                             `'-----''                      Read SICP for help.\n\n");
}

int balanced_parens(char *exp) {
	int out = 0;
	char c;

	while((c = *(exp++))) {
		if(c == '(')
			out++;
		if(c == ')')
			out--;
	}

	return out;
}

static char *get_line(FILE *fp) {
	size_t size = 0, len  = 0, last = 0;
	char *buf  = NULL;

	do {
		size += BUFSIZ;
		buf = (char*)realloc(buf, size);
		fgets(buf + last, size, fp);
		len = strlen(buf);
		last = len - 1;
	} while (!feof(fp) && buf[last]!='\n');
	buf[last] = '\0';

	return buf;
}

char *input_exp(int *paren) {
	char *out, *buf;
	size_t bufsize = BUFSIZ, newlen;
	
	*paren = 0;

	if((out = (char*)malloc(bufsize)) == NULL) {
		fprintf(stderr, "ERROR: malloc(%zd) failed.\n", bufsize);
		return NULL;
	}

	memset(out, 0, bufsize);
	do {
		printf("%s", INPUT_PROMPT);
		buf = get_line(stdin);
		*paren += balanced_parens(buf);
		newlen = strlen(out) + strlen(buf) + 2;

		if(newlen > bufsize) {
			bufsize += BUFSIZ;
			if((out = (char*)realloc(out, bufsize)) == NULL) {
				fprintf(stderr, "ERROR: realloc(%zd) failed.\n", bufsize);
				return NULL;
			}
		}
		if(strlen(out))
			strcat(out, " ");
		else
			newlen--;

		strcat(out, buf);
		out[newlen - 1] = '\0';
		free(buf);
	} while(*paren > 0);

	return out;
}

int main(void) {
	data_t *exp_list, *ret;
	size_t readto, reclaimed;
	int error, paren = 0;
	char *exp, *buf;

	mem_verbosity = MEM_SILENT;
	mem_lim_soft = 1024 * 768;
	mem_lim_hard = 1024 * 1024;

	printf("Setting up the global environment...\n\n");
	setup_environment();
	print_banner();

	while(1) {
		readto = 0;

		exp = input_exp(&paren);
		if(paren < 0) {
			fprintf(stderr, "-- Syntax error: Unbalanced parentheses.\n");
			free(exp);
			continue;
		}

		if(!strcmp(exp, "(quit)")) {
			printf("%s\n", GOODBYE);
			free(exp);
			break;
		}
		buf = exp;

		do {
			exp_list = read_exp(exp, &readto, &error);
		
			if(error) {
				printf("-- Syntax Error: '%s'\n", exp);
				break;
			} else {
				ret = eval_thread(exp_list, the_global_env);
				printf("%s", OUTPUT_PROMPT);
				print_data(ret);
				printf("\n");
			}	
		
			exp += readto;

			if((reclaimed = run_GC(GC_LOWMEM)) && (mem_verbosity == MEM_VERBOSE))
				printf("-- GC: %zd bytes of memory reclaimed.\n", reclaimed);
		} while(strlen(exp));
		free(buf);
	}
	cleanup_lisp();
	showmemstats(stdout);
	return EXIT_SUCCESS;
}
