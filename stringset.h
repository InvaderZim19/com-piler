// $Id: stringset.h,v 1.3 2015-03-25 19:03:01-07 - - $

// header file for stringset.cpp

/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 3
   stringset.h */

#ifndef __STRINGSET__
#define __STRINGSET__

#include <string>

#include <stdio.h>

const std::string* intern_stringset (const char*);

void dump_stringset (FILE*);

#endif
