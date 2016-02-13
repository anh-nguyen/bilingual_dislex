/* File: stats.c
 *
 * Statistics initialization, collection, and output routines for DISLEX.
 *
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: stats.c,v 1.3 1998/09/28 03:30:07 risto Exp risto $
 */

#include <stdio.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "defs.h"
#include "globals.c"


/************ statistics constants *************/

#define WITHINERR 0.15		/* close enough to be counted correct */


/******************** function prototypes ************************/

/* global functions */
#include "prototypes.h"

/* functions local to this file */
static double reszero __P ((double num, int den));


/******************** static variables ************************/

/* statistics */
static int 
  nout[NMODULES],		/* number of times output was counted */
  within[NMODULES],		/* units close enough */
  all[NMODULES][MAXWORDS],	/* # of all words output */
  corr[NMODULES][MAXWORDS];	/* # of words output correctly */
static double deltasum[NMODULES]; /* cumulative error sum */


/*********************  initializations ******************************/

void
init_stats ()
/* initialize the statistics tables */
{
  int modi,			/* module number */
    i;

  /* separate stats for parsing+paraphrasing stories and question answering */
  for (modi = 0; modi < NMODULES; modi++)
    {
      nout[modi] = 0;
      deltasum[modi] = 0.0;
      if (testing)
	{
	  within[modi] = 0;
	  /* there is a different number of lexical vs. semantic words */
	  for (i = 0;
	       i < ((modi == SINPMOD || modi == SOUTMOD) ? nswords : ((modi == L1INPMOD || modi == L1OUTMOD) ? nl1words : nl2words));
	       i++)
	    corr[modi][i] = all[modi][i] = 0;
	}
    }
}


/*********************  cumulation ******************************/

void
collect_stats (modi)
/* the main routine for accumulating performance statistics */
/* all modules call up this routine after propagation */
     int modi;			/* module number */
{
  int i, nearest;
  double uniterror;		/* error of this unit */
  WORDSTRUCT *words;		/* lexicon (lexical or semantic) */
  int nrep;			/* rep dimension (lexical or semantic) */
  int nwords;			/* number of words (lexical or semantic) */

  /* first select the right lexicon */
  select_lexicon (modi, &words, &nrep, &nwords);
  
  /* cumulate error for each output unit */
  for (i = 0; i < noutrep[modi]; i++)
    {
      /* calculate the absolute error of this unit */
      uniterror = fabs (tgtrep[modi][i] - outrep[modi][i]);
      /* cumulate error (for average) */
      deltasum[modi] += uniterror;
      if (testing)
	/* cumulate number of "correct" units (close enough to correct) */
	if (uniterror < WITHINERR)
	  within[modi]++;
    }

  if (!testing)
    /* cumulate output count */
    nout[modi]++;
  else
    {
      /* cumulate word-based error counts */
      all[modi][target[modi]]++;	/* cumulate total number of words */
      /* find the representation in the lexicon closest to the output */
      nearest = find_nearest (outrep[modi], words, nrep, nwords);
      if (nearest == target[modi])
	/* it was the right one; update the correct count for this word */
	corr[modi][target[modi]]++;
    }
}


/*********************  results  ******************************/

void
write_error (fp)
    FILE *fp;
{
  int modi;			/* module number */
  
  for (modi = 0; modi < NMODULES; modi++)
    fprintf (fp, " %f", reszero (deltasum[modi], nout[modi] * noutrep[modi]));
  fprintf (fp, "\n");
}


void
print_stats (epoch)
/* print out the performance statistics in the log output */
    int epoch;			/* current epoch number */
{
  int modi,			/* module number */
    nwords, nrep,		/* number of words and rep components */
    i,
    sum_corr, sum_all,		/* number of correct and all words */
    sum_corrinst, sum_allinst;	/* number of correct and allinstances */

  printf ("\n%s (%d pairs), epoch %d\n", current_inpfile, npairs, epoch);
  printf ("Module        All   Inst  <%.2f    AvgErr\n", WITHINERR);
  /* print all modules separately */
  for (modi = 0; modi < NMODULES; modi++)
    {
      /* first set the dimensions properly (lexical or semantic) */
      nwords = (modi == SINPMOD || modi == SOUTMOD) ? nswords : ((modi == L1INPMOD || modi == L1OUTMOD) ? nl1words : nl2words);
      nrep = (modi == SINPMOD || modi == SOUTMOD) ? nsrep : ((modi == L1INPMOD || modi == L1OUTMOD) ? nl1rep : nl2rep);
      sum_corr = sum_all = sum_corrinst = sum_allinst = 0;
      /* sum up all word counts and all correct word counts */
      for (i = 0; i < nwords; i++)
	{
	  sum_corr += corr[modi][i];
	  sum_all += all[modi][i];
	}
      /* sum up all instance word counts */
      for (i = 0; i < ninstances; i++)
	{
	  sum_corrinst += corr[modi][instances[i]];
	  sum_allinst += all[modi][instances[i]];
	}
      printf ("Module %2d: ", modi);
      printf ("%6.1f %6.1f %6.1f %9.4f\n",
	      /* percentage of correct words */
	      reszero (100.0 * sum_corr, sum_all),
	      /* percentage of correct instances */
	      reszero (100.0 * sum_corrinst, sum_allinst),
	      /* percentage of output units close enough */
	      reszero (100.0 * within[modi], sum_all * nrep),
	      /* average error per output unit */
	      reszero (deltasum[modi], sum_all * nrep));
    }
}


static double
reszero (num, den)
/* if no data was collected, should print 0 instead of crashing */
     double num;			/* numerator */
     int den;				/* denominator */
{
  if (den == 0)
    return (0.0);
  else
    return (num / (double) den);
}
