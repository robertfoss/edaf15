#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

static void
fm_elim(fm_row *rows, unsigned int nbr_rows)
{
	printf("unused variable: %u, %p", nbr_rows, rows);
	return;
}

static unsigned int
count_rows(FILE *fp)
{
	if(fp == NULL)
		return 0;

	unsigned int rows = 0;
	char ch = '\0';

	while(ch != EOF) {
		ch = fgetc(fp);
		if(ch == '\n')
			rows++;
	}
	rewind(fp);

	return rows;
}

static unsigned int
count_cols(FILE *fp)
{
	if(fp == NULL)
		return 0;

	unsigned int cols = 0;
	char ch = '\0';

	while(ch != EOF && ch != '\n') {
		ch = fgetc(fp);
		if(ch == '\t' || ch == ' ')
			cols++;
	}
	cols++; // Since we've counted whitespaces but are really looking for columns.
	rewind(fp);

	return cols;
}

static fm_row *
parse_files(FILE *afile, FILE *cfile)
{
	unsigned int rows = count_rows(afile);
	unsigned int cols = count_cols(afile);

	fm_row *fm_rows = (fm_row *) malloc(sizeof(fm_row)*rows);
	fm_poly *fm_polys = (fm_poly *) malloc(sizeof(fm_poly)*rows*2);
	fm_poly_entry *fm_lesser_rows  = (fm_poly_entry *) malloc(sizeof(fm_poly_entry)*rows*cols);
	fm_poly_entry *fm_greater_rows = (fm_poly_entry *) malloc(sizeof(fm_poly_entry)*rows);

	if(fm_rows == NULL || fm_polys == NULL || fm_lesser_rows == NULL ||
	   fm_greater_rows == NULL) {
		fprintf(stderr, "Unable to allocate memory!\n");
		exit(1);
	}

	for(unsigned int i = 0; i < rows; ++i) {
		fm_rows[i].lesser = &(fm_polys[i]);
		fm_rows[i].greater = &(fm_polys[rows+i]);

		fm_rows[i].lesser->poly = &(fm_lesser_rows[i*cols]);
		fm_rows[i].lesser->poly_len = cols;

		fm_rows[i].greater->poly = &(fm_greater_rows[i]);
		fm_rows[i].greater->poly_len = 1;
	}

	return fm_rows;
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

	parse_files(afile, cfile);

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
