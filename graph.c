/* File: graph.c
 * 
 * X interface for DISLEX
 * 
 * Copyright (C) 1994 Risto Miikkulainen
 *
 *  This software can be copied, modified and distributed freely for
 *  educational and research purposes, provided that this notice is included
 *  in the code, and the author is acknowledged in any materials and reports
 *  that result from its use. It may not be used for commercial purposes
 *  without expressed permission from the author.
 *
 * $Id: graph.c,v 1.15 1997/01/09 03:28:48 risto Exp risto $
 */

#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xaw/AsciiText.h>

#include "defs.h"
#include "Gwin.h"
#include "globals.c"


/************ graphics constants *************/

/* color parameters */
#define UNITCOLORS 0		/* chooses bryw colorscale */
#define WEIGHTCOLORS 1		/* chooses gbryw colorscale (not used) */
#define MAXCOLORS 256		/* max # of colors */
#define MINCOLORS 10		/* warning if fewer colors obtained */
#define MAXCOLGRAN 65535	/* granularity of color values */

/* colormap entry */
typedef struct RGB
  {
    short red, green, blue;	/* hold possible rgb values in XColor struct */
  }
RGB;

/* graphics separator constants */
#define VERSP 3			/* generic vertical separation */
#define HORSP 3			/* generic horizontal separation */
#define BOXSP 1			/* vertical space around boxed text */

/* displaying labels */
#define MORELABEL "more.."	/* if # of labels > maxlabels, this is shown */

#define NWINS NMODULES		/* number of windows */


/******************** Function prototypes ************************/

/* global functions */
#include "prototypes.h"
extern unsigned sleep __P ((unsigned seconds));

/* functions local to this file */
static void handle_events __P((void));
static void runstop_callback __P((Widget w, XtPointer client_data,
				  XtPointer call_data));
static void start_running __P((void));
static void toggle_callback __P((Widget w, XtPointer client_data,
				 XtPointer call_data));
static void clear_callback __P((Widget w, XtPointer client_data,
				XtPointer call_data));
static void clear_networks_display __P((void));
static void quit_callback __P((Widget w, XtPointer client_data,
			       XtPointer call_data));
static void close_display __P((void));
static void init_rgbtab_noweights __P((void));
static void init_rgbtab_bw __P((void));
static void alloc_col __P((void));
static void clean_color_map __P((int k));
static void create_colormap __P((void));
static int createGC __P((Window New_win, GC *New_GC, Font fid, Pixel FGpix,
			 Pixel theBGpix));
static XFontStruct *loadFont __P((char fontName[]));
static void common_resize __P((int modi, Widget w));
static void expose_lex __P((Widget w, XtPointer units, XtPointer call_data));
static void resize_lex __P((Widget w, XtPointer client_data,
			    XtPointer call_data));
static void lexsemmouse_handler __P ((Widget w, XtPointer client_data,
				      XEvent *p_event));
static void display_assocweights __P ((int srcmodi,
				       FMUNIT srcunits[MAXLSNET][MAXLSNET],
				       int nsrcnet, WORDSTRUCT *srcwords,
				       int nsrcrep, int nsrcwords, int tgtmodi,
				       FMUNIT tgtunits[MAXLSNET][MAXLSNET],
				       int ntgtnet, WORDSTRUCT *tgtwords,
				       int ntgtrep, int ntgtwords,
				       int x, int y,
				       double assoc[MAXLSNET][MAXLSNET]
				                   [MAXLSNET][MAXLSNET]));
static void display_title __P((int modi, char name[]));
static void display_log __P((int modi));
static void frameRectangle __P((int modi, int x, int y, int width, int height,
				int colorindex));
static void labelbox __P((int modi, int x, int y, int width, int height,
			  double value, char labels[][MAXWORDL + 1], 
			  int labelcount, XFontStruct *fontstruct,
			  GC fGC, GC bGC, GC nGC));
static void collect_labels __P((int modi, char labels[][MAXWORDL + 1],
				int *count, char label[]));
static int trans_to_color __P((double value, int map));
static void clearRectangle __P((int modi, int x, int y, int width,
				int height));
static void fillRectangle __P((int modi, int x, int y, int width, int height,
			       int colorindex));
static void drawRectangle __P((int modi, int x, int y, int width, int height));
static void drawText __P((int modi, int x, int y, char text[], GC currGC));
static void drawoverText __P((int modi, int x, int y, char text[], GC currGC));


/******************** static variables *********************/

static XColor colors[MAXCOLORS];
static int actual_color_range;	/* number of colors in use */

/* button, command, and graphics widgets */
static Widget
  runstop, step, clear, quit,	/* buttons */
  command,			/* command input widget */
  l1, l2, sem;			/* network graphics widgets */

/* useful pointers to windows */
static Window
  theMain, runstopWin, commandWin, Win[NWINS];	/* network graphics windows */

/* text constants to be displayed */
static char
 *titles[] =
{"L1 MAP", "L2 MAP",	/* input and output L1 lexical maps are combined*/
 "SEMANTIC MAP", "unused",  /* input and output L2 lexical maps are combined*/
 "unused", "unused"};	/* into one window; their entries are unused */

/* graphics contexts */
static GC
  titleGC,			/* window name */
  logGC,			/* log line: item and error */
  l1fGC,
  l2fGC,
  semfGC,			/* foreground labels on maps */
  l1bGC,
  l2bGC,
  sembGC,			/* background (reverse) labels */
  l1nGC,
  l2nGC,
  semnGC,			/* nearestlabels */
  clearGC,			/* for clearing areas */
  boxGC,			/* for drawing network boxes */
  activityGC;			/* for displaying activations */

