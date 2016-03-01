/* File: main.c
 *
 * Main lexical processing loop, simulation management, and I/O for 
 *  the DISLEX lexicon model.
 *
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: main.c,v 1.14 1994/09/20 10:46:35 risto Exp risto $
 */

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Cardinals.h>

#include "defs.h"
#define DEFINE_GLOBALS		/* so global variables get defined here only */
#include "globals.c"


/************ simu, input, option, and parameter keyword strings *************/

/* name defaults */
#define APP_CLASS "Dislex"		/* class of this application */
#define DEFAULT_SIMUFILENAME "simu"	/* simulation file */
#define SEMANTIC_KEYWORD "semantic"	/* name of semantic reps */
#define L1_KEYWORD "l1"	/* name of L1 reps */
#define L2_KEYWORD "l2" /* name of L2 reps */

/* keywords in the simulation specification file */
#define SIMU_INPUTFILE "inputfile"
#define SIMU_L1REPFILE "l1repfile"
#define SIMU_L2REPFILE "l2repfile"
#define SIMU_SREPFILE "srepfile"
#define SIMU_L1MAPSIZE "l1mapsize"
#define SIMU_L2MAPSIZE "l2mapsize"
#define SIMU_SMAPSIZE "smapsize"
#define SIMU_L1_EXPOSURE "l1exposure"
#define SIMU_L2_EXPOSURE "l2exposure"
#define SIMU_SEED "seed"
#define SIMU_SHUFFLING "shuffling"
#define SIMU_SIMULATIONENDEPOCH "simulationendepoch"
#define SIMU_SNAPSHOTEPOCHS "snapshotepochs"
#define SIMU_PHASE_FIRSTEPOCHS "phase-firstepochs"
#define SIMU_L1_ALPHAS "l1-alphas"
#define SIMU_L2_ALPHAS "l2-alphas"
#define SIMU_SEM_ALPHAS "sem-alphas"
#define SIMU_L1L2_ASSOC_ALPHAS "l1l2-assoc-alphas"
#define SIMU_SL1_ASSOC_ALPHAS "sl1-assoc-alphas"
#define SIMU_SL2_ASSOC_ALPHAS "sl2-assoc-alphas"
#define SIMU_L1_NCS "l1-ncs"
#define SIMU_L2_NCS "l2-ncs"
#define SIMU_SEM_NCS "sem-ncs"
#define SIMU_L1_RUNNING "l1-running"
#define SIMU_L2_RUNNING "l2-running"
#define SIMU_SEM_RUNNING "sem-running"
#define SIMU_L1L2_ASSOC_RUNNING "l1l2-assoc-running"
#define SIMU_SL1_ASSOC_RUNNING "sl1-assoc-running"
#define SIMU_SL2_ASSOC_RUNNING "sl2-assoc-running"
#define SIMU_EPOCH "epoch"
#define SIMU_NETWORK_ERRORS "network-errors"
#define SIMU_NETWORK_WEIGHTS "network-weights"

/* keywords in the representation files */
#define REPS_INST "instances"
#define REPS_REPS "word-representations"
#define REPS_REPSIZE "nwordrep"

/* keywords in the input file */
#define INP_SYMBOL_CONCEPT_PAIRS "symbol-concept-pairs"
#define INP_NONE "_"

/* option keywords */
#define OPT_HELP "-help"
#define OPT_TEST "-test"
#define OPT_TRAIN "-train"
#define OPT_NPROPUNITS "-npropunits"
#define OPT_GRAPHICS "-graphics"
#define OPT_NOGRAPHICS "-nographics"
#define OPT_NEARESTLABELS "-nearestlabels"
#define OPT_NONEARESTLABELS "-nonearestlabels"
#define OPT_OWNCMAP "-owncmap"
#define OPT_NOOWNCMAP "-noowncmap"
#define OPT_DELAY "-delay"

/* ranges for weight values */
#define WEIGHTLOW 0.0
#define WEIGHTHIGH 1.0
#define WEIGHTSPAN (WEIGHTHIGH - WEIGHTLOW)


/******************** Function prototypes ************************/

/* global functions */
#include "prototypes.h"
extern double drand48 __P ((void));
extern void srand48 __P ((long seed));
extern long lrand48 __P ((void));
extern int strcasecmp __P ((const char *s1, const char *s2));

/* functions local to this file */
static void run_simulation __P ((void));
static void run_simulation_once __P ((void));
static void read_and_process_snapshots __P ((int *epoch));
static void test_snapshot __P ((int epoch));
static void training __P ((int epoch));
static void create_toplevel_widget __P ((int argc, char **argv));
static void process_remaining_arguments __P ((int argc, char **argv));
static void process_display_options __P ((int argc, char **argv));
static void process_nodisplay_options __P ((int *argc, char **argv));
static char *get_option __P ((XrmDatabase db,
			      char *app_name, char *app_class,
			      char *res_name, char *res_class));
static void usage __P ((char *app_name));
static void init_system __P ((void));
static void read_params __P ((FILE * fp));
static void reps_init __P((char *lexsem, char *repfile, WORDSTRUCT words[],
			   int *nwords, int *nrep));
static void read_input_pairs __P ((FILE * fp));
static int list2indices __P((int itemarray[], char rest[],
			     int maxitems, char listname[]));
static int text2floats __P((double itemarray[], int nitems, char nums[]));
static int text2ints __P((int itemarray[], int nitems, char nums[]));
static int wordindex __P ((char wordstring[], WORDSTRUCT words[], int nwords));
static int read_till_keyword __P ((FILE * fp, char keyword[], int required));
static char *rid_sspace __P ((char rest[]));
static void rid_fspace __P ((FILE * fp));
static void fgetline __P ((FILE * fp, char *s, int lim));
static void fgl __P ((FILE * fp));
static int open_file __P((char *filename, char *mode, FILE **fp,int required));
static void readfun __P ((FILE * fp, double *place, double par1, double par2));
static void writefun __P ((FILE * fp, double *place, double par1, double par2));
static void randfun __P ((FILE * fp, double *place, double low, double span));
static void get_current_params __P ((int epoch));
static double current_alpha __P ((int epoch, int phase, int phaseends[],
				  double alphas[]));
static int current_nc __P ((int epoch, int phase, int phaseends[], int ncs[]));
static void save_current __P ((int epoch));
static void update_nextsnapshot __P ((int epoch));
static void shuffle __P ((void));
static double f01rnd __P ((void));


/******************** static variables ******************** */

static int _phaseends[MAXPHASE + 1],	/* phase end epochs */
  *phaseends,			/* actual pointer; -1 is for initial */
  snapshots[MAXSNAPS];		/* snapshot epochs */
static int nphase,		/* number of phases */
  nsnaps,			/* number of snapshots */
  nextsnapshot;			/* index of the next snapshot */
