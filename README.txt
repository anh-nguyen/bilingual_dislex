DISLEX: The DISCERN lexicon model
=================================
Version 2.1 1/3/99

Copyright (C) 1994,1997,1998,1999 Risto Miikkulainen

This package contains the C-code and data for training and testing the
DISLEX model of the lexicon, which is also part of the DISCERN story
processing system. DISLEX consists feature maps for lexical symbols and
of word meanings, and associative connections between them that
translate between symbols and meanings. DISLEX includes a "real-time"
X11 graphics display for visualization and debugging and routines for
collecting performance statistics throughout training and testing. See
the file INSTALL for instructions on how to install DISLEX; see USERDOC
for instructions on how to run it; see FILES for explanation on the
software package itself.

DISLEX is described in detail in
@Article{miikkulainen:bl97,
  author       = "Risto Miikkulainen",
  title	       = "Dyslexic and Category-Specific Impairments in a
                  Self-Organizing Feature Map Model of the Lexicon",
  journal      = "Brain and Language",
  year	       = 1997,
  volume       = 59,
  pages	       = "334--366",
}
which is also available in the World Wide Web under NLP publications in
http://www.cs.utexas.edu/users/nn. The DISCERN system is described in
@Book{miikkulainen:subsymbolic,
  author = 	"Risto Miikkulainen",
  title = 	"Subsymbolic Natural Language Processing: {A}n
		 Integrated Model of Scripts, Lexicon, and Memory",
  publisher = 	"MIT Press",
  year = 	"1993",
  address = "Cambridge, MA"
}

The fully trained DISCERN system is available by anonymous ftp from
ftp.cs.utexas.edu:pub/neural-nets/ discern/discern.tar.Z, and in World
Wide Web under software in http://www.cs.utexas.edu/users/nn. It was put
together from the final results of three different programs available in
the discern directory: PROC developed the processing modules, DISLEX
(this package) the lexicon, and HFM organized the episodic memory. Even
though DISLEX was developed as part of DISCERN, it itself can serve as a
starting point for various experiments in modular connectionist NLP
architectures. It includes code for self-organizing feature maps and
Hebbian associative connections that form many-to-many mappings between
maps. There are also general routines for managing the simulation,
collecting performance statistics, and visualizing the feature map
organization and associative connections on a "real-time" X11 graphics
display.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as published
by the Free Software Foundation. This program is distributed in the hope
that it will be useful, but without any warranty; without even the
implied warranty of merchantability or fitness for a particular purpose.
See the GNU General Public License for more details.

We hope that this software will be a useful starting point for your own
explorations in connectionist NLP. The software is provided as is,
however, we will do our best to maintain it and accommodate
suggestions. If you want to be notified of future releases of the
software or have questions, comments, bug reports or suggestions, send
email to discern@cs.utexas.edu.


Versions
--------
v1.0 10/3/91 risto
- HP Starbase graphics version (called lex)

v2.0 9/21/94 risto
- X11 graphics
- data and simulation management rewritten

v2.0.1 1/8/97 risto
- fixed color allocation bug (screens with more than 256 colors)

v2.0.2 9/27/98 risto
- fixed "within" statistics bug

v2.1 1/3/99 risto
- added the phonological map simulation data