/* corresponding fonts */
static XFontStruct
 *titlefontStruct, *logfontStruct, *l1fontStruct, *l2fontStruct, *semfontStruct;

static int titleboxhght;	/* size of text boxes */

/* Array of rgb values that represents a linear color spectrum */
static RGB rgbtab[MAXCOLORS];
/* the unit value colormap */
static short cmap[MAXCOLORS];


/********************* general initialization ***********************/

void
display_init ()
/* initialize the X display */
{
  Arg args[3];
  char s[MAXSTRL + 1];
  Pixel theBGpix;		/* background color */
  Dimension borderwidth,
    height, width,		/* various widget heights and widths given */
    tot_width;			/* computed total width of display */

  printf ("Initializing graphics...\n");

  XtSetArg (args[0], XtNborderWidth, &borderwidth);
  XtGetValues (main_widget, args, 1);	/* get the current border width  */
  /* create a form with no space between widgets */
  XtSetArg (args[0], XtNdefaultDistance, 0);
  form = XtCreateManagedWidget ("form", formWidgetClass, main_widget, args, 1);

  /* the command button for running and stopping the simulation */
  runstop = XtCreateManagedWidget ("runstop", commandWidgetClass,
				   form, args, 1);
  /* toggle switch: if on, the simulation will stop after each propagation */
  step = XtCreateManagedWidget ("step", toggleWidgetClass, form, args, 1);
  /* the command button for clearing the networks */
  clear = XtCreateManagedWidget ("clear", commandWidgetClass, form, args, 1);
  /* the command button for exiting the program */
  quit = XtCreateManagedWidget ("quit", commandWidgetClass, form, args, 1);

  /* create command line widget: */
  /* first get the height from runstop */
  XtSetArg (args[0], XtNheight, &height);
  /* then figure out the total width of the display by */
  /* getting the widths of each widget, adding them up, and subtracting
     from the total width of the display */
  XtSetArg (args[1], XtNwidth, &width);
  XtGetValues (runstop, args, 2);
  tot_width = width + 2 * borderwidth;
  XtSetArg (args[0], XtNwidth, &width);
  XtGetValues (step, args, 1);
  tot_width += width + 2 * borderwidth;
  XtSetArg (args[0], XtNwidth, &width);
  XtGetValues (clear, args, 1);
  tot_width += width + 2 * borderwidth;
  XtSetArg (args[0], XtNwidth, &width);
  XtGetValues (quit, args, 1);
  tot_width += width + 2 * borderwidth;
  XtSetArg (args[0], XtNheight, height);
  XtSetArg (args[1], XtNwidth, data.netwidth - tot_width);
  /* display the name of the simufile in the command line widget */
  sprintf (s, "simulation file: %s", simufile);
  XtSetArg (args[2], XtNstring, s);
  command = XtCreateManagedWidget ("command", asciiTextWidgetClass,
				   form, args, 3);

  /* create the lex network displays */
  /* get the size from resourses */
  XtSetArg (args[0], XtNwidth, data.netwidth);
  XtSetArg (args[1], XtNheight, data.l1netheight);
  l1 = XtCreateManagedWidget ("l1", gwinWidgetClass, form, args, 2);
  XtSetArg (args[1], XtNheight, data.l2netheight);
  l2 = XtCreateManagedWidget ("l2", gwinWidgetClass, form, args, 2);
  XtSetArg (args[1], XtNheight, data.semnetheight);
  sem = XtCreateManagedWidget ("sem", gwinWidgetClass, form, args, 2);

  /* callbacks: what to do when a button is pressed */
  XtAddCallback (runstop, XtNcallback, runstop_callback, NULL);
  XtAddCallback (step, XtNcallback, toggle_callback, NULL);
  XtAddCallback (clear, XtNcallback, clear_callback, NULL);
  XtAddCallback (quit, XtNcallback, quit_callback, NULL);

  /* network callbacks: redrawing the state */
  XtAddCallback (l1, XtNexposeCallback, expose_lex, l1units);
  XtAddCallback (l2, XtNexposeCallback, expose_lex, l2units);
  XtAddCallback (sem, XtNexposeCallback, expose_lex, sunits);

  /* network callbacks for resizing */
  XtAddCallback (l1, XtNresizeCallback, resize_lex, NULL);
  XtAddCallback (l2, XtNresizeCallback, resize_lex, NULL);
  XtAddCallback (sem, XtNresizeCallback, resize_lex, NULL);

  /* network event handlers for mouse clicks */
  XtAddEventHandler (l1, ButtonPressMask, FALSE,
		     (XtEventHandler) lexsemmouse_handler, NULL);
  XtAddEventHandler (l2, ButtonPressMask, FALSE,
         (XtEventHandler) lexsemmouse_handler, NULL);
  XtAddEventHandler (sem, ButtonPressMask, FALSE,
		     (XtEventHandler) lexsemmouse_handler, NULL);

  /* figure out the display type and allocate colors */
  create_colormap ();
  /* put the display on screen */
  XtRealizeWidget (main_widget);
  theMain = XtWindow (main_widget);
  runstopWin = XtWindow (runstop);
  commandWin = XtWindow (command);

  /* get the lexicon windows */
  Win[L1WINMOD] = XtWindow (l1);	/* get a pointer to the window */
  Win[L2WINMOD] = XtWindow (l2);  /* get a pointer to the window */
  Win[SEMWINMOD] = XtWindow (sem);	/* get a pointer to the window */

  /* set a common font for all buttons and command line */
  XtSetArg (args[0], XtNfont, loadFont (data.commandfont));
  XtSetValues (runstop, args, 1);
  XtSetValues (step, args, 1);
  XtSetValues (clear, args, 1);
  XtSetValues (quit, args, 1);
  XtSetValues (command, args, 1);

  /* load the other fonts */
  titlefontStruct = loadFont (data.titlefont);
  logfontStruct = loadFont (data.logfont);
  l1fontStruct = loadFont (data.l1font);
  l2fontStruct = loadFont (data.l2font);
  semfontStruct = loadFont (data.semfont);

  /* figure out space needed for the title and labels */
  titleboxhght = titlefontStruct->ascent + titlefontStruct->descent
    + 2 * BOXSP;

  /* get the background color */
  XtSetArg (args[0], XtNbackground, &theBGpix);
  XtGetValues (main_widget, args, 1);

  /* create graphics context for all fonts */
  createGC (theMain, &titleGC, titlefontStruct->fid, data.textColor, theBGpix);
  createGC (theMain, &logGC, logfontStruct->fid, data.textColor, theBGpix);

  /* these are foreground colors */
  createGC (theMain, &l1fGC, l1fontStruct->fid, data.textColor, theBGpix);
  createGC (theMain, &l2fGC, l2fontStruct->fid, data.textColor, theBGpix);
  createGC (theMain, &semfGC, semfontStruct->fid, data.textColor, theBGpix);

  /* and these are background (when the label color needs to be reversed) */
  createGC (theMain, &l1bGC, l1fontStruct->fid, theBGpix, theBGpix);
  createGC (theMain, &l2bGC, l2fontStruct->fid, theBGpix, theBGpix);
  createGC (theMain, &sembGC, semfontStruct->fid, theBGpix, theBGpix);

  /* and these are the nearestlabel colors (same as net) */
  createGC (theMain, &l1nGC, l1fontStruct->fid, data.netColor, theBGpix);
  createGC (theMain, &l2nGC, l2fontStruct->fid, data.netColor, theBGpix);
  createGC (theMain, &semnGC, semfontStruct->fid, data.netColor, theBGpix);

  /* clearing areas */
  createGC (theMain, &clearGC, logfontStruct->fid, theBGpix, theBGpix);
  /* network boxes */
  createGC (theMain, &boxGC, logfontStruct->fid, data.netColor, theBGpix);
  /* generic context for displaying unit activity */
  createGC (theMain, &activityGC, logfontStruct->fid, theBGpix, theBGpix);

  /* calculate all network geometries and put them on screen */
  resize_lex (l1, NULL, NULL);
  resize_lex (l2, NULL, NULL);
  resize_lex (sem, NULL, NULL);

  printf ("Graphics initialization complete.\n");
}