static double
  l1_a[MAXPHASE + 1], *l1_alphas, 	/* L1 alphas for each phase */
  l2_a[MAXPHASE + 1], *l2_alphas,   /* L2 alphas for each phase */
  s_a[MAXPHASE + 1], *sem_alphas,	/* semmap alphas for each phase */
  l1l2_a_a[MAXPHASE + 1], *l1l2_assoc_alphas,	/* assoc alphas for each phase */
  sl1_a_a[MAXPHASE + 1], *sl1_assoc_alphas, /* assoc alphas for each phase */
  sl2_a_a[MAXPHASE + 1], *sl2_assoc_alphas; /* assoc alphas for each phase */
static int
  l1_n[MAXPHASE + 1], *l1_ncs,	/* L1 neighborhood sizes per phase */
  l2_n[MAXPHASE + 1], *l2_ncs,  /* L2 neighborhood sizes per phase */
  s_n[MAXPHASE + 1], *sem_ncs,	/* sem neighborhood sizes per phase */
  l1_runnings[MAXPHASE],	/* whether L1 is running this phase */
  l2_runnings[MAXPHASE],  /* whether L2 is running this phase */
  sem_runnings[MAXPHASE],	/* whether sem is running this phase */
  l1l2_assoc_runnings[MAXPHASE],	/* whether L1 L2 assoc is running this phase */
  sl1_assoc_runnings[MAXPHASE], /* whether Sem L1 assoc is running this phase */
  sl2_assoc_runnings[MAXPHASE]; /* whether Sem L2 assoc is running this phase */

/* define the geometry of the display */
static String fallback_resources[] =
{
  "*runstop.left: 	ChainLeft",
  "*runstop.right: 	ChainLeft",
  "*runstop.top:	ChainTop",
  "*runstop.bottom:	ChainTop",
  "*step.fromHoriz: 	runstop",
  "*step.left:	 	ChainLeft",
  "*step.right: 	ChainLeft",
  "*step.top:		ChainTop",
  "*step.bottom:	ChainTop",
  "*clear.fromHoriz: 	step",
  "*clear.left: 	ChainLeft",
  "*clear.right: 	ChainLeft",
  "*clear.top:		ChainTop",
  "*clear.bottom:	ChainTop",
  "*quit.fromHoriz: 	clear",
  "*quit.left:	 	ChainLeft",
  "*quit.right: 	ChainLeft",
  "*quit.top:		ChainTop",
  "*quit.bottom:	ChainTop",
  "*command.fromHoriz: 	quit",
  "*command.left: 	ChainLeft",
  "*command.right: 	ChainRight",
  "*command.top:	ChainTop",
  "*command.bottom:	ChainTop",
  "*l1.fromVert: 	runstop",
  "*l1.top:		ChainTop",
  "*l2.fromVert:  l1",
  "*sem.fromVert: 	l2",

  /* define the color defaults */
  "*foreground:	        white",
  "*background:		black",
  "*borderColor:	white",

  NULL
};

/* these are the possible command line options */
static XrmOptionDescRec options[] = 
{
  {OPT_HELP, ".help", XrmoptionNoArg, "true"},
  {OPT_TEST, ".testing", XrmoptionNoArg, "true"},
  {OPT_TRAIN, ".testing", XrmoptionNoArg, "false"},
  {OPT_NPROPUNITS, ".npropunits", XrmoptionSepArg, NULL},
  {OPT_GRAPHICS, ".bringupDisplay", XrmoptionNoArg, "true"},
  {OPT_NOGRAPHICS, ".bringupDisplay", XrmoptionNoArg, "false"},
  {OPT_NEARESTLABELS, ".nearestlabels", XrmoptionNoArg, "true"},
  {OPT_NONEARESTLABELS, ".nearestlabels", XrmoptionNoArg, "false"},
  {OPT_OWNCMAP, ".owncmap", XrmoptionNoArg, "true"},
  {OPT_NOOWNCMAP, ".owncmap", XrmoptionNoArg, "false"},
  {OPT_DELAY, ".delay", XrmoptionSepArg, NULL},
};

/* the default values for the application-specific resources;
   see defs.h for component descriptions */
static XtResource resources[] =
{
  {"bringupDisplay", "BringupDisplay", XtRBoolean, sizeof (Boolean),
   XtOffset (RESOURCE_DATA_PTR, bringupdisplay), XtRImmediate,
   (XtPointer) True},
  {"nearestlabels", "Nearestlabels", XtRBoolean, sizeof (Boolean),
   XtOffset (RESOURCE_DATA_PTR, nearestlabels), XtRImmediate,
     (XtPointer) False},
  {"owncmap", "Owncmap", XtRBoolean, sizeof (Boolean),
   XtOffset (RESOURCE_DATA_PTR, owncmap), XtRImmediate, (XtPointer) False},
  {"delay", "Delay", XtRInt, sizeof (int),
   XtOffset (RESOURCE_DATA_PTR, delay), XtRString, "0"},

  {"netwidth", "Netwidth", XtRDimension, sizeof (Dimension),
   XtOffset (RESOURCE_DATA_PTR, netwidth), XtRString, "768"},
  {"l1netheight", "L1netheight", XtRDimension, sizeof (Dimension),
   XtOffset (RESOURCE_DATA_PTR, l1netheight), XtRString, "348"},
  {"l2netheight", "L2netheight", XtRDimension, sizeof (Dimension),
   XtOffset (RESOURCE_DATA_PTR, l2netheight), XtRString, "348"},
  {"semnetheight", "Semnetheight", XtRDimension, sizeof (Dimension),
   XtOffset (RESOURCE_DATA_PTR, semnetheight), XtRString, "348"},

  {"textColor", "TextColor", XtRPixel, sizeof (Pixel),
   XtOffset (RESOURCE_DATA_PTR, textColor), XtRString, "green"},
  {"netColor", "NetColor", XtRPixel, sizeof (Pixel),
   XtOffset (RESOURCE_DATA_PTR, netColor), XtRString, "red"},
  {"reversevalue", "Reversevalue", XtRFloat, sizeof (float),
   XtOffset (RESOURCE_DATA_PTR, reversevalue), XtRString, "0.3"},

  {"commandfont", "Commandfont", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, commandfont), XtRString, "7x13bold"},
  {"titlefont", "Titlefont", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, titlefont), XtRString, "8x13bold"},
  {"logfont", "Logfont", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, logfont), XtRString, "6x10"},
  {"l1font", "L1font", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, l1font), XtRString, "5x8"},
  {"l2font", "L2font", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, l2font), XtRString, "5x8"},
  {"semfont", "Semfont", XtRString, sizeof (String),
   XtOffset (RESOURCE_DATA_PTR, semfont), XtRString, "5x8"},

  /* command line options */
  {"help", "Help", XtRBoolean, sizeof (Boolean),
   XtOffset (RESOURCE_DATA_PTR, help), XtRImmediate, (XtPointer) False},
  {"testing", "Testing", XtRBoolean, sizeof (Boolean),
   XtOffset (RESOURCE_DATA_PTR, testing), XtRImmediate, (XtPointer) False},
  {"npropunits", "Npropunits", XtRInt, sizeof (int),
   XtOffset (RESOURCE_DATA_PTR, npropunits), XtRString, LARGEINTNEGSTR},
};

