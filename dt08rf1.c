#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

static unsigned long long	fm_count;
static volatile bool		proceed = false;

typedef struct {
	long long numerator;
	long long denominator;
	unsigned long long index; // 14*x_{index} + -2x_{index} <= 68. 0 designates no x variable
} fm_poly_entry;

typedef struct {
	unsigned int poly_len;
	fm_poly_entry *poly;
} fm_poly;

typedef struct {
	fm_poly *lesser;
	fm_poly *greater;
} fm_row;


long fsize(FILE *fp) {
	long size;

	if(fp != NULL) {
		if( fseek(fp, 0, SEEK_END) ) {
			fclose(fp);
			return -1;
		}

		size = ftell(fp);
		rewind(fp);
		return size;
	}
	return -1;
}


static void
fm_elim(fm_row *rows, unsigned int nbr_rows)
{
	return;
}


static unsigned int
count_rows(FILE *fp)
{
	if(fp == NULL)
		return 0;

	char ch = fgetc(fp);
	unsigned int rows = atoi(&ch);
	rewind(fp);

	return rows;
}


static unsigned int
count_cols(FILE *fp)
{
	if(fp == NULL)
		return 0;

	char ch = fgetc(fp);
	ch = fgetc(fp);
	ch = fgetc(fp);
	unsigned int cols = atoi(&ch);
	rewind(fp);

	return cols;
}


static fm_row *
parse_files(FILE *afile, FILE *cfile)
{
	unsigned int i,j;
	unsigned int rows = count_rows(afile);
	unsigned int cols = count_cols(afile);

	fm_row *fm_rows = (fm_row *) malloc(sizeof(fm_row)*rows);
	fm_poly *fm_polys = (fm_poly *) malloc(sizeof(fm_poly)*rows*2);
	fm_poly_entry *fm_lesser_rows  = (fm_poly_entry *) malloc(sizeof(fm_poly_entry)*rows*cols);
	fm_poly_entry *fm_greater_rows = (fm_poly_entry *) malloc(sizeof(fm_poly_entry)*rows);

	if(fm_rows == NULL || fm_polys == NULL || fm_lesser_rows == NULL || fm_greater_rows == NULL) {
		fprintf(stderr, "Unable to allocate memory!\n");
		exit(1);
	}

	for(i = 0; i < rows; ++i) {
		fm_rows[i].lesser = &(fm_polys[i]);
		fm_rows[i].greater = &(fm_polys[rows+i]);

	    fm_rows[i].lesser->poly = &(fm_lesser_rows[i*cols]);
		fm_rows[i].lesser->poly_len = cols;

		fm_rows[i].greater->poly = &(fm_greater_rows[i]);
		fm_rows[i].greater->poly_len = 1;
	}

	long file_length = fsize(afile);
	char *buffer = (char *) malloc(sizeof(char)*file_length);
	fread(buffer, sizeof(char), file_length, afile);

	long long read_integer;
	char read_char;

	char *tok = strtok(buffer, "\n");
	tok = strtok(NULL, "\n");
	for(i = 0; i < rows; ++i) {
		for(j = 0; j < cols; ++j) {
			int ret_val = sscanf(tok, "%lld%c", &read_integer, &read_char);
			if(ret_val != 2) {
				read_integer = 0;
			}
			printf("(%u,%u) %lld\n",i,j,read_integer);
				
			fm_rows[i].lesser->poly[j].numerator = read_integer;
			fm_rows[i].lesser->poly[j].denominator = 1;
			fm_rows[i].lesser->poly[j].index = j+1;
		}
		tok = strtok(NULL, "\n");
	}
	return fm_rows;
}


static void
print_fm_rows(fm_row* rows, unsigned int nbr_rows)
{
	unsigned int i,j;

	for(i = 0; i < nbr_rows; ++i) {
		fm_poly *poly_lesser  = rows[i].lesser;
		fm_poly *poly_greater = rows[i].greater;
		fm_poly_entry *poly_entry;

		for(j = 0; j < poly_lesser->poly_len; ++j) {
			poly_entry = &(poly_lesser->poly[j]);
			printf("(%lld/%lld)", poly_entry->numerator, poly_entry->denominator);
			if(poly_entry->index > 0) {
				printf("x_%llu ", poly_entry->index);
			} else {
				printf(" ");
			}
		}

		printf("<= ");

		for(j = 0; j < poly_greater->poly_len; ++j) {
			poly_entry = &(poly_greater->poly[j]);
			printf("(%lld/%lld)", poly_entry->numerator, poly_entry->denominator);
			if(poly_entry->index > 0) {
				printf("x_%llu ", poly_entry->index);
			} else {
				printf(" ");
			}
		}

		printf("\n");
	}
}


// TODO: Also free resources
static void
done(int unused)
{
	proceed = false;
	unused = unused;
}


unsigned long long
dt08rf1(char* aname, char* cname, int seconds)
{
	FILE*		afile = fopen(aname, "r");
	FILE*		cfile = fopen(cname, "r");

	fm_count = 0;

	if (afile == NULL) {
		fprintf(stderr, "could not open file A\n");
		exit(1);
	}

	if (cfile == NULL) {
		fprintf(stderr, "could not open file c\n");
		exit(1);
	}

	fm_row *fm_rows = parse_files(afile, cfile);
	print_fm_rows(fm_rows, count_rows(afile));

	if (seconds == 0) {
		/* Just run once for validation. */
			
		// Uncomment when your function and variables exist...
		// return fm_elim(rows, cols, a, c);

		fm_elim(NULL, 0);
		return 1; // return one, i.e. has a solution for now...
	}

	/* Tell operating system to call function DONE when an ALARM comes. */
	signal(SIGALRM, done);
	alarm(seconds);

	/* Now loop until the alarm comes... */
	proceed = true;
	while (proceed) {
		// Uncomment when your function and variables exist...
		// fm_elim(rows, cols, a, c);

		fm_elim(NULL, 0);

		fm_count++;
	}

	return fm_count;
}