/*********************  event handler  ***********************/

static void
handle_events ()
  /* event handling loop */
  /* we need this instead of XtMainLoop because the main task is to run
     the simulation, and only occasionally check for events */
{
  XEvent theEvent;

  while (XtAppPending (app_con))
  /* as long as there are unprocessed events */
    {
      XtAppNextEvent (app_con, &theEvent);
      if (!(theEvent.type == Expose && theEvent.xexpose.count > 0))
	/* only process the last expose event */
	{
	  XtDispatchEvent (&theEvent);
	  XFlush (theDisplay);
	}
    }
}


void
wait_and_handle_events ()
/* this is called after each main step of the simulation
   to stop and wait for "run" if stepping, and to deal with
   any other possible events before displaying simulation results */
{
  if (stepping)
    wait_for_run ();		/* wait until user is ready */
  else
    /* slow down the display this many seconds */
    sleep ((unsigned) abs (data.delay));
  handle_events ();		/* process all pending events before proceeding
				   to display the simulation results */
}


/*********************  button callback routines  ***********************/

static void
runstop_callback (w, client_data, call_data)
  /* "Run/Stop" button press handler:
     if simulation is running, stop it; otherwise, continue simulation.
     from wherever handle_events was called */
  /* standard callback parameters, not used here */
     Widget w; XtPointer client_data, call_data;
{
  if (simulator_running)
    wait_for_run ();
  else
    start_running ();
}


void
wait_for_run ()
  /* stop the simulation and start processing events */
{
  Arg args[1];

  simulator_running = FALSE;        	/* process events */
  XtSetArg (args[0], XtNlabel, "Run");	/* change label on "Run/Stop" button */
  XtSetValues (runstop, args, 1);
  XFlush (theDisplay);
  while (!simulator_running)		/* process events until Run pressed */
    {
      /* Process one event if any or block */
      XtAppProcessEvent (app_con, XtIMAll);
      /* Process more events, but don't block */
      handle_events ();
    }
}


static void
start_running ()
  /* set up to continue or start simulation */
{
  Arg args[1];

  simulator_running = TRUE;		/* allow exit from wait_for_run */
  XtSetArg (args[0], XtNlabel, "Stop");	/* change label on "Run/Stop" button */
  XtSetValues (runstop, args, 1);
}


static void
toggle_callback (w, client_data, call_data)
  /* "Step" button press handler: turn stepping on or off */
  /* standard callback parameters, not used here */
     Widget w; XtPointer client_data, call_data;
{
  stepping = !stepping;
}