/* interrupt handling */
jmp_buf loop_map_env;	/* jump here from after interrupt */


/*********************  main processing loops ******************************/

void
main (argc, argv)
/* initialize X display, system parameters, read inputs, process them */
     int argc;
     char **argv;
{
  int xargc;			/* saved argc & argv */
  char **xargv;
  int i;

  /* save command line args so we can parse them later */
  xargc = argc;
  xargv = (char **) XtMalloc (argc * sizeof (char *));
  for (i = 0; i < argc; i++)
    xargv[i] = argv[i];

  /* try to open the display */
  XtToolkitInitialize ();
  app_con = XtCreateApplicationContext ();
  XtAppSetFallbackResources (app_con, fallback_resources);
  theDisplay = XtOpenDisplay (app_con, NULL, NULL, APP_CLASS,
			      options, XtNumber (options), &argc, argv);
  if (theDisplay != NULL)
    /* create the top-level widget and get application resources */
    create_toplevel_widget(xargc, xargv);
  else
    fprintf (stderr, "No display: running in text mode.\n");
  
  process_remaining_arguments (argc, argv);
  init_system ();
  run_simulation ();
  exit (EXIT_NORMAL);
}


static void
run_simulation ()
/* run once through the simulation if no graphics, otherwise
   multiple times until user hits "quit" */
{
  if (setjmp (loop_map_env))
    {
      /* longjmp gets here */
    }
  else
    {
      /* return from setjmp */
    }
  if (displaying)
    while (TRUE)
      {
	/* if the Xdisplay is up, process events until the user hits "Run" */
	/* user hitting "Quit" will terminate the program */
	wait_for_run ();
	/* user hit "Run"; start processing snapshots */
	run_simulation_once ();
      }
  else
    /* no Xdisplay; iterate snapshots right away */
    run_simulation_once ();
}


static void
run_simulation_once ()
/* iterate once through the snapshots and training if required */
{
  int i,
  epoch = NONE;		/* indicates no snapshots were saved */

  if (!testing)
    {
      /* start a random number sequence */
      srand48 (seed);
      /* always randomize weights again
         so that the random number sequence stays the same */
      iterate_weights (randfun, NULL, WEIGHTLOW, WEIGHTSPAN);
      normalize_all_assocweights ();

      /* read all snapshots, getting current weights and epoch */
      read_and_process_snapshots (&epoch);

      /* update shuffling (so that we can continue where we left off) */
      if (shuffling)
	{
	  /* reinitialize shuffletable */
	  for (i = 0; i < npairs; i++)
	    shuffletable[i] = i;
	  if (epoch != NONE)
	    for (i = 0; i < epoch; i++)
	      shuffle ();
	}

      /* save the initial weights if required */
      nextsnapshot = 0;
      if (epoch == NONE)
	{
	  /* no snapshots read; check if initial one needs to be saved */
	  epoch = 0;
	  if (nsnaps > 0)
	    if (snapshots[nextsnapshot] == 0)
	      save_current (epoch);
	}
      else
	update_nextsnapshot (epoch);

      /* continue training from the next epoch on */
      training (++epoch);
    }
  else
    /* process each snapshot */
    read_and_process_snapshots (&epoch);
}


static void
read_and_process_snapshots (epoch)
/* iterate through all snapshots; test each one if in testing mode */
     int *epoch;		/* return the epoch of the last snapshot */
{
  FILE *fp;			/* pointer to the simufile */
  
  /* reopen the simufile (necessary because the user may want to */
  /* do the same simulation multiple times in one sitting */
  fp = fopen (simufile, "r");
  read_params (fp);

  while (read_till_keyword (fp, SIMU_EPOCH, NOT_REQUIRED))
    {
      /* read the epoch number of snapshot */
      fscanf (fp, "%d", epoch);
      if (displaying)
	{
	  printf ("Reading snapshot %d, hold on...", *epoch);
	  fflush (stdout);
	}
      read_till_keyword (fp, SIMU_NETWORK_WEIGHTS, REQUIRED);
      /* read the current weights */
      iterate_weights (readfun, fp, NOT_USED, NOT_USED);
      if (displaying)
	printf ("Done.\n");
      if (testing)
	/* run through the test set: display and collect stats */
	test_snapshot (*epoch);
    }
  fclose (fp);
}


static void
test_snapshot (epoch)
/* run through all stories and collect statistics about performance */
     int epoch;			/* epoch of the snapshot */
{
  get_current_params (epoch);	/* current learning rates and neighborhoods */
  init_stats ();
  iterate_pairs ();		/* run through the test set */
  print_stats (epoch);		/* performance statistics */
}


static void
training (epoch)
/* train the modules until simulationendepoch */
     int epoch;			/* starting from this epoch */
{
  while (epoch <= simulationendepoch)
    {
      get_current_params (epoch);/* current learning rates and neighborhoods */
      init_stats ();
      if (shuffling)		/* change the order of stories */
	shuffle ();
      printf("before interate_pairs() \n");
      iterate_pairs ();		/* run through the training set */

      printf("after interate_pairs() \n");
      /* log the progress of training error */
      printf ("Epoch %d errors: ", epoch);
      write_error (stdout);
      if (nextsnapshot < nsnaps)
	if (epoch >= snapshots[nextsnapshot])
	  save_current (epoch);
      epoch++;
    }
}


/*********************  initializations ******************************/

/**************** X interface, command line */

static void
create_toplevel_widget (argc, argv)
  /* retrieve resources, create a colormap, and start the top-level widget */
  int argc;
  char **argv;
{
  Widget dummy;			/* dummy top-level widget */
  Arg args[10];
  int scr;			/* temporary screen (for obtaining colormap) */
  int n = 0;			/* argument counter */

  /* Create a dummy top-level widget to retrieve resources */
  /* (necessary to get the right netcolor etc with owncmap) */
  dummy = XtAppCreateShell (NULL, APP_CLASS, applicationShellWidgetClass,
			    theDisplay, NULL, ZERO);
  XtGetApplicationResources (dummy, &data, resources,
			     XtNumber (resources), NULL, ZERO);
  scr = DefaultScreen (theDisplay);
  visual = DefaultVisual (theDisplay, scr);
  
  /* Select colormap; data.owncmap was specified in resources
     or as an option */
  if (data.owncmap)
    {
      colormap = XCreateColormap (theDisplay, DefaultRootWindow (theDisplay),
				  visual, AllocNone);
      XtSetArg (args[n], XtNcolormap, colormap);
      n++;
    }
  else
    colormap = DefaultColormap (theDisplay, scr);
  XtDestroyWidget (dummy);
  
  /* Create the real top-level widget */
  XtSetArg (args[n], XtNargv, argv);
  n++;
  XtSetArg (args[n], XtNargc, argc);
  n++;
  main_widget = XtAppCreateShell (NULL, APP_CLASS, applicationShellWidgetClass,
				  theDisplay, args, n);
  XtGetApplicationResources (main_widget, &data, resources,
			     XtNumber (resources), NULL, ZERO);
}


