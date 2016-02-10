/* File: defs.h
 *
 * Defines parameters and data structures for the DISLEX system.
 *
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: defs.h,v 1.5 1994/09/20 10:46:35 risto Exp risto $
 */

/*********** size constants *************/

/* These constants define the maximum sizes of tables that hold data
   and the network weights and representations */
#define MAXREP 25		/* max size of lexical & semantic reps */
#define MAXLSNET 20		/* max size of lexicon feature maps (side) */
#define MAXFMLABELS 10		/* max # of displayed labels per map unit
                                   (includes the nearestlabel for each) */
#define MAXWORDS 300		/* max number of lexical & semantic words */
#define MAXPAIRS 300		/* max number of training pairs */
#define MAXPHASE 10		/* max number of phases in simulation */
#define MAXSNAPS 50		/* max number of snapshots */
#define MAXWORDL 30		/* max length of input words (chars) */
#define MAXFILENAMEL 100	/* max length of file names (chars) */
#define MAXSTRL 1000		/* max length of input lines (chars) */

/* system constants (not maxsize but actual size) */
#define NMODULES 4		/* lex and sem output and input */

/*********** module, parameter, return, and range constants *************/

/* Module numbers are used to index data structures where module-specific
   data such as input/output activation, weights, graphics values are kept.
   The processing modules need to be consequtive before memory modules,
   which can appear in any order */
#define LINPMOD 0		/* module # for lexical input map  */
#define SOUTMOD 1		/* module # for semantic output map  */
#define SINPMOD 2		/* module # for semantic input map  */
#define LOUTMOD 3		/* module # for lexical output map  */
#define LEXWINMOD LINPMOD	/* module # for lex window */
#define SEMWINMOD SINPMOD	/* module # for sem window */

/* initialization, index, return code constants */
#define LARGEINT 999999999	/* used to init bestcounts */
#define LARGEINTNEGSTR "-999999999"/* used to init npropunits resource */
#define LARGEFLOAT 999999999.9	/* used to initialize bestvalues */
#define NOPREV (-1)		/* no previous value on the map */
#define TRUE 1
#define FALSE 0
#define NONE (-2)		/* indicates no value; note -1 = BLANKINDEX */
#define NOT_USED 0		/* this parameter is not in use */
#define REQUIRED 1		/* whether to require a keyword */
#define NOT_REQUIRED 0		/* or not */

/* exit codes; 1 is used by the system */
#define EXIT_NORMAL 0
#define EXIT_SIZE_ERROR 2
#define EXIT_DATA_ERROR 3
#define EXIT_COMMAND_ERROR 4
#define EXIT_FILE_ERROR 5
#define EXIT_X_ERROR 6


/*********** data types *************/

/* the lexicon data structure: an array of word strings and reps */
typedef struct WORDSTRUCT
  {
    char chars[MAXWORDL + 1];
    double rep[MAXREP];
  }
WORDSTRUCT;

/* definition for feature map units */
typedef struct FMUNIT
  {
    /* previous value is needed to see if display needs updating */
    float value,
      prevvalue,		/* activation value, previous activ. */
      bestvalue;		/* needed for finding the nearest input */
    double comp[MAXREP];	/* input weights */
    int labelcount;		/* how many inputs mapped on the unit */
    char labels[MAXFMLABELS][MAXWORDL + 1];/* the labels of those inputs */
  }
FMUNIT;

/* the word pair structure: index of the lexical
   and the corresponding semantic word */
typedef struct PAIRSTRUCT
  {
    int lindex, sindex;
  }
PAIRSTRUCT;


/************ graphics definitions *************/

/* geometry of a network display */
typedef struct NETSTRUCT
  {
    int
      width,			/* window width and height */
      height,
      uwidth,			/* unit width and height */
      uhght,
      maxlabels,		/* max number of labels that fit */
      marg;			/* left margin */
    char log[MAXSTRL + 1];	/* current I/O item and error */
  }
NETSTRUCT;

/* the resource data for the X display */
typedef struct RESOURCE_DATA
  {
    Boolean bringupdisplay,	/* whether to start display */
      nearestlabels;		/* whether to display nearestlabels */
    int delay;			/* seconds to sleep in major simulation steps*/
    float reversevalue;		/* threshold for reversing color */
    Dimension netwidth,		/* width of network widgets */
      lexnetheight,		/* height of the lexical map window */
      semnetheight;		/* height of the semantic map window */
    Boolean owncmap;		/* use private colormap */
    Pixel textColor,		/* color of the text on display */
      netColor;			/* color of network outlines */
    String commandfont,		/* command line */
      titlefont,		/* network title */
      logfont,			/* I/O item and error */
      lexfont,
      semfont;			/* lexical and semantic map labels */
    Boolean help,		/* user help command line option */
      testing;			/* testing/training mode command line option */
    int npropunits;		/* number of propagation units option */
  }
RESOURCE_DATA, *RESOURCE_DATA_PTR;


/************ funtion prototype macros *************/

#if __STDC__
#define __P(a) a
#else
#ifndef __P			/* Sun ANSI compiler defines this */
#define __P(a) ()
#endif
#endif