static void
clear_callback (w, client_data, call_data)
  /* "Clear" button press handler: clear the display and network activations,
     start a fresh command processing loop */
  /* standard callback parameters, not used here */
     Widget w; XtPointer client_data, call_data;
{
  extern jmp_buf loop_map_env;

  clear_networks_display ();
  longjmp (loop_map_env, 0);
}


static void
clear_networks_display ()
  /* clear the network activations, labels, logs, and the display */
{
  /* clear the L1 map and its display */
  clear_values (l1units, nl1net);
  clear_prevvalues (l1units, nl1net);
  clear_labels (l1units, nl1net);
  sprintf (net[L1WINMOD].log, "%s", "");
  XClearArea (theDisplay, Win[L1WINMOD], 0, 0, 0, 0, True);

  /* clear the L2 map and its display */
  clear_values (l2units, nl2net);
  clear_prevvalues (l2units, nl2net);
  clear_labels (l2units, nl2net);
  sprintf (net[L2WINMOD].log, "%s", "");
  XClearArea (theDisplay, Win[L2WINMOD], 0, 0, 0, 0, True);

  /* clear the semantic map and its display */
  clear_values (sunits, nsnet);
  clear_prevvalues (sunits, nsnet);
  clear_labels (sunits, nsnet);
  sprintf (net[SEMWINMOD].log, "%s", "");
  XClearArea (theDisplay, Win[SEMWINMOD], 0, 0, 0, 0, True);

  /* update the output immediately before data changes */
  XFlush (theDisplay);
  handle_events ();
}


static void
quit_callback (w, client_data, call_data)
  /* "Quit" button press handler: exit the program */
  /* standard callback parameters, not used here */
     Widget w; XtPointer client_data, call_data;
{
  close_display ();
  exit (EXIT_NORMAL);
}


static void
close_display ()
  /* free fonts and close the display */
{
  XFreeFont (theDisplay, titlefontStruct);
  XFreeFont (theDisplay, logfontStruct);
  XFreeFont (theDisplay, l1fontStruct);
  XFreeFont (theDisplay, l2fontStruct);
  XFreeFont (theDisplay, semfontStruct);
  XCloseDisplay (theDisplay);
}


/*********************  colormap allocation  ***********************/

static void
create_colormap ()
  /* allocate colors for activation values: depending on the display type
     and available colors, allocate a continuous spectrum the best you can */
{
  if (!(visual->class == GrayScale || visual->class == StaticGray))
    /* we have color display */
    init_rgbtab_noweights ();
  else
    /* black and white screen */
    init_rgbtab_bw ();
  alloc_col ();
}


static void
init_rgbtab_noweights ()
/* calculates values for the linear color spectrum black-red-yellow-white
   and stores them in rbgtab */
{
  double multiplier;
  int rangeby3, i;

  /* divide the range into three sections:
     black-red, red-yellow, yellow-white */
  rangeby3 = MAXCOLORS / 3;
  multiplier = ((double) MAXCOLGRAN) / rangeby3;
  for (i = 0; i < MAXCOLORS; i++)
    {
      rgbtab[i].green = 0;
      rgbtab[i].blue = 0;
      if (i < rangeby3)
	rgbtab[i].red = (int) (i * multiplier);
      else
	/* second section: red to yellow */
	{
	  rgbtab[i].red = MAXCOLGRAN;
	  if (i < 2 * rangeby3)
	    rgbtab[i].green = (int) ((i - rangeby3) * multiplier);
	  else
	    /* third section: yellow to white */
	    {
	      rgbtab[i].green = MAXCOLGRAN;
	      rgbtab[i].blue = (int) ((i - 2 * rangeby3) * multiplier);
	    }
	}
    }
}


static void
init_rgbtab_bw ()
/* calculates values for the linear gray scale and stores them in rbgtab */
{
  double multiplier;
  int i;

  /* straight scale from black to white */
  multiplier = ((double) MAXCOLGRAN) / MAXCOLORS;
  for (i = 0; i < MAXCOLORS; i++)
    rgbtab[i].green =  rgbtab[i].blue = rgbtab[i].red
      = (int) (i * multiplier);
}


static void
alloc_col ()
/* allocate colors from rgbtab until no more free cells,
   using existing colors if they match what we want */
{
  int start = MAXCOLORS / 2;		/* starting offset in an alloc sweep */
  int d = MAXCOLORS;			/* increment in an alloc sweep */
  int j,				/* index to the linear spectrum */
    k;					/* number of colors allocated */

  for (j = 0; j < MAXCOLORS; j++)
    cmap[j] = NONE;
  k = 0;

  /* add colors to cmap, keep them uniformly distributed in the range,
     and gradually make the spectrum more refined */
  while (d > 1)
    {
      /* add colors to cmap with d as the current distance
	 between new colors in the spectrum and start as the first location */
      j = start;
      while (j < MAXCOLORS)		/* completed a sweep of new colors */
	{
	  colors[k].flags = DoRed | DoGreen | DoBlue;	/* use all planes */
	  colors[k].red = rgbtab[j].red;
	  colors[k].green = rgbtab[j].green;
	  colors[k].blue = rgbtab[j].blue;
	  
	  cmap[j] = k;
	  if (XAllocColor (theDisplay, colormap, &(colors[k])) == 0)
	    {
	      cmap[j] = NONE;
	      clean_color_map(k);
	      return;
	    }
	  k++;				/* allocated one new color */
	  j += d;			/* next location in the spectrum */
	}
      d /= 2;				/* set up a tighter distance */
      start /= 2;			/* start lower in the spectrum */
    }
  clean_color_map(k);	                /* set # of colors, clean up */
}