static void
process_remaining_arguments (argc, argv)
  /* parse nongraphics options, simufile and inputfile, setup displaying */
  int argc;
  char **argv;
{
  int i;

  /* if opendisplay was successful, all options were parsed into "data" */
  /* otherwise, we have to get the options from command-line argument list */
  if (theDisplay != NULL)
    process_display_options (argc, argv);
  else
    process_nodisplay_options (&argc, argv);

  /* figure out the simufile and input file names */
  sprintf (simufile, "%s", "");
  sprintf (current_inpfile, "%s", "");
  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-')
      {
	fprintf (stderr, "Unknown option %s\n", argv[i]);
	usage (argv[0]);
	exit (EXIT_COMMAND_ERROR);
      }
    else if (!strlen (simufile))	/* first argument is simufile */
      sprintf (simufile, "%s", argv[i]);
    else if (!strlen (current_inpfile))	/* second is inputfile */
      sprintf (current_inpfile, "%s", argv[i]);
    else
      {
	fprintf (stderr, "Too many arguments\n");
	usage (argv[0]);
	exit (EXIT_COMMAND_ERROR);
      }
  if (!strlen (simufile))		/* if no simufile given */
    sprintf (simufile, "%s", DEFAULT_SIMUFILENAME);
  /* if no inputfilename is specified, the one in the simufile is used */

  /* decide whether to bring up display */
  if (theDisplay && data.bringupdisplay)
    displaying = TRUE;
  else
    displaying = FALSE;
}


static void
process_display_options (argc, argv)
/* get the non-graphics-related options from the "data" structure */
  int argc;
  char **argv;
{
  /* quick user help */
  if (data.help)
    {
      usage (argv[0]);
      exit (EXIT_NORMAL);
    }
  /* training or testing mode */
  testing = data.testing;

  /* npropunits */
  if (data.npropunits != -LARGEINT)
    {
      npropunits = data.npropunits;
      if (npropunits <= 0)
	{
	  fprintf (stderr, "%s argument should be at least 1\n",
		   OPT_NPROPUNITS);
	  exit (EXIT_COMMAND_ERROR);	  
	}
      if (npropunits <= 0 || npropunits > MAXLSNET * MAXLSNET)
	  npropunits = MAXLSNET * MAXLSNET;
	npropunits_given = TRUE;
      }
  else
    npropunits_given = FALSE;
}


static void
process_nodisplay_options (argc, argv)
/* get the non-graphics-related options from the command string */
  int *argc;
  char **argv;
{
  char *res_str;		/* string value of an option */
  XrmDatabase db = NULL;	/* resource database for options */

  XrmParseCommand (&db, options, XtNumber (options), APP_CLASS, argc, argv);

  /* quick user help */
  res_str = get_option (db, argv[0], APP_CLASS, "help", "Help");
  if (res_str != NULL)
    if (!strcasecmp ("true", res_str))
      {
	usage (argv[0]);
	exit (EXIT_NORMAL);
      }
  
  /* training or testing mode */
  res_str = get_option (db, argv[0], APP_CLASS, "testing", "Testing");
  if (res_str != NULL)
    testing = !strcasecmp ("true", res_str);
  else
    testing = FALSE;

  /* npropunits */
  res_str = get_option (db,argv[0], APP_CLASS, "npropunits", "Npropunits");
  if (res_str != NULL)
    if (sscanf (res_str, "%d", &npropunits) == 1)
      {
	if (npropunits <= 0)
	  {
	    fprintf (stderr, "%s argument should be at least 1\n",
		     OPT_NPROPUNITS);
	    exit (EXIT_COMMAND_ERROR);	  
	  }
	if (npropunits <= 0 || npropunits > MAXLSNET * MAXLSNET)
	  npropunits = MAXLSNET * MAXLSNET;
	npropunits_given = TRUE;
      }
    else
      {
	fprintf (stderr, "Warning: Cannot convert %s to type Int\n", res_str);
	npropunits_given = FALSE;
      }
  else
    npropunits_given = FALSE;
}


static char *
get_option (db, app_name, app_class, res_name, res_class)
  /* return the pointer to the string value of the resource */
  XrmDatabase db;			/* resource database */
  char *res_name, *res_class,		/* resource name and class */
    *app_name, *app_class;		/* application name and class */
{
  XrmValue value;			/* value of the resource */
  char *type,				/* resource type */
    name[MAXSTRL + 1], class[MAXSTRL + 1];/* full name and class of resource */

  sprintf (name, "%s.%s", app_name, res_name);
  sprintf (class, "%s.%s", app_class, res_class);
  XrmGetResource(db, name, class, &type, &value);
  return (value.addr);
}


static void
usage (app_name)
  /* print out the list of options and arguments */
  char *app_name;			/* name of the program */
{
  char s[MAXSTRL + 1], ss[MAXSTRL + 1];

  sprintf(s, "%s %s", OPT_DELAY, "<sec>");
  sprintf(ss, "%s %s", OPT_NPROPUNITS, "<number>");
  fprintf (stderr, "Usage: %s [options] [simulation file] [input file]\n\
where the options are\n\
  %-20s  Prints this message\n\
  %-20s  Testing mode\n\
  %-20s  Training mode (default)\n\
  %-20s  Size of neighborhood (number of units)\n\
  %-20s  Bring up graphics display\n\
  %-20s  Text output only\n\
  %-20s  Show nearest input for each unit\n\
  %-20s  Label only the image units\n\
  %-20s  Use a private colormap\n\
  %-20s  Use the existing colormap\n\
  %-20s  Delay in updating the screen (in seconds)\n",
	   app_name, OPT_HELP, OPT_TEST, OPT_TRAIN, ss,
	   OPT_GRAPHICS, OPT_NOGRAPHICS, OPT_NEARESTLABELS,OPT_NONEARESTLABELS,
	   OPT_OWNCMAP, OPT_NOOWNCMAP, s);
}


/********************* system setup */

