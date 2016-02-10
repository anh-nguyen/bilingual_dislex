/* File: prototypes.h
 *
 * Prototypes for global functions.
 *
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: prototypes.h,v 1.5 1994/09/19 05:46:46 risto Exp risto $
 */

/********* defined in main.c *************/
int find_nearest __P((double rep[], WORDSTRUCT words[],
		      int nrep, int nwords));
double distance __P((int *foo, double v1[], double v2[], int nrep));
int select_lexicon __P ((int modi, WORDSTRUCT **words,
			 int *nrep, int *nwords));
int fsmaller __P((double x, double y));
int fgreater __P((double x, double y));


/********** defined in nets.c *********/
void iterate_pairs __P((void));
void distanceresponse __P((FMUNIT *unit, double inpvector[], int ninpvector,
			   double (*responsefun)
			   (int *, double *, double *, int),
			   int indices[]));
void updatebestworst __P((double *best, double *worst, int *besti,
			  int *bestj, FMUNIT *unit, int i, int j,
			  int (*better) (double, double),
			  int (*worse) (double, double)));
void iterate_weights __P((void (*dofun) (FILE *, double *, double, double),
			  FILE *fp, double par1, double par2));
void normalize_all_assocweights __P ((void));
void clear_values __P((FMUNIT units[MAXLSNET][MAXLSNET],
		       int nnet));
void clear_prevvalues __P((FMUNIT units[MAXLSNET][MAXLSNET],
			   int nnet));
void clear_labels __P((FMUNIT units[MAXLSNET][MAXLSNET],
		       int nnet));


/********** defined in stats.c *********/
void init_stats __P((void));
void collect_stats __P((int modi));
void write_error __P((FILE *fp));
void print_stats __P((int epoch));


/********* defined in graph.c ***********/
void display_init __P((void));
void wait_and_handle_events __P((void));
void wait_for_run __P((void));
void init_lex_display __P((int modi, int nnet, int nwords, WORDSTRUCT words[],
			   int nrep, FMUNIT units[MAXLSNET][MAXLSNET]));
void display_lex __P((int modi, FMUNIT units[MAXLSNET][MAXLSNET], int nnet));
void display_error __P((int modi));