static void
clean_color_map(k)
/* set the number of colors, print message, and clean up the map */
     int k;				/* number of colors allocated */
{
  int i, m;			/* counters for cleaning up cmap */

  /* colors[k-1] is the last valid colorcell */
  actual_color_range = k;

  if (actual_color_range < MINCOLORS)
    {
      fprintf (stderr, "Warning: obtained only %d colors\n", k);
      fprintf (stderr, "(consider using a private colormap)\n");
    }
  else
    printf ("Obtained %d colors.\n", k);

  /* clean up cmap; move all entries to the beginning */
  m = 0;
  while (m < MAXCOLORS && cmap[m] != NONE)
    ++m;
  for (i = m + 1; i < MAXCOLORS; i++) {
    if (cmap[i] == NONE)	/* no colorcell */
      continue;
    else
      cmap[m] = cmap[i], ++m; }
}


/*********************  GCs, fonts, resizing  ***********************/

static int
createGC (New_win, New_GC, fid, theFGpix, theBGpix)
  /* create a new graphics context for the given window with given
     font and foreground and background colors */
     Window New_win;		/* window for the GC */
     GC *New_GC;		/* return pointer to the created GC here */
     Font fid;			/* font id  */
     Pixel theFGpix, theBGpix;	/* foreground and background colors */
{
  XGCValues GCValues;		/* graphics context parameters; not used */

  *New_GC = XCreateGC (theDisplay, New_win, (unsigned long) 0, &GCValues);

  if (*New_GC == 0)		/* could not create */
    return (FALSE);
  else
    {
      /* set font, foreground and background */
      XSetFont (theDisplay, *New_GC, fid);
      XSetForeground (theDisplay, *New_GC, theFGpix);
      XSetBackground (theDisplay, *New_GC, theBGpix);
      return (TRUE);
    }
}


static XFontStruct *
loadFont (fontName)
  /* load a given font */
     char fontName[];		/* name of the font */
{
  XFontStruct *fontStruct;	/* return font here */

  if ((fontStruct = XLoadQueryFont (theDisplay, fontName)) == NULL)
    {
      fprintf (stderr, "Cannot load font: %s, using fixed\n", fontName);
      if ((fontStruct = XLoadQueryFont (theDisplay, "fixed")) == NULL)
	{
	  fprintf (stderr, "Cannot load fixed font\n");
	  exit (EXIT_X_ERROR);
	}
    }
  return (fontStruct);
}


static void
common_resize (modi, w)
  /* when resizing any net, first get the new window width and height */
     int modi;			/* module number */
     Widget w;			/* and its widget */
{
  Arg args[2];
  Dimension width, height;

  /* get the current width and height from the server */
  XtSetArg (args[0], XtNwidth, &width);
  XtSetArg (args[1], XtNheight, &height);

  XtGetValues (w, args, 2);
  /* and store them for further calculations */
  net[modi].width = width;
  net[modi].height = height;

}


/********************* lexicon operations ***********************/

void
init_lex_display (modi, nnet, nwords, words, nrep, units)
/* assign labels for the image units of each word, clear the values */
     int modi;				/* module number */
     int nnet,				/* map size */
     nwords;				/* number of words */
     WORDSTRUCT words[];		/* lexicon (lexical or semantic) */
     int nrep;				/* dimension of the word rep */
     FMUNIT units[MAXLSNET][MAXLSNET];	/* map */
{
  int i, j, k;				/* map and word indices */
  int besti, bestj;			/* indices of the image unit */
  double best, foo;			/* best and worst response found */

  /* clean up the old labels */
  clear_labels (units, nnet);

  /* clean up the value for finding nearest inputs */
  for (i = 0; i < nnet; i++)
    for (j = 0; j < nnet; j++)
      units[i][j].bestvalue = LARGEFLOAT;
  
  /* assign labels to maximally responding units for each word,
     updating nearestlabels when appropriate */
  for (k = 0; k < nwords; k++)
    {
      /* find the max responding unit */
      best = foo = LARGEFLOAT;
      for (i = 0; i < nnet; i++)
	for (j = 0; j < nnet; j++)
	  {
	    /* response proportional to distance of weight and input vectors */
	    distanceresponse (&units[i][j], words[k].rep, nrep, distance,NULL);
	    /* find best and worst and best indices */
	    updatebestworst (&best, &foo, &besti, &bestj, &units[i][j], i, j,
			     fsmaller, fgreater);
	    if (units[i][j].value < units[i][j].bestvalue)
	      {
		units[i][j].bestvalue = units[i][j].value;
		sprintf (units[i][j].labels[0], "%s", words[k].chars);
	      }
	  }
      /* update the label list of that unit */
      collect_labels (modi, units[besti][bestj].labels,
		      &units[besti][bestj].labelcount, words[k].chars);
    }
    
  /* clear the activations */
  clear_values (units, nnet);
  clear_prevvalues (units, nnet);

  /* display map activity */
  display_lex (modi, units, nnet);
  /* display the log line */
  sprintf(net[modi].log, "%s", "");
  display_log (modi);
}