static void
init_system ()
/* set up blank word, read simulation parameters and input data */
{
  FILE *fp;			/* pointer to the simufile and inputfile */
  int i;

  /* first fix the pointers to phase data, making room for initial
     values under index -1 */
  phaseends = _phaseends + 1;
  phaseends[-1] = 0;
  l1_alphas = l1_a + 1;
  l2_alphas = l2_a + 1;
  sem_alphas = s_a + 1;
  l1l2_assoc_alphas = l1l2_a_a + 1;
  sl1_assoc_alphas = sl1_a_a + 1;
  sl2_assoc_alphas = sl2_a_a + 1;
  l1_ncs = l1_n + 1;
  l2_ncs = l2_n + 1;
  sem_ncs = s_n + 1;

  /* read simulation parameters from simufile */
  printf ("Initializing DISLEX simulation from %s...\n", simufile);
  open_file (simufile, "r", &fp, REQUIRED);
  read_params (fp);
  fclose (fp);

  /* read the lexical reps */
  reps_init (L1_KEYWORD, l1repfile, l1words, &nl1words, &nl1rep);
  reps_init (L2_KEYWORD, l2repfile, l2words, &nl2words, &nl2rep);

  /* read the semantic reps */
  reps_init (SEMANTIC_KEYWORD, srepfile, swords, &nswords, &nsrep);

  /* read the input data from the input file */
  printf ("Reading word pairs from %s...", current_inpfile);
  fflush (stdout);
  open_file (current_inpfile, "r", &fp, REQUIRED);
  read_input_pairs (fp);
  fclose (fp);
  printf ("%d pairs.\n", npairs);

  /* set up numbers of input/output words and reps for convenience */
  ninprep[L1INPMOD] = noutrep[L1INPMOD] = nl1rep;
  ninprep[L1OUTMOD] = noutrep[L1OUTMOD] = nl1rep;
  ninprep[L2INPMOD] = noutrep[L2INPMOD] = nl2rep;
  ninprep[L2OUTMOD] = noutrep[L2OUTMOD] = nl2rep;
  ninprep[SINPMOD] = noutrep[SINPMOD] = nsrep;
  ninprep[SOUTMOD] = noutrep[SOUTMOD] = nsrep;

  /* initialize the shuffle table */
  for (i = 0; i < npairs; i++)
    shuffletable[i] = i;

  /* initialize graphics */
  if (displaying)
    display_init ();

  /* print out the switch settings so we can recreate the simulation later */
  printf ("Simulation switches: %s",
	  testing ? OPT_TEST : OPT_TRAIN);
  if (npropunits_given)
    printf (" %s %d", OPT_NPROPUNITS, npropunits);
  printf ("\n");
  printf ("System initialization complete.\n");
}


