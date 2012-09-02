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

typedef struct{
    unsigned int nbr_rows;
    unsigned int nbr_x; //largest x index in system
    unsigned int curr_nbr_x; //current largest x index, changes with elimination
    fm_row* rows;
} fm_system;

long fsize(FILE *fp) {
	long size;

	if(fp != NULL) {
		if( fseek(fp, 0, SEEK_END) ) {
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
	unsigned int i;
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

	long file_length = fsize(afile)+1;
	char *buffer = (char *) malloc(sizeof(char)*file_length);
	fread(buffer, sizeof(char), file_length, afile);
	rewind(afile);
	buffer[file_length-1] = '\0';

	long long read_integer;
	char *line_tok, *integer_tok, *line_save, *integer_save;
	unsigned int ctr_col = 0;
	unsigned int ctr_row = 0;

	for (line_tok = strtok_r(buffer+4*sizeof(char), "\n", &line_save); line_tok; line_tok = strtok_r(NULL, "\n", &line_save)) {
		//printf("line=%s\n", line_tok);
		for (integer_tok = strtok_r(line_tok, " \t", &integer_save); integer_tok; integer_tok = strtok_r(NULL, " \t", &integer_save)) {
				//printf("integer=%s\n", integer_tok);
				sscanf(integer_tok, "%lld", &read_integer);

				fm_rows[ctr_row].lesser->poly[ctr_col].numerator = read_integer;
				fm_rows[ctr_row].lesser->poly[ctr_col].denominator = 1;
				fm_rows[ctr_row].lesser->poly[ctr_col].index = ctr_col + 1;
				++ctr_col;
		}
		ctr_col = 0;
		++ctr_row;
	}

	file_length = fsize(cfile)+1;
	buffer = (char *) malloc(sizeof(char)*file_length);
	fread(buffer, sizeof(char), file_length, cfile);
	rewind(cfile);
	buffer[file_length-1] = '\0';

	ctr_row = 0;

	for (line_tok = strtok_r(buffer+2*sizeof(char), " \t\n", &line_save); line_tok; line_tok = strtok_r(NULL, "\n", &line_save)) {
		printf("line=%s\n", line_tok);
		sscanf(line_tok, "%lld", &read_integer);

		fm_rows[ctr_row].greater->poly[0].numerator = read_integer;
		fm_rows[ctr_row].greater->poly[0].denominator = 1;
		fm_rows[ctr_row].greater->poly[0].index = 0;
		++ctr_row;
	}

	free(buffer);
	return fm_rows;
}

static void print_row(fm_row* row){

    unsigned int j;

    fm_poly *poly_lesser  = row->lesser;
	fm_poly *poly_greater = row->greater;
	fm_poly_entry *poly_entry;

	for(j = 0; j < poly_lesser->poly_len; ++j) {
		poly_entry = &(poly_lesser->poly[j]);

        if(poly_entry->denominator != 1){
            if(poly_entry->numerator > 0 && poly_entry->denominator > 0 && j != 0){
                printf("+ ");
            }
		    printf("(%lld/%lld)", poly_entry->numerator, poly_entry->denominator);
        } else {
            if(poly_entry->numerator > 0 && j != 0){
                printf("+ ");
            }
            printf("%lld", poly_entry->numerator);
        }

		if(poly_entry->index > 0) {
			printf("x_{%llu} ", poly_entry->index);
		} else {
			printf(" ");
		}
	}

	printf("<= ");

	for(j = 0; j < poly_greater->poly_len; ++j) {
		poly_entry = &(poly_greater->poly[j]);
		if(poly_entry->denominator != 1){
		    if(poly_entry->numerator > 0 && poly_entry->denominator > 0 && j != 0){
                printf("+ ");
            }
		    printf("(%lld/%lld)", poly_entry->numerator, poly_entry->denominator);
        } else {
            if(poly_entry->numerator > 0 && j != 0){
                printf("+ ");
            }
            printf("%lld", poly_entry->numerator);
        }
		if(poly_entry->index > 0) {
			printf("x_{%llu} ", poly_entry->index);
		} else {
			printf(" ");
		}
	}

	printf("\n");
}

static void
print_system(fm_system* system)
{
	unsigned int i;

	for(i = 0; i < system->nbr_rows; ++i) {
		print_row(&(system->rows[i]));
	}
}


// TODO: Also free resources
static void
done(int unused)
{
	proceed = false;
	unused = unused;
}

static void sort_by_coeffs(fm_system* system){ //sort system->rows by coeffs of largest x-index

    unsigned int i;
    fm_row row;
    fm_poly *poly_lesser, *poly_greater, *poly;
    fm_poly_entry sort_entry;
    long long numerator, denominator;
    unsigned int nbr_pos, nbr_neg, nbr_zero;
    
    nbr_pos = nbr_neg = nbr_zero = 0;
    unsigned int nbr_rows = system->nbr_rows;
    unsigned int nbr_x = system->nbr_x;
	fm_row *pos_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
	fm_row *neg_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
	fm_row *zero_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
    
    for(i = 0; i < nbr_rows; ++i){
        row = system->rows[i];
        poly_lesser = row.lesser;
        poly_greater = row.greater;
        
        if(poly_lesser->poly_len == 1 && poly_lesser->poly[0].index == 0){ //lesser == const side
            poly = poly_greater;
        } else if(poly_greater->poly_len == 1 && poly_greater->poly[0].index == 0){ //greater == const side
            poly = poly_lesser;
        } else { //TODO: is this possible?
            printf("no const side in row: ");
            print_row(&row);
            exit(1);
        }
        
        sort_entry = poly->poly[poly->poly_len-1]; //largest x-index term in row
        
        if(sort_entry.index != nbr_x){ //coeff == 0
            zero_rows[nbr_zero++] = row;
            continue;
        }
        
        numerator = sort_entry.numerator;
        denominator = sort_entry. denominator;
        
        if((numerator > 0 && denominator > 0) || (numerator < 0 && denominator < 0)){ // coeff > 0
            pos_rows[nbr_pos++] = row;
        } else { //coeff < 0
            neg_rows[nbr_neg++] = row;
        }
    }    
    
    for(i = 0; i < nbr_pos; ++i){
        system->rows[i] = pos_rows[i];
    }
    for(; i < nbr_pos + nbr_neg; ++i){
        system->rows[i] = neg_rows[i - nbr_pos];
    }
    for(; i < nbr_rows; ++i){
        system->rows[i] = zero_rows[i - (nbr_pos + nbr_neg)];
    }
    
    free(pos_rows);
    free(neg_rows);
    free(zero_rows);
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
    fm_system* system = (fm_system*)malloc(sizeof(fm_system));
    system->rows = parse_files(afile, cfile);
    system->nbr_rows = count_rows(afile);
	print_system(system);

    //TODO: move
    sort_by_coeffs(system);

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