/********************* expose event handler */
static void
expose_lex (w, units, call_data)
  /* expose event handler for a lexicon map: redraw the window */
  /* standard callback parameters; widget is actually used here */
     Widget w; XtPointer units, call_data;
{
  /* figure out the module number and size from the widget given */
  int
    modi = ((w == sem) ? SEMWINMOD : ((w == l1) ? L1WINMOD : L2WINMOD)),
    nnet = ((w == sem) ? nsnet : ((w == l1) ? nl1net : nl2net));
  
  XClearWindow (theDisplay, Win[modi]);
  display_title (modi, titles[modi]);
  clear_prevvalues (units, nnet);
  display_lex (modi, units, nnet);
  display_log (modi);
}


/********************* resizing */
static void
resize_lex (w, client_data, call_data)
  /* resize event handler for a lexicon map: recalculate geometry */
  /* standard callback parameters; widget is actually used here */
     Widget w; XtPointer client_data, call_data;
{

  /* figure out the module number, size and font from the widget given */
  int
    modi = ((w == sem) ? SEMWINMOD : ((w == l1) ? L1WINMOD : L2WINMOD)),
    nnet = ((w == sem) ? nsnet : ((w == l1) ? nl1net : nl2net));  

  XFontStruct *fontstruct = ((w == sem) ? semfontStruct : ((w == l1) ? l1fontStruct : l2fontStruct));

  common_resize (modi, w);	/* get new window size */

  /* height of the boxes representing unit activities */
  net[modi].uhght = (net[modi].height - titleboxhght - VERSP) / nnet;
  net[modi].uwidth = (net[modi].width - 2 * HORSP) / nnet;

  /* just have the max number of labels possible instead of limiting by display size */
  /* net[modi].maxlabels = net[modi].uhght / fontstruct->ascent; */
  net[modi].maxlabels = MAXFMLABELS;

  /* horizontal margin from the edge of the window */
  net[modi].marg = (net[modi].width - nnet * net[modi].uwidth) / 2;
}


/*********************  mouse button press handler  */
static void
lexsemmouse_handler (w, client_data, p_event)
/* handles the button press events from the lexical and semantic map */
     Widget w;
     XtPointer client_data;
     XEvent *p_event;
{
  int x = p_event->xbutton.x,		/* mouse coordinates */
      y = p_event->xbutton.y;
  
  if (w == l1) {
    display_assocweights (L1WINMOD, l1units, nl1net, l1words, nl1rep, nl1words,
			  SEMWINMOD, sunits, nsnet, swords, nsrep, nswords,
			  x, y, l1sassoc);
    display_assocweights (L1WINMOD, l1units, nl1net, l1words, nl1rep, nl1words,
        L2WINMOD, l2units, nl2net, l2words, nl2rep, nl2words,
        x, y, l1l2assoc); }
  else if (w == l2) {
    display_assocweights (L2WINMOD, l2units, nl2net, l2words, nl2rep, nl2words,
        SEMWINMOD, sunits, nsnet, swords, nsrep, nswords,
        x, y, l2sassoc);
    display_assocweights (L2WINMOD, l2units, nl2net, l2words, nl2rep, nl2words,
        L1WINMOD, l1units, nl1net, l1words, nl1rep, nl1words,
        x, y, l2l1assoc); }
  else {
    display_assocweights (SEMWINMOD, sunits, nsnet, swords, nsrep, nswords,
			  L1WINMOD, l1units, nl1net, l1words, nl1rep, nl1words,
			  x, y, sl1assoc);
    display_assocweights (SEMWINMOD, sunits, nsnet, swords, nsrep, nswords,
        L2WINMOD, l2units, nl2net, l2words, nl2rep, nl2words,
        x, y, sl2assoc); }
}


static void
display_assocweights (srcmodi, srcunits, nsrcnet, srcwords, nsrcrep, nsrcwords,
		      tgtmodi, tgtunits, ntgtnet, tgtwords, ntgtrep, ntgtwords,
		      x, y, assoc)
/* display the associative weights of the unit clicked, and show the label
   of the word rep in the lexicon nearest to the image unit */
   int srcmodi, nsrcnet,		/* source module and map size */
     nsrcrep, nsrcwords,		/* source wordrepsize and #of words */
     tgtmodi, ntgtnet,			/* target module and map size */
     ntgtrep, ntgtwords,		/* target wordrepsize and #of words */
     x, y;				/* location of the mouse click */
   WORDSTRUCT *srcwords, *tgtwords;	/* source and target lexicon */
   FMUNIT srcunits[MAXLSNET][MAXLSNET], 		 /* source map */
     tgtunits[MAXLSNET][MAXLSNET];	    		 /* target map */
   double assoc[MAXLSNET][MAXLSNET][MAXLSNET][MAXLSNET]; /* assoc weights */
   
