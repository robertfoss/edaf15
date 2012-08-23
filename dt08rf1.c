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
	unsigned long long index; // x_{index} + x_{index} <= x_{index}
} fm_row_entry;

typedef struct {
	fm_row_entry *lesser;
	fm_row_entry *greater;
} fm_row;

static void
fm_elim(fm_row *rows, unsigned int nbr_rows)
{
	printf("unused variable: %u, %p", nbr_rows, rows);
	return;
}

static void
count_rows_cols(FILE *fp, unsigned int *rows, unsigned int *cols)
{
	rows = 0;
	cols = 0;
	char ch = '\0';

	while(ch != EOF) {
		ch = fgetc(fp);
		if(ch == '\n')
			cols++;
		if(ch == '\t' || ch == ' ')
			rows++;
	}
	if(rows != 0)
		rows++;
	
	rewind(fp);

	return;
}

static fm_row*
parse_files(FILE *afile, FILE *cfile)
{   
    if(cfile == NULL){
        printf("cfile is null\n");
    }
	unsigned int rows = 0;
	unsigned int cols = 0;
	count_rows_cols(afile, &rows, &cols);

	fm_row *fmrows = (fm_row*) malloc(sizeof(fm_row)*rows);
	fm_row_entry *lesser_rows = (fm_row_entry*) malloc(sizeof(fm_row_entry)*rows*cols);
	fm_row_entry *greater_rows = (fm_row_entry*) malloc(sizeof(fm_row_entry)*cols);
	if(rows == 0 || lesser_rows == NULL || greater_rows == NULL) {
		fprintf(stderr, "Unable to allocate memory!\n");
		exit(1);
	}

	for(unsigned int i = 0; i < rows; ++i) {
		*fmrows[i].lesser = lesser_rows[i*cols];
		*fmrows[i].greater = greater_rows[i];
	}
    return fmrows;
}


// TODO: Free resources
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
