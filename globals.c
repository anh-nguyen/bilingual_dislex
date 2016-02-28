/* File: globals.c
 *
 * Defines global variables for the DISLEX system.
 *
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: globals.c,v 1.6 1994/09/20 10:46:35 risto Exp risto $
 */

#ifdef DEFINE_GLOBALS		/* Defined in the main.c file */
/* these definitions are in effect in main.c */


/************ word representations  *************/

/* lexical and semantic lexica (for checks); index [-1] is used for blank */
WORDSTRUCT l1words[MAXWORDS],	/* L1 lexical words */
  l2words[MAXWORDS],  /* L2 lexical words */
  swords[MAXWORDS];		/* semantic words */
int nl1words, nl2words, nswords,		/* number of lexical and semantic words */
  nl1rep, nl2rep, nsrep;			/* dimension of lexical and semantic reps */
int instances[MAXWORDS];	/* indices of the instance words */
int ninstances;			/* number of instances */

/***************** input data *****************/

PAIRSTRUCT pairs[MAXPAIRS];	/* corresponding lexical and semantic indices*/
int npairs;			/* number of word pairs */
int shuffletable[MAXPAIRS];	/* order of wordpair presentations */

/***************** lexicon maps *****************/

int nl1net, nl2net, nsnet;		/* size (side) of lex and sem maps*/

FMUNIT l1units[MAXLSNET][MAXLSNET], /* L1 feature map */
  l2units[MAXLSNET][MAXLSNET], /* L2 feature map */
  sunits[MAXLSNET][MAXLSNET];	/* semantic feature map */

double
  l1sassoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], /* L1->sem assoc */
  sl1assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], /* sem->L1 assoc */
  l2sassoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], /* L2->sem assoc */
  sl2assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], /* sem->L2 assoc */
  l1l2assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], /* L1->L2 assoc */
  l2l1assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET]; /* L2->L1 assoc */


/************ simulation management *************/

/* current input and output */
int input[NMODULES],		/* index of current input */
  target[NMODULES];		/* index of current target*/
double
  inprep[NMODULES][MAXREP],	/* input vector */
  outrep[NMODULES][MAXREP],	/* output vector */
  tgtrep[NMODULES][MAXREP];	/* target representation */
int ninprep[NMODULES],		/* input vector dimension */
  noutrep[NMODULES];		/* output vector dimension */

/* learning rates, neighborhoods, running */
double
  l1_exposure,  /* L1 exposure parameter */
  l2_exposure,  /* L2 exposure parameter */
  l1_alpha,			/* L1 map learning rate */
  l2_alpha,     /* L2 map learning rate */
  sem_alpha,			/* semantic map learning rate */
  l1l2_assoc_alpha,			/* associative connections learning rate */
  sl1_assoc_alpha,      /* associative connections learning rate */
  sl2_assoc_alpha;      /* associative connections learning rate */
int
  l1_nc,			/* L1 neighborhood size */
  l2_nc,      /* L2 neighborhood size */
  sem_nc,			/* sem neighborhood size */
  l1_running,			/* whether L1 is running this phase */
  l2_running,     /* whether L2 is running this phase */
  sem_running,			/* whether sem is running this phase */
  l1l2_assoc_running,		/* whether L1 to L2 assoc is running this phase */
  sl1_assoc_running,    /* whether Sem to L1 assoc is running this phase */
  sl2_assoc_running;    /* whether Sem to L2 assoc is running this phase */

/* simulation flags */
int simulator_running,		/* flag: process events or run */
  stepping,			/* stop after every major propagation */
  displaying,			/* whether X display is up */
  testing,			/* testing or training */
  seed,				/* random number seed */
  shuffling,			/* shuffled or fixed sentence order */
  npropunits_given,		/* whether number of propunits is fixed */
  simulationendepoch;		/* last epoch for the simulation */
int npropunits;			/* number of propagation units */

/* file names */
char 
  l1repfile[MAXFILENAMEL + 1],	/* L1 representations */
  l2repfile[MAXFILENAMEL + 1],  /* L2 representations */
  srepfile[MAXFILENAMEL + 1],	/* semantic representations */
  simufile[MAXFILENAMEL + 1],	/* lexicon simulation file */
  current_inpfile[MAXFILENAMEL + 1];	/* wordpair input file */


/******************  graphics  **********************/

/* graphics data */
XtAppContext app_con;		/* application context */
Display *theDisplay;		/* display pointer */
Visual *visual;			/* type of display */
Colormap colormap;		/* colormap definition */
Widget main_widget, form;	/* main widget and organization */
NETSTRUCT net[NMODULES];	/* outline of network geometry etc. */
RESOURCE_DATA data;		/* resource data structure */


/************ end definitions *************/

#else
/* these definitions are in effect in all other files except main.c */

extern WORDSTRUCT l1words[MAXWORDS], l2words[MAXWORDS], swords[MAXWORDS];
extern int nl1words, nl2words, nswords,
  nl1rep, nl2rep, nsrep;
extern int instances[MAXWORDS];
extern int ninstances;
extern PAIRSTRUCT pairs[MAXPAIRS];
extern int npairs;
extern int shuffletable[MAXPAIRS];
extern int 
  input[NMODULES],
  target[NMODULES];
extern double
  inprep[NMODULES][MAXREP],
  outrep[NMODULES][MAXREP],
  tgtrep[NMODULES][MAXREP];
extern int ninprep[NMODULES],
  noutrep[NMODULES];
extern char
  l1repfile[MAXFILENAMEL + 1],
  l2repfile[MAXFILENAMEL + 1],
  srepfile[MAXFILENAMEL + 1],
  simufile[MAXFILENAMEL + 1],
  current_inpfile[MAXFILENAMEL + 1];
extern double
  l1_exposure,
  l2_exposure,
  l1_alpha,
  l2_alpha,
  sem_alpha,  
  l1l2_assoc_alpha, 
  sl1_assoc_alpha,
  sl2_assoc_alpha;  
extern int
  l1_nc,
  l2_nc,
  sem_nc,
  l1_running,
  l2_running,
  sem_running,
  l1l2_assoc_running,
  sl1_assoc_running,
  sl2_assoc_running;
extern int
  displaying,
  simulator_running,
  stepping,
  testing,
  seed,
  shuffling,
  npropunits_given,
  simulationendepoch;
extern int npropunits;
extern XtAppContext app_con;
extern Display *theDisplay;
extern Visual *visual;
extern Colormap colormap;
extern Widget main_widget, form;
extern NETSTRUCT net[NMODULES];
extern RESOURCE_DATA data;
extern int nl1net, nl2net, nsnet;
extern FMUNIT l1units[MAXLSNET][MAXLSNET],
  l2units[MAXLSNET][MAXLSNET],
  sunits[MAXLSNET][MAXLSNET];
extern double
  l1sassoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], 
  sl1assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET],
  l2sassoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], 
  sl2assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET], 
  l1l2assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET],
  l2l1assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET];

#endif /*  #ifdef DEFINE_GLOBALS */