{
  int i, j;				/* weight index counter */
  int uniti, unitj;			/* weights of this unit are shown */
  int besti, bestj;			/* indices of the image unit */
  double best = (-1), foo = (-1);	/* best and worst response found */
  
  /* only accept points within the network display */
  if (x >= net[srcmodi].marg && y >= titleboxhght &&
      x <= net[srcmodi].marg + nl1net * net[srcmodi].uwidth && 
      y <= titleboxhght + nl1net * net[srcmodi].uhght)
    {
      /* calculate which unit is requested */
      uniti = (x - net[srcmodi].marg) / net[srcmodi].uwidth;
      unitj = (y - titleboxhght) / net[srcmodi].uhght;

      /* turn it and it alone on in the source map */
      for (i = 0; i < nsrcnet; i++)
	for (j = 0; j < nsrcnet; j++)
	  {
	    srcunits[i][j].prevvalue = srcunits[i][j].value;
	    srcunits[i][j].value = 0.0;
	  }
      srcunits[uniti][unitj].value = 1.0;
      display_lex (srcmodi, srcunits, nsrcnet);
      
      /* find and display the label of the closest word in the lexicon */
      sprintf (net[srcmodi].log, "Source unit: %s",
	       srcwords[find_nearest (srcunits[uniti][unitj].comp,
				      srcwords, nsrcrep, nsrcwords)].chars);

      display_log (srcmodi);

      /* establish values to be displayed as activations on the assoc map */
      /* find and display the label of the closest word in the lexicon */
      for (i = 0; i < ntgtnet; i++)
	for (j = 0; j < ntgtnet; j++)
	  {
	    tgtunits[i][j].prevvalue = tgtunits[i][j].value;
	    tgtunits[i][j].value = assoc[uniti][unitj][i][j];
	    updatebestworst (&best, &foo, &besti, &bestj, &tgtunits[i][j],
			     i, j, fgreater, fsmaller);
	  }

      display_lex (tgtmodi, tgtunits, ntgtnet);
      sprintf (net[tgtmodi].log, "Assoc weights: %s",
	       tgtwords[find_nearest (tgtunits[besti][bestj].comp,
				      tgtwords, ntgtrep, ntgtwords)].chars);
      display_log (tgtmodi);
    }
}


/********************* lexicon display subroutines */
void
display_lex (modi, units, nnet)
/* display the map activations */
     int modi,				/* module number */
     nnet;				/* map size */
     FMUNIT units[MAXLSNET][MAXLSNET];	/* map */
{
  int i, j;
  XFontStruct *fontstruct;		/* label font */
  GC currfGC, currbGC, currnGC;		/* dark, light, and net label color */

  /* both semantic input and output go to the same window */
  if (modi == SINPMOD || modi == SOUTMOD)
    modi = SEMWINMOD;
  /* both lexical input and output go to the same window */
  if (modi == L1INPMOD || modi == L1OUTMOD)
    modi = L1WINMOD;
  if (modi == L2INPMOD || modi == L2OUTMOD)
    modi = L2WINMOD;

  /* lexical and semantic fonts and colors could be different */
  if (modi == L1WINMOD)
    {
      fontstruct = l1fontStruct;
      currfGC = l1fGC;
      currbGC = l1bGC;
      currnGC = l1nGC;
    }
  else if (modi == L2WINMOD)
    {
      fontstruct = l2fontStruct;
      currfGC = l2fGC;
      currbGC = l2bGC;
      currnGC = l2nGC;
    }
  else
    {
      fontstruct = semfontStruct;
      currfGC = semfGC;
      currbGC = sembGC;
      currnGC = semnGC;
    }
  /* here i and j are not switched like in discern */
  for (i = 0; i < nnet; i++)
    for (j = 0; j < nnet; j++)
      /* only display if the activation has changed */
      if (units[i][j].value != units[i][j].prevvalue)
	{
    /* color the box representing unit according to activation */
	  frameRectangle (modi, net[modi].marg + i * net[modi].uwidth,
			  titleboxhght + j * net[modi].uhght,
			  net[modi].uwidth, net[modi].uhght,
			  trans_to_color (units[i][j].value, UNITCOLORS));
	  /* display the list of labels on the box */
	  labelbox (modi, net[modi].marg + i * net[modi].uwidth,
		    titleboxhght + j * net[modi].uhght,
		    net[modi].uwidth, net[modi].uhght,
		    units[i][j].value, units[i][j].labels,
		    units[i][j].labelcount, fontstruct,
		    currfGC, currbGC, currnGC);
	}
  XFlush (theDisplay);
}


void
display_error (modi)
  /* write out the error in the map representation */
  int modi;			/* module number */
{
  int i, nearest,
  winmodi = modi;		/* window module number */
  double sum = 0.0;		/* total error */
  WORDSTRUCT *words;		/* lexicon (lexical or semantic) */
  int nrep;			/* rep dimension (lexical or semantic) */
  int nwords;			/* number of words (lexical or semantic) */

  /* first select the right lexicon and establish the output window */
  winmodi = select_lexicon (modi, &words, &nrep, &nwords);

  /* first figure out whether this is input map or output (associative) map */
  if (modi == L1INPMOD || modi == L2INPMOD ||modi == SINPMOD)
    sprintf (net[winmodi].log, "Input");
  else
    sprintf (net[winmodi].log, "Assoc");

  /* calculate the representation error */
  for (i = 0; i < nrep; i++)
    sum += fabs (words[target[modi]].rep[i] - outrep[modi][i]);

  /* find out whether the word is correct */
  nearest = find_nearest (outrep[modi], words, nrep, nwords);
  if (nearest == target[modi])
    sprintf (net[winmodi].log, "%s %s: Eavg %.3f",
	     net[winmodi].log, words[nearest].chars, sum / nrep);
  else
    /* if not, display the correct word as well */
    sprintf (net[winmodi].log, "%s *%s(%s)*: Eavg %.3f",
	     net[winmodi].log,
	     words[nearest].chars, words[target[modi]].chars,
	     sum / nrep);

  display_log (winmodi);
  XFlush (theDisplay);
}


/********************* general routines ***********************/

static void
display_title (modi, name)
/* write the name of the module on top center of the graphics window */
     int modi;				/* module number */
     char name[];			/* module name string */
{
  drawText (modi,
  (net[modi].width - XTextWidth (titlefontStruct, name, strlen (name))) / 2,
	    BOXSP + titlefontStruct->ascent,
	    name, titleGC);
}


