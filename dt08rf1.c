#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

#define CHK_BOUND(name, accessed_element, allocated_elements) { \
if((unsigned int)accessed_element >= (unsigned int)allocated_elements){\
	printf("Error: %s[%u] accessed when only %u elements are allocated\n", name, (unsigned int)accessed_element, (unsigned int)allocated_elements);\
}\
};

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


static fm_system* parse_files(FILE *afile, FILE *cfile)
{

    fm_system* system = (fm_system*)malloc(sizeof(fm_system));
    
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
        system->nbr_x = ctr_col;
        system->curr_nbr_x = ctr_col;
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
	
	system->rows = fm_rows;
	system->nbr_rows = count_rows(afile);
	
	return system;
}

static void print_row(fm_row* row){

    unsigned int j;

    fm_poly *poly_lesser  = row->lesser;
	fm_poly *poly_greater = row->greater;
	fm_poly_entry *poly_entry;

	for(j = 0; j < poly_lesser->poly_len; ++j) {
		poly_entry = &(poly_lesser->poly[j]);

        if(poly_entry->numerator == 0){
            continue;
        }

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
		
		if(poly_entry->numerator == 0){
            continue;
        }
		
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

static unsigned int* sort_by_coeffs(fm_system* system){ //sort system->rows by coeffs of largest x-index

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
        
        if(sort_entry.index != nbr_x || sort_entry.numerator == 0){ //coeff == 0
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
    /*free(pos_rows);
    free(neg_rows);
    free(zero_rows);*/
    
    printf("sorted on x_{%d}?\n", nbr_x);
    print_system(system);
    unsigned int *ret = malloc(sizeof(unsigned int)*2);
    ret[0] = nbr_pos;
    ret[1] = nbr_neg;
    return ret;
}

int f_m_elim(fm_system* system){
    unsigned int r = system->nbr_rows;
    unsigned int s = system->curr_nbr_x;
    unsigned int s2;
printf("sizeof(fm_poly_entry): %llu\n", (unsigned long long) sizeof(fm_poly_entry));
printf("system->nbr_rows: %u\n", system->nbr_rows);
printf("system->nbr_x: %u\n", system->nbr_x);
printf("system->curr_nbr_x: %u\n", system->curr_nbr_x);
    fm_poly_entry* t = (fm_poly_entry*)malloc(sizeof(fm_poly_entry)*system->nbr_rows*system->nbr_x);
    fm_poly_entry* q = (fm_poly_entry*)malloc(sizeof(fm_poly_entry)*system->nbr_rows);

    unsigned int i, j, n1, n2;
    unsigned int* ns;
    fm_poly_entry entry;
    fm_poly *b1, *b2;

    fm_row row;
    fm_poly *poly_lesser, *poly_greater;
    for(i = 0; i < system->nbr_rows; ++i){
        row = system->rows[i];
        poly_lesser = row.lesser;
        poly_greater = row.greater;
        
        if(poly_lesser->poly_len == 1 && poly_lesser->poly[0].index == 0){ //lesser == const side
            for(j = 0; j < system->nbr_x; ++j){
                t[i + j] = poly_greater->poly[j];
            }
            q[i] = poly_lesser->poly[0];
        } else if(poly_greater->poly_len == 1 && poly_greater->poly[0].index == 0){ //greater == const side
            for(j = 0; j < system->nbr_x; ++j){
                t[i + j] = poly_lesser->poly[j];
            }
            q[i] = poly_greater->poly[0];
        }
    }

    while(1){

        ns = sort_by_coeffs(system);
        n1 = ns[0];
        n2 = n1 + ns[1];
        for(i = 1; i <= r - 1; ++i){
            for(j = 1; j < n2; ++j){
                entry = t[i + j];
                entry.numerator = entry.numerator * t[r + j].denominator;
                entry.denominator = entry.denominator * t[r + j].numerator;
            }
        }
        for(j = 1; j < n2; ++j){
            entry = q[j];
            entry.numerator = entry.numerator * t[r + j].denominator;
            entry.denominator = entry.denominator * t[r + j].numerator;
        }
        if(n2 > n1){
            int c = 0;
            b1 = (fm_poly*)malloc(sizeof(fm_poly)*(n2 - (n1)));
            b1->poly_len = 0;
            fm_poly_entry* tmp = malloc(sizeof(fm_poly_entry)*(r+1));
            for(j = n1; j < n2; ++j){//TODO: possibly j=n1+1?
                for(i = 0; i < r - 1; ++i){
                    tmp[i] = t[i + j]; //TODO: neg?
                }
                tmp[i+1] = q[j];
                b1[c].poly_len++;
                b1[c].poly = tmp;
                c++;
            }
            free(tmp);
        }else{
            b1 = NULL;
        }
        if(n1 > 0){
            int c = 0;
            b2 = (fm_poly*)malloc(sizeof(fm_poly)*(n1));
            b2->poly_len = 0;
			fm_poly_entry* tmp = malloc(sizeof(fm_poly_entry)*(r+1));
            for(j = 0; j < n1; ++j){ //TODO: possibly j=1?
                for(i = 0; i < r - 1; ++i){
                    tmp[i] = t[i + j]; //TODO: neg?
                }
                tmp[i+1] = q[j];
                b2[c].poly_len++;
                b2[c].poly = tmp;
                c++;
            }
            free(tmp);
        }else{
            b2 = NULL;
        }
        if(r == 1){
            if((b1 != NULL && b2 != NULL) && (b1[0].poly->numerator * b2[0].poly->denominator) > (b2[0].poly->numerator * b1[0].poly->denominator)){
                return 0;
            }
            for(j = n2 + 1; j <= s; ++j){
                if(q[j].numerator < 0){
                    return 0;
                }
            }
            return 1;
        }

        s2 = s - n2 + n1 * (n2 - n1);
        if(s2 == 0){
            return 1;
        }
        r--;
        s = s2;
        
        for(i = 1; i <= r; ++i){
            
        }
    }

    
    
    
    
/*
typedef struct {
	long long numerator;
	long long denominator;
	unsigned long long index; // 14*x_{index} + -2x_{index} <= 68. 0 designates no x variable
} fm_poly_entry;

typedef struct {
	unsigned int poly_len;
	fm_poly_entry *poly;
} fm_poly;

typedef struct{
    unsigned int nbr_rows;
    unsigned int nbr_x; //largest x index in system
    unsigned int curr_nbr_x; //current largest x index, changes with elimination
    fm_row* rows;
} fm_system;
*/
    
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
    /*fm_system* system = (fm_system*)malloc(sizeof(fm_system));
    system->rows = parse_files(afile, cfile);
    system->nbr_rows = count_rows(afile);*/
	fm_system* system = parse_files(afile, cfile);
	print_system(system);

    //TODO: move
    f_m_elim(system);

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
