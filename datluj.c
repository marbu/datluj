/*
 * This is 'datluj', foolish cli tool to simulate dumbphone keyborad typing.
 *
 * Copyright (C) 2003, 2014 Martin B. <martin.bukatovic@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of  MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * We use these extensions (not included in C99 standard):
 *  - POSIX 2004    (for timespec)
 *  - GNU extension (for getline)
 */
#define _XOPEN_SOURCE 600
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <getopt.h>

#define ARGUMENTS "hd:c:i:"

void usage(char* progname)
{
	printf("Usage: %s [options]\n\n", progname);
	printf(" -h        Show this help\n");
	printf(" -d DELAY  Key press delay (in ms)\n");
	printf(" -c CLASH  Key press delay when key clash happens (in ms)\n");
	printf(" -i PATH   Input file\n");
	printf("\nReport %s bugs to https://github.com/marbu/datluj\n", progname);
}

/*
 * Sleep msec miliseconds (just a wrapper for nanosleep).
 */
void sleep(int msec)
{
	struct timespec requested_time;
	requested_time.tv_sec = msec/1000;
	requested_time.tv_nsec = (msec%1000)*1000000L;
	nanosleep(&requested_time, NULL);
}

/*
 * definition of simple dumphone keyboard
 */
char *kbd[] = {
	"adgjmptw",
	"behknqux",
	"cfilorvy",
	"sz\0.....",
};

/*
 * How many times you have to press a key to write this character,
 * excluding final key press for the character itlself?
 */
int get_hits(char chr)
{
	chr = tolower(chr);
	for (int i=0; i<4; i++) {
		for (int j=0; kbd[i][j] != 0; j++) {
			if (chr == kbd[i][j]) {
				return i;
			}
		}
	}
	return 0;
}

/*
 * Check if these two chars shares the same key on mobile keyborad.
 */
int same_key(char c1, char c2)
{
	c1 = tolower(c1);
	c2 = tolower(c2);
	if (c1==c2) {
		return 1;
	}
	int same_button = 0;
	for (int i=0; i<8; i++) {
		same_button = \
			(c1 >= kbd[0][i]) && (c1 < kbd[0][i+1]) && \
			(c2 >= kbd[0][i]) && (c2 < kbd[0][i+1]);
		if (same_button) {
			return 1;
		}
	}
	return 0;
}

void datluj(char* input, int delay_ms, int clash_delay_ms)
{
	FILE* out_file = stdout;
	int kb_hits; 

	for (int i=0; input[i] != '\0'; i++) {
		kb_hits = get_hits(input[i]);
		/* print characters listed before this one on the key (if any) */
		for (int p=kb_hits; p>0; p--) {
			if (p!=kb_hits)
				 fprintf(out_file, "\b");
			fprintf(out_file, "%c", input[i]-p);
			fflush(out_file);
			sleep(delay_ms);
		}
		if (kb_hits)
			fprintf(out_file, "\b");
		/* print the character itself */
		fprintf(out_file, "%c", input[i]);
		fflush(out_file);
		/* check if the next char is going to be on the same key */
		if (same_key(input[i],input[i+1])) {
			sleep(delay_ms+clash_delay_ms);
		} else {
			sleep(delay_ms);
		}
	}
}

int main(int argc, char* argv[])
{
	FILE *in_file = stdin;
	char *in_file_path = NULL;
	int delay_ms = 150;
	int clash_delay_ms = 300;
	char *buffer = NULL;
	size_t line_len = 0;
	int c;

	while ((c = getopt(argc, argv, ARGUMENTS)) != -1) {
		switch (c) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'd':
			delay_ms = atoi(optarg);
			break;
		case 'c':
			clash_delay_ms = atoi(optarg);
			break;
		case 'i':
			in_file_path = optarg;
			break;
		}
	}

	if (in_file_path != NULL) {
		in_file = fopen(in_file_path, "r");
		if (in_file == NULL) {
			fprintf(stderr, "err: failed to open file %s\n", in_file_path);
			return EXIT_FAILURE;
		}
	}

	while (getline(&buffer, &line_len, in_file) != -1) {
		datluj(buffer, delay_ms, clash_delay_ms);
	}

	if (buffer)
		free(buffer);

	if (in_file_path != NULL)
		fclose(in_file);

	return EXIT_SUCCESS;
}