static void
display_log (modi)
/* write the log line on top left of the graphics window */
     int modi;				/* module number */
{ 
  clearRectangle (modi, 0, 0, net[modi].width, titleboxhght);
  drawText (modi, net[modi].marg, BOXSP + titlefontStruct->ascent,
	    net[modi].log, logGC);
  /* if we did not overwrite the title, rewrite it */
  if (net[modi].marg +
      XTextWidth (logfontStruct, net[modi].log, strlen (net[modi].log)) <
      (net[modi].width -
       XTextWidth (titlefontStruct, titles[modi], strlen (titles[modi]))) / 2)
    display_title (modi, titles[modi]);
}


static void
frameRectangle (modi, x, y, width, height, colorindex)
/* draw a filled rectangle with a given color and draw a frame around it */
     int modi,				/* module number */
     x, y,				/* top left of the rectangle */
     width, height,			/* dimensions of the rectangle */
     colorindex;			/* fill color */
{
  fillRectangle (modi, x, y, width, height, colorindex);
  drawRectangle (modi, x, y, width, height);
}


static void
labelbox (modi, x, y, width, height, value, labels, labelcount, fontstruct,
	  fGC, bGC, nGC)
/* write the label list on the box representing feature map unit */
     int modi,				/* module number */
     x, y,				/* top left of the box */
     width, height;			/* dimensions of the box */
     double value;			/* activation of the unit */
     char labels[][MAXWORDL + 1];	/* label list */
     int labelcount;			/* number of labels on the list */
     XFontStruct *fontstruct;		/* label font */
     GC fGC, bGC, nGC;			/* dark, light and net colors */
{
  int k,				/* label counter */
    i;
  GC currGC;				/* label color */
  char s[MAXWORDL + 1];			/* copy the string here */
  int nalllabels,			/* number of labels to be displayed */
    firstlabel;				/* index of the first one to be disp */
  
  /* if activation is large, use reverse colors */
  if (value > data.reversevalue)
    currGC = bGC;
  else
    currGC = fGC;

  /* hang them from top if nearestlabls, otherwise center */
  nalllabels = data.nearestlabels ? net[modi].maxlabels : labelcount - 1;
  firstlabel = data.nearestlabels ? 0 : 1;

  /* write each label in the list, one after another in the box */
  for (k = firstlabel; k < labelcount; k++)
    {
      /* if necessary, cut the text from the right to make it fit */
      /* first copy it to something we can always modify */
      sprintf (s, "%s", labels[k]);
      for (i = strlen (s);
	   XTextWidth (fontstruct, s, strlen (s)) > width;
	   s[--i] = '\0')
	;
    drawoverText (modi,
		  (int) (x + 0.5 * (width -
				    XTextWidth (fontstruct, s, strlen (s)))),
		  (int) (y + 0.5 * height + ((k - firstlabel) + 1 -
				    0.5 * nalllabels) * fontstruct->ascent),
		  s,
		  (k == 0) ? nGC : currGC); /* nearestlabels in netcolor */
    }
}


static void
collect_labels (modi, labels, count, label)
/* if there is still room in the list, add the label to it */
     int modi;				/* module number */
     char labels[][MAXWORDL + 1];	/* label list */
     int *count;			/* number of labels */
     char label[];			/* label string */
{
  if (*count == MAXFMLABELS ||
      (*count == net[modi].maxlabels && data.nearestlabels) ||
      (*count == net[modi].maxlabels + 1 && !data.nearestlabels))
    /* list is full; make the last label MORELABEL */
    sprintf (labels[(*count) - 1], MORELABEL);
  else
    /* otherwise add it to the list and increment labelcount */
    sprintf (labels[(*count)++], label);
}


static int
trans_to_color (value, map)
/* translate an activation value into color index */
/* only unitcolors supported in this version */
     double value;			/* activation */
     int map;				/* selects a colormap */
{
  if (map == UNITCOLORS) {
    /* map the number [0,1] to corresponding color */
    return ((long) (((actual_color_range - 1) * value) + 0.499999)); }
  else
    {
      fprintf (stderr, "Wrong colorscale (only unitcolors supported)\n");
      exit (EXIT_X_ERROR);
    }
}


/********************* low-level operations ***********************/

static void
clearRectangle (modi, x, y, width, height)
/* draw a rectangle in the background color */
     int modi, x, y, width, height;
{
  XFillRectangle (theDisplay, Win[modi], clearGC, x, y, width, height);
}


static void
fillRectangle (modi, x, y, width, height, colorindex)
/* draw a filled rectangle in given color */
     int modi, x, y, width, height, colorindex;
{ 
  XSetForeground (theDisplay, activityGC, colors[cmap[colorindex]].pixel);
  XFillRectangle (theDisplay, Win[modi], activityGC, x, y, width, height);
}


static void
drawRectangle (modi, x, y, width, height)
/* draw a rectangle in the box color */
     int modi, x, y, width, height;
{
  XDrawRectangle (theDisplay, Win[modi], boxGC, x, y, width, height);
}


static void
drawText (modi, x, y, text, currGC)
/* draw text image according to given graphics context */
     int modi, x, y;
     char text[];
     GC currGC;
{
  XDrawImageString (theDisplay, Win[modi], currGC, x, y, text, strlen (text));
}


static void
drawoverText (modi, x, y, text, currGC)
/* overwrite text on screen according to give graphics context */
     int modi, x, y;
     char text[];
     GC currGC;
{
  XDrawString (theDisplay, Win[modi], currGC, x, y, text, strlen (text));
}