static void
read_params (fp)
/* read the simulation parameters from the simufile:
   read (ignore everything) until the next keyword, read the value
   of the parameter, and check that it makes sense. Make sure
   all keywords are found; abort if not. */
     FILE *fp;			/* pointer to the simufile */
{
  char rest[MAXSTRL + 1];	/* parameter string */
  int begphase[MAXPHASE + 1], 	/* temporarily holds the phasefirstepochs */
    nbegphase,			/* temporarily holds the number of phases */
    i;

  read_till_keyword (fp, SIMU_INPUTFILE, REQUIRED);
  /* read the input file name if it was not given as a command line param */
  if (!strlen(current_inpfile))
    fscanf (fp, "%s", current_inpfile);

  read_till_keyword (fp, SIMU_L1REPFILE, REQUIRED);
  fscanf (fp, "%s", l1repfile);

  read_till_keyword (fp, SIMU_L2REPFILE, REQUIRED);
  fscanf (fp, "%s", l2repfile);

  read_till_keyword (fp, SIMU_SREPFILE, REQUIRED);
  fscanf (fp, "%s", srepfile);

  read_till_keyword (fp, SIMU_L1MAPSIZE, REQUIRED);
  fscanf (fp, "%d", &nl1net);
  if (nl1net > MAXLSNET || nl1net <= 0)
    {
      fprintf (stderr, "%s exceeds array size\n", SIMU_L1MAPSIZE);
      exit (EXIT_SIZE_ERROR);
    }

  read_till_keyword (fp, SIMU_L2MAPSIZE, REQUIRED);
  fscanf (fp, "%d", &nl2net);
  if (nl2net > MAXLSNET || nl2net <= 0)
    {
      fprintf (stderr, "%s exceeds array size\n", SIMU_L2MAPSIZE);
      exit (EXIT_SIZE_ERROR);
    }

  read_till_keyword (fp, SIMU_SMAPSIZE, REQUIRED);
  fscanf (fp, "%d", &nsnet);
  if (nsnet > MAXLSNET || nsnet <= 0)
    {
      fprintf (stderr, "%s exceeds array size\n", SIMU_SMAPSIZE);
      exit (EXIT_SIZE_ERROR);
    }

  read_till_keyword (fp, SIMU_L1_EXPOSURE, REQUIRED);
  fscanf (fp, "%lf", &l1_exposure);

  read_till_keyword (fp, SIMU_L2_EXPOSURE, REQUIRED);
  fscanf (fp, "%lf", &l2_exposure);

  read_till_keyword (fp, SIMU_SEED, REQUIRED);
  fscanf (fp, "%d", &seed);

  read_till_keyword (fp, SIMU_SHUFFLING, REQUIRED);
  fscanf (fp, "%d", &shuffling);

  read_till_keyword (fp, SIMU_SIMULATIONENDEPOCH, REQUIRED);
  fscanf (fp, "%d", &simulationendepoch);

  read_till_keyword (fp, SIMU_SNAPSHOTEPOCHS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  /* convert the string to numbers and load */
  /* as a side effect, establish the number of snapshots */
  nsnaps = text2ints (snapshots, MAXSNAPS, rest);
  if (nsnaps > MAXSNAPS)
    {
      fprintf (stderr, "Number of %s exceeds array size\n",
	       SIMU_SNAPSHOTEPOCHS);
      exit (EXIT_SIZE_ERROR);
    }

  read_till_keyword (fp, SIMU_PHASE_FIRSTEPOCHS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  /* as a side effect, establish the number of phases */
  nbegphase = text2ints (begphase, MAXPHASE, rest);
  if (nbegphase > MAXPHASE + 1)
    {
      fprintf (stderr, "Number of %s exceeds array size\n",
	       SIMU_PHASE_FIRSTEPOCHS);
      exit (EXIT_SIZE_ERROR);
    }
  if (begphase[nbegphase - 1] < simulationendepoch + 1)
    {
      fprintf (stderr, "%s must be less than the last %s\n",
	       SIMU_SIMULATIONENDEPOCH, SIMU_PHASE_FIRSTEPOCHS);
      exit (EXIT_DATA_ERROR);
    }
  /* convert firstepochs into lastepochs */
  for (i = 1; i < nbegphase; i++)
    phaseends[i -1] = begphase[i] - 1;
  nphase = nbegphase -1;

  read_till_keyword (fp, SIMU_L1_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&l1_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_L1_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L2_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&l2_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_L2_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SEM_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&sem_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_SEM_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L1L2_ASSOC_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&l1l2_assoc_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_L1L2_ASSOC_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SL1_ASSOC_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&sl1_assoc_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_SL1_ASSOC_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SL2_ASSOC_ALPHAS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2floats (&sl2_assoc_alphas[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_SL2_ASSOC_ALPHAS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L1_NCS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (&l1_ncs[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_L1_NCS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L2_NCS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (&l2_ncs[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_L2_NCS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SEM_NCS, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (&sem_ncs[-1], MAXPHASE + 1, rest) != nphase + 1)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_SEM_NCS);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L1_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (l1_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_L1_RUNNING);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L2_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (l2_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_L2_RUNNING);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SEM_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (sem_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_SEM_RUNNING);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_L1L2_ASSOC_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (l1l2_assoc_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
	       SIMU_L1L2_ASSOC_RUNNING);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SL1_ASSOC_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (sl1_assoc_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_SL1_ASSOC_RUNNING);
      exit (EXIT_DATA_ERROR);
    }

  read_till_keyword (fp, SIMU_SL2_ASSOC_RUNNING, REQUIRED);
  fgetline (fp, rest, MAXSTRL);
  if (text2ints (sl2_assoc_runnings, MAXPHASE, rest) != nphase)
    {
      fprintf (stderr, "Number of %s does not match the number of phases\n",
         SIMU_SL2_ASSOC_RUNNING);
      exit (EXIT_DATA_ERROR);
    }
}


/********************* inputfile */

static void
read_input_pairs (fp)
/* read the word pair data, make sure the array sizes are not exceeded */
     FILE *fp;			/* pointer to the inputfile */
{
  int i = 0;
  char rest[MAXSTRL + 1],	/* holds one line of input data */
  l1word[MAXSTRL + 1],		/* L1 word read */
  l2word[MAXSTRL + 1],    /* L2 word read */
  sword[MAXSTRL + 1];		/* semantic word read */

  /* find the keyword and get rid of the blanks */
  read_till_keyword (fp, INP_SYMBOL_CONCEPT_PAIRS, REQUIRED);
  rid_fspace (fp);
  /* then read the input pairs */
  /* read the first line */
  for (fgetline (fp, rest, MAXSTRL);
       strlen (rest);
       fgetline (fp, rest, MAXSTRL))
    {
      if (i >= MAXPAIRS)
	{
	  fprintf (stderr, "Number of input pairs exceeds array size\n");
	  exit (EXIT_SIZE_ERROR);
	}

      if (sscanf (rest, "%s %s %s", l1word, l2word, sword) != 3)
	{
	  fprintf (stderr, "Wrong number of words in an input pair\n");
	  exit (EXIT_DATA_ERROR);
	}
      if (strcasecmp (l1word, INP_NONE))
	pairs[i].l1index = wordindex (l1word, l1words, nl1words);
      else
	pairs[i].l1index = NONE;      
      if (strcasecmp (l2word, INP_NONE))
  pairs[i].l2index = wordindex (l2word, l2words, nl2words);
      else
  pairs[i].l2index = NONE;
      if (strcasecmp (sword, INP_NONE))
	pairs[i].sindex = wordindex (sword, swords, nswords);
      else
	pairs[i].sindex = NONE;
      i++;
    }
  npairs = i;			/* number of word pairs */
}


static int
list2indices (itemarray, rest, maxitems, listname)
/* convert at most maxitems words in the rest list to indices,
   store, and add to lexicon if necessary. Return the number of items */
     int itemarray[];		/* array where the indices will be stored */
     int maxitems;		/* size of the array */
     char rest[];		/* the input string */
     char listname[];		/* name of the variable list */
{
  int j = 0;
  char wordstring[MAXSTRL + 1];	/* string for one word */

  while (TRUE)
    {
      rest = rid_sspace (rest);	/* first remove blanks */
      if (rest != NULL)		/* if there is anything left */
	{
	  if (j >= maxitems)
	    {
	      fprintf (stderr, "Number of %s exceeds array size\n",
		       listname);
	      exit (EXIT_SIZE_ERROR);
	    }
	  sscanf (rest, "%s", wordstring);	/* get the next word */
	  rest += strlen (wordstring);		/* remove from the string */
	  					/* put it in the list */
	  itemarray[j++] = wordindex (wordstring, swords, nswords);
	}
      else
	break;			/* if the string has been exhausted */
    }
  return (j);			/* number of words read */
}


static int
text2floats (itemarray, nitems, nums)
/* convert the string of numbers to floats and stores in an array, returning
   the number of floats read, or nitems + 1 if there where too many */
     double itemarray[];	/* array where the numbers will be stored */
     int nitems;		/* max number of items */
     char nums[];		/* the input string */
{
  int j = 0;
  char onenum[MAXSTRL + 1];	/* string for one float */

  while (j < nitems)
    {
      nums = rid_sspace (nums);	/* first get rid of blanks */
      if (nums != NULL)		/* if there is anything left */
	{
	  sscanf (nums, "%s", onenum);  /* get the next number */
	  nums += strlen (onenum);	/* remove from the string */
	  if (sscanf (onenum, "%lf", &itemarray[j]) != 1)
	    break;			/* if could not get a number */
	  else
	    j++;
	}
      else
	break;			/* if the string has been exhausted */
    }
  /* check if there was more stuff in the input string that was not read */
  if (j == nitems && strlen (nums))
    {
      nums = rid_sspace (nums);	/* first get rid of blanks */
      if (nums != NULL)		/* if there is anything left */
	j++;
    }
  return (j);
}


static int
text2ints (itemarray, nitems, nums)
/* convert the string of numbers to ints and stores in an array, returning
   the number of ints read, or nitems + 1 if there where too many */
     int itemarray[];		/* array where the numbers will be stored */
     int nitems;		/* max number of items */
     char nums[];		/* the input string */
{
  int j = 0;
  char onenum[MAXSTRL + 1];	/* string for one float */

  while (j < nitems)
    {
      nums = rid_sspace (nums);	/* first get rid of blanks */
      if (nums != NULL)		/* if there is anything left */
	{
	  sscanf (nums, "%s", onenum);  /* get the next number */
	  nums += strlen (onenum);	/* remove from the string */
	  if (sscanf (onenum, "%d", &itemarray[j]) != 1)
	    break;			/* if could not get a number */
	  else
	    j++;
	}
      else
	break;			/* if the string has been exhausted */
    }
  /* check if there was more stuff in the input string that was not read */
  if (j == nitems && strlen (nums))
    {
      nums = rid_sspace (nums);	/* first get rid of blanks */
      if (nums != NULL)		/* if there is anything left */
	j++;
    }
  return (j);
}


/********************* lexicon entries */

static void
reps_init (lexsem, repfile, words, nwords, nrep)
/* read the word labels and representations from a file */
/* this is called once for lexical and once for semantic words */
     char *lexsem,		/* either "lexical" or "semantic" */
      *repfile;			/* name of the representation file */
     WORDSTRUCT words[];	/* the lexicon data structure */
     int *nwords,		/* return number of words */
      *nrep;			/* return representation dimension */
{
  int i;
  char instancestring[MAXSTRL + 1],/* temporarily holds list of instances */
  wordstring[MAXSTRL + 1],	/* temporarily holds the word */
  repstring[MAXSTRL + 1];	/* temporarily holds the representations */
  FILE *fp;

  printf ("Reading %s representations from %s...", lexsem, repfile);
  fflush (stdout);
  open_file (repfile, "r", &fp, REQUIRED);

  if (!strcmp (lexsem, SEMANTIC_KEYWORD))
    /* if this is semantic file, read the list of instances and hold it */
    {
      /* find the instances */
      read_till_keyword (fp, REPS_INST, REQUIRED);
      fgetline (fp, instancestring, MAXSTRL);
    }

  /* get representation size */
  read_till_keyword (fp, REPS_REPSIZE, REQUIRED);
  fscanf (fp, "%d", nrep);
  if (*nrep > MAXREP || *nrep <= 0)
    {
      fprintf (stderr, "%s exceeds array size\n", REPS_REPSIZE);
      exit (EXIT_SIZE_ERROR);
    }
  
  /* find the representations */
  read_till_keyword (fp, REPS_REPS, REQUIRED);
  for (i = 0; i < MAXWORDS + 1; i++)
    {
      if (fscanf (fp, "%s", wordstring) == EOF)   /* read label */
	break;
      else if (i >= MAXWORDS)
	{
	  fprintf (stderr, "Number of %s words exceeds array size\n",
		   lexsem);
	  exit (EXIT_SIZE_ERROR);
	}
      else if (strlen (wordstring) > MAXWORDL)
	{
	  fprintf (stderr, "Length of word %s exceeds array size\n",
		   wordstring);
	  exit (EXIT_SIZE_ERROR);
	}
      sprintf(words[i].chars, "%s", wordstring);

      /* read the representation components */
      fgetline (fp, repstring, MAXSTRL);
      /* convert the string to numbers and load */
      if (text2floats (words[i].rep, *nrep, repstring) != *nrep)
	{
	  fprintf (stderr, "Wrong number of components for %s\n",
		   words[i].chars);
	  exit (EXIT_DATA_ERROR);
	}
    }
  *nwords = i;			/* set the number of words */
  fclose (fp);

  if (!strcmp (lexsem, SEMANTIC_KEYWORD))
    {
      /* after the lexicon has been set up, we can set up instance indices */
      /* convert the word strings into indices */
      ninstances = list2indices (instances, instancestring, *nwords,REPS_INST);
      printf ("%d reps (%d instances).\n", *nwords, ninstances);
    }
  else
    printf ("%d reps.\n", *nwords);
}


static int
wordindex (wordstring, words, nwords)
/* find the lexicon index of a word given as a string */
     char wordstring[];		/* text word */
     WORDSTRUCT words[];	/* lexicon */
     int nwords;		/* number of words in lexicon */
{
  int i;

  for (i = 0; i < nwords; i++) 	/* scan through the lexicon */
    if (!strcasecmp (wordstring, words[i].chars))
      return (i);		/* if the word found, return its index */
  fprintf (stderr, "Word %s not found in the lexicon\n", wordstring);
  exit (EXIT_DATA_ERROR);
}


int
select_lexicon (modi, words, nrep, nwords)
/* based on the module number, selects either lexical or semantic lexicon
   returns the module number used for the window */
int modi;				/* module number */
WORDSTRUCT **words;			/* lexicon pointer*/
int *nrep,				/* representation size */
  *nwords;				/* number of words */
{
  if (modi == SINPMOD && modi == SOUTMOD)
    /* it is semantic */
    {
      *words = swords;
      *nrep = nsrep;
      *nwords = nswords;
      return (SEMWINMOD);
    }
  else if (modi == L1INPMOD && modi == L1OUTMOD)
    /* it is L1 */
    {
      *words = l1words;
      *nrep = nl1rep;
      *nwords = nl1words;
      return (L1WINMOD);
    }
  else
    /* it is L2 */
    {
      *words = l2words;
      *nrep = nl2rep;
      *nwords = nl2words;
      return (L2WINMOD);
    }
}


/*********************  I/O and low-level routines  *********************/

/***************** inputfile */

static int
read_till_keyword (fp, keyword, required)
/* read text in the file until you find the given keyword
   in the beginning of the line; if the keyword is required but
   not found, terminate program; otherwise return if it was found or not */
     FILE *fp;			/* pointer to the file */
     char keyword[];		/* the keyword string */
     int required;		/* whether to abort if keyword not found */
{
  char s[MAXSTRL + 1];

  while (TRUE)
    if (fscanf (fp, "%s", s) == EOF)
      /* file ran out, keyword was not found */
      if (required)
	{
	  fprintf (stderr, "Could not find keyword: %s\n", keyword);
	  exit (EXIT_DATA_ERROR);
	}
      else
	/* that's ok; return the signal indicating that it was not found */
	return (FALSE);
    else
      /* something was read from the file */
      {
	/* if it was the keyword, return success signal */
	if (!strcasecmp (s, keyword))
	  return (TRUE);
	/* otherwise get rid of the rest of the line */
	fgl (fp);
      }
}


static char *
rid_sspace (rest)
/* read the blanks off the string */
     char rest[];		/* string to be cleaned up */
{
  int i = 0;
  while (i < strlen (rest) && (rest[i] == ' ' || rest[i] == '\t'))
    i++;
  if (i < strlen (rest))	/* if there is anything left */
    return (rest + i);
  else
    return (NULL);
}


static void
rid_fspace (fp)
/* read the blanks off the file */
FILE *fp;
{
  int c;
  
  for (c = getc (fp);
       c != EOF && (c == ' ' || c == '\t' || c == '\n');
       c = getc (fp))
    ;
  ungetc (c, fp);
}


static void
fgetline (fp, s, lim)
/* read a line of at most lim characters from the file fp into string s */
     FILE *fp;
     char *s;
     int lim;
{
  int c = 0;
  int i = 0;

  while (--lim > 0 && (c = getc (fp)) != EOF && c != '\n')
    s[i++] = (char) c;
  s[i] = '\0';
  if (lim == 0 && c != EOF && c != '\n')
    {
      fprintf (stderr, "Line character limit exceeded\n");
      exit (EXIT_SIZE_ERROR);
    }
}


static void
fgl (fp)
/* get rid of the rest of the line and the newline in the end */
     FILE *fp;
{
  int c;

  while ((c = getc (fp)) != EOF && c != '\n')
    ;
}

static int
open_file (filename, mode, fpp, required)
/* open a file, exit or return FALSE if failure */
     char *filename;		/* file name */
     FILE **fpp;		/* return the file pointer */
     char *mode;		/* open mode: "r", "a", "w" */
     int required;		/* whether to exit if cannot open */
{
  if ((*fpp = fopen (filename, mode)) == NULL)
    {
      fprintf (stderr, "Cannot open %s\n", filename);
      if (required)
	exit (EXIT_FILE_ERROR);
      else
	return (FALSE);
    }
  return (TRUE);
}


/***************** weights */

static void
readfun (fp, place, par1, par2)
/* function for reading a float from a file */
/* given as a parameter for reading in weights */
     FILE *fp;			/* weight file */
     double *place,		/* return the float read */
       par1, par2;		/* unused */
{
  if (fscanf (fp, "%lf", place) > 1)
    {
      printf ("fscanf (fp, \"lf\", place)  = %d\n", fscanf (fp, "%lf", place));
      printf ("place  = %lf\n", place);
      fprintf (stderr, "Error reading weights\n");
      exit (EXIT_DATA_ERROR);
    }
}


static void
writefun (fp, place, par1, par2)
/* function for writing a float to a file */
/* given as a parameter for writing weights */
     FILE *fp;			/* weight file */
     double *place,		/* the float to be written */
       par1, par2;		/* unused */
{
  fprintf (fp, "%f\n", *place);
}


static void
randfun (fp, place, low, span)
/* function for assigning a random float value */
/* given as a parameter for initializing weights */
     FILE *fp;			/* not used */
     double *place,		/* the float to be written */
       low, span;		/* bias and scale for the range */
{
  *place = low + span * f01rnd ();
}


/***************** lexicon comparison */

int
find_nearest (rep, words, nrep, nwords)
/* find the index of the nearest representation in the lexicon */
     double rep[];		/* word representation */
     WORDSTRUCT words[];	/* lexicon (lexical or semantic) */
     int nrep,			/* rep dimension */
       nwords;			/* number of words in lexicon */
{
  int i,
    bestindex = 0;		/* index of the closest rep so far */
  double lbest,			/* closest distance so far */
    dist;			/* last distance */

  lbest = LARGEFLOAT;
  /* linearly search through the lexicon */
  /* starting from the blank */
  for (i = 0; i < nwords; i++)
    {
      dist = distance (NULL, rep, words[i].rep, nrep);
      if (dist < lbest)		/* keep track of the best one so far */
	{
	  bestindex = i;
	  lbest = dist;
	}
    }
  return (bestindex);
}


double
distance (foo, v1, v2, nrep)
/* compute the Euclidean distance of two nrep dimensional vectors */
     double v1[], v2[];		/* the vectors */
     int nrep;			/* their dimensionality */
     int *foo;			/* not used */
{
  double sum = 0.0;
  int i;

  for (i = 0; i < nrep; i++)
    sum += (v1[i] - v2[i]) * (v1[i] - v2[i]);
  return (sqrt (sum));
}


/*********************  simulation management  *************************/

static void
get_current_params (epoch)
/* determine the current learning rates and which modules to run
   depending on epoch and phase definitions */
     int epoch;			/* current epoch */
{
  int phase;

  /* first figure out what phase we are in */
  for (phase = nphase - 1; phase > 0 && phaseends[phase - 1] >= epoch; phase--)
    ;

  /* which networks are currently running */
  l1_running = l1_runnings[phase];
  l2_running = l2_runnings[phase];
  sem_running = sem_runnings[phase];
  l1l2_assoc_running = l1l2_assoc_runnings[phase];
  sl1_assoc_running = sl1_assoc_runnings[phase];
  sl2_assoc_running = sl2_assoc_runnings[phase];

  /* get the current learning rates */
  l1_alpha = current_alpha (epoch, phase, phaseends, l1_alphas);
  l2_alpha = current_alpha (epoch, phase, phaseends, l2_alphas);
  sem_alpha = current_alpha (epoch, phase, phaseends, sem_alphas);
  l1l2_assoc_alpha = current_alpha (epoch, phase, phaseends, l1l2_assoc_alphas);
  sl1_assoc_alpha = current_alpha (epoch, phase, phaseends, sl1_assoc_alphas);
  sl2_assoc_alpha = current_alpha (epoch, phase, phaseends, sl2_assoc_alphas);

  /* if number of propagation units is specified,
     calculate the smallest nc that contains them */
  if (npropunits_given)
    l1_nc = l2_nc = sem_nc = ((int) ceil (sqrt (npropunits))) / 2;
  else
    /* use the nc given in the schedule */
    {
      l1_nc = current_nc (epoch, phase, phaseends, l1_ncs);
      l2_nc = current_nc (epoch, phase, phaseends, l2_ncs);
      sem_nc = current_nc (epoch, phase, phaseends, sem_ncs);
    }
}


static double
current_alpha (epoch, phase, phaseends, alphas)
  /* change the gain linearly during the phase */
   int epoch, phase, phaseends[];
double alphas[];
{
  /* use epoch - 1 so that a new value is established during the
     first epoch of each phase */
  return (alphas[phase-1] +
	  (alphas[phase] - alphas[phase-1]) * (epoch - 1 - phaseends[phase-1])
	  / (phaseends[phase] - phaseends[phase-1]));
}


static int
current_nc (epoch, phase, phaseends, ncs)
  /* change the neighborhood size linearly during the phase */
int epoch, phase, phaseends[], ncs[];
{
  /* use epoch - 1 so that a new value is established during the
     first epoch of each phase */
  return (floor (ncs[phase - 1] + 0.999999 +
       ((double) ncs[phase] - ncs[phase-1]) * (epoch - 1 - phaseends[phase-1])
		 / (phaseends[phase] - phaseends[phase-1])));
}

 
static void
save_current (epoch)
/* save the current weights */
     int epoch;			/* current epoch */
{
  FILE *fp;			/* simulation file pointer */

  open_file (simufile, "a", &fp, REQUIRED);
  /* first write the current epoch number and network errors */
  fprintf (fp, "%s %d\n", SIMU_EPOCH, epoch);
  fprintf (fp, "%s", SIMU_NETWORK_ERRORS);
  write_error (fp);
  fprintf (fp, "%s\n", SIMU_NETWORK_WEIGHTS);
  iterate_weights (writefun, fp, NOT_USED, NOT_USED);
  fclose (fp);
  /* update the next snapshot epoch */
  update_nextsnapshot (epoch);
}


static void
update_nextsnapshot (epoch)
/* find out when the next snapshot should be saved */
     int epoch;			/* current epoch */
{
  while (nextsnapshot < nsnaps)
    {
      if (snapshots[nextsnapshot] <= epoch)
	nextsnapshot++;
      else
	break;
    }
}


static void
shuffle ()
/* change the order of training sentences */
{
  int i;
  int dest, destvalue;
  /* sequentially find a new random place for each sentence */
  for (i = 0; i < npairs; i++)
    {
      dest = lrand48 () % npairs;
      destvalue = shuffletable[dest];
      shuffletable[dest] = shuffletable[i];
      shuffletable[i] = destvalue;
    }
}


static double
f01rnd ()
{
  /* random double between 0.0 and 1.0 */
  return (drand48 ());
}


int
fsmaller (x, y)
/* used for comparing unit activities in search for the
   maximally and minimally responding units */
     double x, y;
{
  return (x < y);
}


int
fgreater (x, y)
/* used for comparing unit activities in search for the
   maximally and minimally responding units */
     double x, y;
{
  return (x > y);
}
