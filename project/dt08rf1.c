#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>

#define CHK_BOUND(name, accessed_element, allocated_elements) { \
if((unsigned int)accessed_element >= (unsigned int)allocated_elements){\
	printf("Error: %s[%u] accessed when only %u elements are allocated\n", name, (unsigned int)accessed_element, (unsigned int)allocated_elements);\
}\
};

static unsigned long long	fm_count;
static volatile bool			proceed = false;

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

static void print_row(fm_row* row);


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
printf("line=%s\n", line_tok);
		for (integer_tok = strtok_r(line_tok, " \t", &integer_save); integer_tok; integer_tok = strtok_r(NULL, " \t", &integer_save)) {
printf("integer=%s\n", integer_tok);
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

//	free(buffer);
	
	system->rows = fm_rows;
	system->nbr_rows = count_rows(afile);
	
	return system;
}

static void print_poly(fm_poly* poly){

    unsigned int j;

	fm_poly_entry *poly_entry;
    if(poly == NULL){
        printf("NULL ");
        return;
    }

	for(j = 0; j < poly->poly_len; ++j) {
		poly_entry = &(poly->poly[j]);

        if(poly_entry->numerator == 0){
            //continue;
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

}

static void print_row(fm_row* row){
	print_poly(row->lesser);
	printf("<= ");
	print_poly(row->greater);
	printf("\n");
}

static void print_system(fm_system* system){
	unsigned int i;

	for(i = 0; i < system->nbr_rows; ++i) {
		print_row(&(system->rows[i]));
	}
}


// TODO: Also free resources
static void done(int unused){
	proceed = false;
	unused = unused;
}

static int is_const(fm_poly* poly){
	if(poly == NULL){
		return 0;
	}
	unsigned int i;
	for(i = 0; i < poly->poly_len; i++){
		if(poly->poly[i].numerator != 0 && poly->poly[i].index != 0){
			return 0;
		}
	}
	return 1;
}

static unsigned int* sort_by_coeffs(fm_system* system){ //sort system->rows by coeffs of largest x-index

    unsigned int i, j;
    fm_row row;
    fm_poly *poly_lesser, *poly_greater, *poly;
    fm_poly_entry sort_entry;
    long long numerator, denominator;
    unsigned int nbr_pos, nbr_neg, nbr_zero;
    
    nbr_pos = nbr_neg = nbr_zero = 0;
    unsigned int nbr_rows = system->nbr_rows;
    unsigned int nbr_x = system->curr_nbr_x;
	fm_row *pos_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
	fm_row *neg_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
	fm_row *zero_rows = (fm_row*) malloc(sizeof(fm_row)*nbr_rows);
    
    for(i = 0; i < nbr_rows; ++i){
        row = system->rows[i];
        poly_lesser = row.lesser;
        poly_greater = row.greater;
        if(is_const(poly_lesser)){ //lesser == const side
            poly = poly_greater;
        } else if(is_const(poly_greater)){ //greater == const side
            poly = poly_lesser;
        } else {
			// The case of two sides being const. Hopefully
			if(poly_greater != NULL || poly_lesser != NULL)
				printf("Error: No side are const, but both aren't NULL.");
			continue;

        }
printf("sort poly (%u): ", nbr_x);print_poly(poly);printf("\n");
		
        sort_entry = poly->poly[nbr_x-1]; //largest x-index term in row
		if(sort_entry.index != (nbr_x)){
			for(j = 0; j < poly->poly_len; ++j){
				if(poly->poly[j].index == (nbr_x)){
					sort_entry = poly->poly[j]; //largest x-index term in row
				}
			}
		}
printf("sort_entry = (%lld/%lld)x_%llu\n", sort_entry.numerator, sort_entry.denominator, sort_entry.index);


//printf("\tsort_entry = (%lld/%lld)x_{%llu}\n", sort_entry.numerator, sort_entry.denominator, sort_entry.index);
        
        if(sort_entry.index != nbr_x || sort_entry.numerator == 0){ //coeff == 0
            zero_rows[nbr_zero++] = row;
//printf("zero: "); print_row(&row);
            continue;
        }
        
        numerator = sort_entry.numerator;
        denominator = sort_entry. denominator;
        
        if((numerator > 0 && denominator > 0) || (numerator < 0 && denominator < 0)){ // coeff > 0
//printf("pos: "); print_row(&row);
            pos_rows[nbr_pos++] = row;
        } else { //coeff < 0
//printf("neg: "); print_row(&row);
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
    unsigned int *ret = malloc(sizeof(unsigned int)*3);
    ret[0] = nbr_pos;
    ret[1] = nbr_neg;
    ret[2] = nbr_zero;
    return ret;;
}

fm_poly* copy_poly(fm_poly* poly){
	if(poly == NULL)
		return NULL;
	fm_poly *tmp = malloc(sizeof(fm_poly));
	fm_poly_entry *tmp_entries = malloc(sizeof(fm_poly_entry)*poly->poly_len);
	tmp->poly_len = poly->poly_len;
	tmp->poly = tmp_entries;

	memcpy(tmp_entries, poly->poly, sizeof(fm_poly_entry)*poly->poly_len);
//printf("orig poly: "); print_poly(poly);
//printf("copy poly: "); print_poly(tmp);
	return tmp;
}

int elim_2(fm_system* system){
    
    unsigned int i, j;
	fm_poly *b1 = (fm_poly*) malloc(sizeof(fm_poly)*(system->nbr_rows));
	fm_poly *b2 = (fm_poly*) malloc(sizeof(fm_poly)*(system->nbr_rows));
	unsigned int n_b1 = 0;
	unsigned int n_b2 = 0;
	unsigned int n_neg = 0;

	while(system->curr_nbr_x > 0){

printf("system:\n");print_system(system);printf("printed system\n");

		//Sort
		unsigned int* ns = sort_by_coeffs(system);
		unsigned int n_pos = ns[0];
		n_neg = ns[1];
		unsigned int n_zero = ns[2];
		unsigned int n_non_zero = n_pos + n_neg;

		
		fm_poly *tmp_poly;
		fm_poly_entry tmp_poly_entry;
		
		//Divide
		printf("\nDivide:\n");
printf("npos = %d, nneg = %d\n", n_pos, n_neg);
		for(i = 0; i < n_pos + n_neg; ++i){
		    tmp_poly = system->rows[i].lesser;
		    tmp_poly_entry = tmp_poly->poly[system->curr_nbr_x - 1];
printf("\ttmp_poly_entry = (%lld/%lld)x_{%llu}\n", tmp_poly_entry.numerator, tmp_poly_entry.denominator, tmp_poly_entry.index);
		    
		    for(j = 0; j < tmp_poly->poly_len; ++j){
		        tmp_poly->poly[j].numerator = tmp_poly->poly[j].numerator * tmp_poly_entry.denominator;
		        tmp_poly->poly[j].denominator = tmp_poly->poly[j].denominator * tmp_poly_entry.numerator;

				if(tmp_poly->poly[j].numerator == tmp_poly->poly[j].denominator){
					tmp_poly->poly[j].numerator = 1;
					tmp_poly->poly[j].denominator = 1;
				}
		    }

			tmp_poly = system->rows[i].greater;
		    
		    for(j = 0; j < tmp_poly->poly_len; ++j){
		        tmp_poly->poly[j].numerator = tmp_poly->poly[j].numerator * tmp_poly_entry.denominator;
		        tmp_poly->poly[j].denominator = tmp_poly->poly[j].denominator * tmp_poly_entry.numerator;

				if(tmp_poly->poly[j].numerator == tmp_poly->poly[j].denominator){
					tmp_poly->poly[j].numerator = 1;
					tmp_poly->poly[j].denominator = 1;
				}
		    }
		    
		    //system->rows[i].greater->poly[0].numerator = system->rows[i].greater->poly[0].numerator * tmp_poly_entry.denominator;
		    //system->rows[i].greater->poly[0].denominator = system->rows[i].greater->poly[0].denominator * tmp_poly_entry.numerator;
		    
		    if((tmp_poly_entry.numerator < 0 && tmp_poly_entry.denominator > 0) || (tmp_poly_entry.numerator > 0 && tmp_poly_entry.denominator < 0)){
		        system->rows[i].greater = system->rows[i].lesser;
		        system->rows[i].lesser = tmp_poly;
		    }
		}
		print_system(system);
		
		//Isolate
		fm_row *new_rows = (fm_row*) malloc(sizeof(fm_row)*n_non_zero);
		fm_poly *new_poly = (fm_poly*) malloc(sizeof(fm_poly)*n_non_zero); //TODO: Maybe add individual allocations for consistency
		fm_poly_entry *new_entries = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*system->nbr_rows*(system->nbr_x + 1));
		fm_poly *curr_poly;
		printf("\nIsolate:\n");
		for(i = 0; i < n_non_zero; ++i){
		
		    curr_poly = &(new_poly[i]);
		    curr_poly->poly_len = system->nbr_x + 1;
		    curr_poly->poly = &(new_entries[i*curr_poly->poly_len]);
		    
		    if(is_const(system->rows[i].lesser)){ //if lesser == const side
		       
		        for(j=0; j < system->curr_nbr_x-1; ++j){
		            curr_poly->poly[j].numerator = -system->rows[i].greater->poly[j].numerator;
		            curr_poly->poly[j].denominator = system->rows[i].greater->poly[j].denominator;
		            curr_poly->poly[j].index = system->rows[i].greater->poly[j].index;
		        }
				for(; j < system->nbr_x; ++j){
		            curr_poly->poly[j].numerator = 0;
		            curr_poly->poly[j].denominator = 1;
		            curr_poly->poly[j].index = j+1;
				}
//printf("const? ");print_poly((system->rows[i].lesser));printf("\n");
		        curr_poly->poly[j].numerator = system->rows[i].lesser->poly[system->rows[i].lesser->poly_len - 1].numerator;
		        curr_poly->poly[j].denominator = system->rows[i].lesser->poly[system->rows[i].lesser->poly_len - 1].denominator;
		        curr_poly->poly[j].index = system->rows[i].lesser->poly[system->rows[i].lesser->poly_len - 1].index;
//print_poly(curr_poly); printf("\tpoly_len = %u\n", curr_poly->poly_len);
		        new_rows[i].greater = NULL;
		        new_rows[i].lesser = curr_poly;
		        
		    } else {    
		            
		        for(j=0; j < system->curr_nbr_x-1; ++j){
		            curr_poly->poly[j].numerator = -system->rows[i].lesser->poly[j].numerator;
		            curr_poly->poly[j].denominator = system->rows[i].lesser->poly[j].denominator;
		            curr_poly->poly[j].index = system->rows[i].lesser->poly[j].index;
		        }
				for(; j < system->nbr_x; ++j){
		            curr_poly->poly[j].numerator = 0;
		            curr_poly->poly[j].denominator = 1;
		            curr_poly->poly[j].index = j+1;
				}
//printf("const? ");print_poly((system->rows[i].greater));printf("\n");
		        curr_poly->poly[j].numerator = system->rows[i].greater->poly[system->rows[i].greater->poly_len - 1].numerator;
		        curr_poly->poly[j].denominator = system->rows[i].greater->poly[system->rows[i].greater->poly_len - 1].denominator;
		        curr_poly->poly[j].index = system->rows[i].greater->poly[system->rows[i].greater->poly_len - 1].index;
//print_poly(curr_poly); printf("\tpoly_len = %u\n", curr_poly->poly_len);
		        new_rows[i].lesser = NULL;
		        new_rows[i].greater = curr_poly;
		    }
		    print_row(&(new_rows[i]));
		}
		
		//B's
		b1 = (fm_poly*) malloc(sizeof(fm_poly)*(system->nbr_rows));
		b2 = (fm_poly*) malloc(sizeof(fm_poly)*(system->nbr_rows));
		n_b1 = 0;
		n_b2 = 0;
		for(i = 0; i < n_non_zero; ++i){
		    if(new_rows[i].lesser == NULL){
		        b2[n_b2++] = *(new_rows[i].greater);
		    } else {
		        b1[n_b1++] = *(new_rows[i].lesser);
		    }
		}

		if(n_b1 == 0){
			n_b1 = 1;
			b1 = NULL;
/*			fm_poly_entry* b1_poly_entry = malloc(sizeof(fm_poly_entry));
			b1_poly_entry->index = 0;
			b1_poly_entry->numerator = -(ULONG_MAX-1);
			b1_poly_entry->denominator = 1;
			b1[0].poly = b1_poly_entry;
			b1[0].poly_len = 1;
*/		}
		if(n_b2 == 0){
			n_b2 = 1;
			b2 = NULL;
/*			fm_poly_entry* b2_poly_entry = malloc(sizeof(fm_poly_entry));
			b2_poly_entry->index = 0;
			b2_poly_entry->numerator = ULONG_MAX;
			b2_poly_entry->denominator = 1;
			b2[0].poly = b2_poly_entry;
			b2[0].poly_len = 1;
*/		}

		if(system->curr_nbr_x == 1){
			break; //This is the last iteration, further steps arent needed.
		}

		printf("\nCreate b1:\n");
		for(i = 0; i<n_b1;++i) {print_poly(&(b1[i])); printf("\n");}
		printf("Create b2:\n");
		for(i = 0; i<n_b2;++i) {print_poly(&(b2[i])); printf("\n");}
		printf("#b1: %u\t #b2: %u\n", n_b1, n_b2);
		
		//Merge b's
		printf("\nMerge b*:\n");
		int count = 0;
		fm_row *b_rows = (fm_row*) malloc(sizeof(fm_row)*(n_b1*n_b2+n_zero));
		for(i = 0; i < n_b1; ++i){
		    for(j = 0; j < n_b2; ++j){
		        b_rows[count].lesser = copy_poly(&(b1[i]));
		        b_rows[count].greater = copy_poly(&(b2[j]));
				count++;
		    }
		}

	//	free(b1); TODO:
	//	free(b2); TODO:
		for(i = 0; i < n_b1*n_b2; ++i) print_row(&(b_rows[i]));

		//Simplify
		printf("\nSimplify:\n");
		fm_poly_entry *great_e, *less_e;
		for(i = 0; i < n_b1*n_b2; ++i){
			// If either side is 'infinity' ie NULL.
			if(b_rows[i].greater == NULL || b_rows[i].lesser == NULL)
				continue;

			//Move non-constants to lesser side
			for(j = 0; j < b_rows[i].greater->poly_len-1; ++j){
				great_e = &(b_rows[i].greater->poly[j]);
				less_e = &(b_rows[i].lesser->poly[j]);
				less_e->numerator =  less_e->numerator*great_e->denominator - great_e->numerator*less_e->denominator;
				less_e->denominator = less_e->denominator*great_e->denominator;

				great_e->numerator = 0;
				great_e->denominator = 1;
			}
			great_e = &(b_rows[i].greater->poly[b_rows[i].greater->poly_len-1]);
			less_e = &(b_rows[i].lesser->poly[b_rows[i].greater->poly_len-1]);

			//Move constants to greater side
			great_e->numerator = great_e->numerator*less_e->denominator - less_e->numerator*great_e->denominator;
			great_e->denominator = great_e->denominator*less_e->denominator;
			less_e->numerator = 0;
			less_e->denominator = 1;
		}
		for(i = 0; i < n_b1*n_b2; ++i) print_row(&(b_rows[i]));
	 

		//Add rows with 0 coeff
		printf("\nAdd zero-coeff rows:\n");
		i = n_b1*n_b2;
		for(j = n_pos + n_neg; j < system->nbr_rows; ++j){
		    b_rows[i].lesser = copy_poly(system->rows[j].lesser);
			b_rows[i].greater = copy_poly(system->rows[j].greater);
			++i;
		}
		for(i = 0; i < n_b1*n_b2+n_zero; ++i) print_row(&(b_rows[i]));
		printf("b_rows size: %u\n", n_b1*n_b2+n_zero);
		printf("elim_2 done\n");
		system->curr_nbr_x = system->curr_nbr_x - 1;
printf("curr nbr x = %d\n", system->curr_nbr_x);
		//COPY ALL NON-ZERO COEFF ENTRIES FROM TO SYSTEM FROM b_row
//		free(system->rows); //TODO:
		system->rows = b_rows;
		
		system->nbr_rows = n_b1*n_b2+n_zero;
		
	}

	//max b1, min b2
	fm_poly_entry *tmp;
	double max=(-DBL_MAX), min=DBL_MAX, cmp;
	printf("max: %f  min: %f\n", max, min);
	for(i = 0; i < n_b1 && b1!=NULL; i++){
		
		tmp = &b1[i].poly[b1[i].poly_len - 1];
		if(tmp->index != 0){
			printf("Error: Kill all humans1010001b1000\n");
		}
		cmp = (double)tmp->numerator / (double)tmp->denominator;
		if(cmp > max)
			max = cmp;
	}
	for(i = 0; i < n_b2 && b2!=NULL; i++){
		tmp = &b2[i].poly[b2[i].poly_len - 1];
		if(tmp->index != 0){
			printf("Error: Kill all humans1010001b2000\n");
		}
		cmp = (double)tmp->numerator / (double)tmp->denominator;
		if(cmp < min)
			min = cmp;
	}
	//TODO::::
	//free(b1);
	//free(b2);
	printf("\n------------------\nAlgorithm complete\n------------------\n");print_system(system);
	/*if(min >= max)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;*/
	printf("\n-----------\nMax bound: %f\nMin bound: %f\n------------\n", max, min);
    if(max > min){
		return EXIT_FAILURE;
	}

	for(j = n_neg; j < system->nbr_rows; ++j){
		if(is_const(system->rows[i].lesser)){
			if(system->rows[i].lesser->poly[system->rows[i].lesser->poly_len - 1].numerator < 0){
				return EXIT_FAILURE;
			}
		}else{
			if(system->rows[i].greater->poly[system->rows[i].greater->poly_len - 1].numerator < 0){
				return EXIT_FAILURE;
			}
		}
	}
	return EXIT_SUCCESS;
}

int f_m_elim(fm_system* system){
    return 5;
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
        for(i = 0; i < r - 1; ++i){
            for(j = 0; j < n2; ++j){
                entry = t[i + j];
                entry.numerator = entry.numerator * t[r + j].denominator;
                entry.denominator = entry.denominator * t[r + j].numerator;
            }
        }

        for(j = 0; j < n2; ++j){
            entry = q[j];
            entry.numerator = entry.numerator * t[r + j].denominator;
            entry.denominator = entry.denominator * t[r + j].numerator;
        }
        printf("B1:\n");
        if(n2 > n1){
            int c = 0;
            b1 = (fm_poly*)malloc(sizeof(fm_poly)*(n2 - (n1)));
            //b1->poly_len = 0;
            for(j = n1; j < n2; ++j){
                fm_poly_entry* tmp = malloc(sizeof(fm_poly_entry)*(r+1));
                for(i = 0; i < r - 1; ++i){
                    tmp[i] = t[i + j]; //TODO: neg?
                }
                tmp[i+1] = q[j];
                b1[c].poly_len = r - 1;
                b1[c].poly = tmp;
                c++;
            }
            print_poly(&(b1[c]));
            //free(tmp);
        }else{
            b1 = NULL;
        }
        
        printf("B2:\n");
        if(n1 > 0){
            int c = 0;
            b2 = (fm_poly*)malloc(sizeof(fm_poly)*(n1));
            //b2->poly_len = 0;
            for(j = 0; j < n1; ++j){
			    fm_poly_entry* tmp = malloc(sizeof(fm_poly_entry)*(r+1));
                for(i = 0; i < r - 1; ++i){
                    tmp[i] = t[i + j]; //TODO: neg?
                }
                tmp[i+1] = q[j];
                b2[c].poly_len = r - 1;
                b2[c].poly = tmp;
                c++;
            }
            print_poly(&(b2[c]));
        }else{
            b2 = NULL;
        }
        if(r == 1){
            if((b1 != NULL && b2 != NULL) && (b1[0].poly->numerator * b2[0].poly->denominator) > (b2[0].poly->numerator * b1[0].poly->denominator)){
                return 0;
            }
            for(j = n2; j < s; ++j){
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
        //s - n2 + n1n2 -n1n1
        //n2
        r--;
        s = s2;
        
        //fm_row *b_combined_rows = (fm_row*) malloc(sizeof(fm_row)*n1*(n2-n1)); //TODO: wutwutwut
        fm_row *b_combined_rows = (fm_row*) malloc(sizeof(fm_row)*s);
        
        fm_poly l, g;
        unsigned int k, row_count;
        row_count = 0;
        
        fm_row *tmp_test_row = (fm_row*) malloc(sizeof(fm_row));
        
        for(i = 0; i < (n2-n1); ++i){
            for(j = 0; j < n1; ++j){
                
                l = b1[i];
                g = b2[j];
                
                tmp_test_row->lesser = &l;
                tmp_test_row->greater = &g;
                printf("tem test? ");
                print_row(tmp_test_row);

                fm_poly *poly = (fm_poly*) malloc(sizeof(fm_poly));
                fm_poly_entry *entries = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*(l.poly_len-1));
                
                for(k = 0; k < l.poly_len - 2; ++k){
                        
                    fm_poly_entry pe_l = l.poly[k];
                    fm_poly_entry pe_g = g.poly[k];
                    
                    fm_poly_entry *tmp_entry = &(entries[k]);
                    tmp_entry->numerator = pe_l.numerator * pe_g.denominator - pe_g.numerator * pe_l.denominator;
                    tmp_entry->denominator = pe_l.denominator * pe_g.denominator;
                    tmp_entry->index = pe_g.index;
                        
                }
                poly->poly = entries;
                poly->poly_len = k;
                b_combined_rows[row_count].lesser = poly;
                fm_poly_entry pe_l = l.poly[k];
                fm_poly_entry pe_g = g.poly[k];
                
                poly = (fm_poly*) malloc(sizeof(fm_poly));
                poly->poly = (fm_poly_entry*) malloc(sizeof(fm_poly_entry));
                poly->poly_len = 1;
                b_combined_rows[row_count].greater = poly;
                poly->poly[0].numerator = pe_l.numerator * pe_g.denominator - pe_g.numerator * pe_l.denominator;
                poly->poly[0].denominator = pe_l.denominator * pe_g.denominator;
                poly->poly[0].index = pe_g.index;
                
                printf("comb rows \n");
                print_row(b_combined_rows);
                
                row_count++;
                
            }
            
            for(i = n2; i < s; ++i){
                b_combined_rows[row_count].lesser = system->rows[i].lesser;
                b_combined_rows[row_count].greater = system->rows[i].greater;
                
                print_row(b_combined_rows);
                
                row_count++;
            }
            
        }
        
        system->rows = b_combined_rows;
        printf("Changed system?!?!?!?\n");
        print_system(system);
        
        
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

*/
        
        
        
        
        /*
        fm_poly *b_combined = (fm_poly*) malloc(sizeof(fm_poly)*s);
        fm_poly_entry b_constants
        */        
        
        
        
        
        
        /*///////////////////////////
        fm_poly_entry *t_ik;
        fm_poly_entry *t_il;
        fm_poly_entry *q_k;
        fm_poly_entry *q_l;
        
        unsigned int k, l;
        
        fm_poly_entry *new_t = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*(r-1)*s2);
        fm_poly_entry *new_q = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*s2);
        fm_poly_entryb1plusb2 = 
        
        for(i = 0; i < r - 1; ++i){
            for(k = 0; k < n1; ++k){
                for(l = n1; l < n2; ++l){
                
                    t_ik = t[i + k];
                    t_il = t[i + l];
                    
                    q_k = q[k];
                    q_l = q[l];
                    
                    //TODO: wut?
                    new_t[ + i].numerator = (t_ik->numerator * t_il->denominator) - (t_il->numerator * t_ik->denominator);
                    new_t[ + i].denominator = t_ik->denominator * t_il->denominator;
                    new_t[ + i].index = i;
                    
                    new_q[].numerator = (q_k->numerator * q_l->denominator) - (q_l->numerator * q_k->denominator);
                    new_q[].denominator = q_k->denominator * q_l->denominator;
                    new_q[].index = 0;
                    
                }
            }
            
            for(j = n2; j < s2; ++j){
                new_t[].numerator = t[i +j].numerator;
                new_t[].denominator = t[i +j].denominator;
                new_t[].index = t[i +j].index;
                
                new_q[].numerator = q[j].numerator;
                new_q[].denominator = q[j].denominator;
                new_q[].index = 0;
            }
        }
        
        
        //TODO: move up
        r--;
        s = s2;
        */
        
        /*
        fm_poly_entry *new_t = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*r*s);
        fm_poly_entry *new_q = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*s);
        fm_row *new_rows = (fm_row*) malloc(sizeof(fm_row)*s);
        
        for(j = 0; j < s; ++j){
            fm_poly *lesser_poly  = (fm_poly*) malloc(sizeof(fm_poly));
            fm_poly *greater_poly = (fm_poly*) malloc(sizeof(fm_poly));
            
            fm_poly_entry *lesser_poly_entries  = (fm_poly_entry*) malloc(sizeof(fm_poly_entry)*r);
            fm_poly_entry *greater_poly_entries = (fm_poly_entry*) malloc(sizeof(fm_poly_entry));
            
            lesser_poly->poly_len = r;
            greater_poly->poly_len = 1;
            
            lesser_poly->poly = lesser_poly_entries;
            greater_poly->poly = greater_poly_entries;
            
            new_rows[j].lesser  = lesser_poly;
            new_rows[j].greater = greater_poly;

            //memcpy(&(greater_poly->poly[0]), &(q[j]), sizeof(fm_poly_entry));
            greater_poly->poly[0].numerator = q[j].numerator;
            greater_poly->poly[0].denominator = q[j].denominator;
            greater_poly->poly[0].index = q[j].index;
            
            //memcpy(&(new_q[j]), &(q[j]), sizeof(fm_poly_entry));
            new_q[j].numerator = q[j].numerator;
            new_q[j].denominator = q[j].denominator;
            new_q[j].index = q[j].index;
            
            for(i = 0; i < r; ++i){
                //memcpy(&(lesser_poly->poly[0]), &(t[i+j]), sizeof(fm_poly_entry));
                lesser_poly->poly[0].numerator = t[i+j].numerator;
                lesser_poly->poly[0].denominator = t[i+j].denominator;
                lesser_poly->poly[0].index = t[i+j].index;
            
                //memcpy(&(new_t[i+j]), &(t[i+j]), sizeof(fm_poly_entry));
                new_t[i+j].numerator = t[i+j].numerator;
                new_t[i+j].denominator = t[i+j].denominator;
                new_t[i+j].index = t[i+j].index;
            }
            printf("ROW: ");
            print_row(&(new_rows[j]));
        }
        
        //TODO: Free old t&&q
        //t = new_t;
        //q = new_q;
        
        printf("SYSTEM #1?\n");
        print_system(system);
        
        system->nbr_rows = r;
        system->curr_nbr_x -= 1;
        system->rows = new_rows;
        
        printf("SYSTEM #2?\n");
        print_system(system);
        */
        
    }

    
    
    

    
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
    elim_2(system);

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
