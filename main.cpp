// $Id: cppstrtok.cpp,v 1.3 2014-10-07 18:09:11-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

using namespace std;

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <wait.h>

#include "auxlib.h"
#include "stringset.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

char *tempOptarg = NULL;
bool dCheck = false; 

// Chomp the last character from a buffer if it is delim.
void chomp (char* string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char* nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

// used https://www.safaribooksonline.com/library/view/c-cookbook/0596007612/ch10s17.html for reference
// parses the filename without the suffix and adds a new extension
void changeExt(string& s, const string& newExt) {
   string::size_type i = s.rfind('.', s.length()); 
   if (i != string::npos) {
      s.replace(i+1, newExt.length(), newExt);
   }
}


// Run cpp against the lines of the file.
void cpplines (FILE* pipe, char* filename) {
   int linenr = 1;
   char inputname[LINESIZE];
   strcpy (inputname, filename);

   // goes through line by line until there are no more to go through
   for (;;) {
      char buffer[LINESIZE];
      char* fgets_rc = fgets (buffer, LINESIZE, pipe);
      if (fgets_rc == NULL) break;
      chomp (buffer, '\n');
      char* savepos = NULL;
      char* bufptr = buffer;

      // checks each token in the line through fgets
      for (int tokenct = 1;; ++tokenct) {
         char* token = strtok_r (bufptr, " \t\n", &savepos);
         bufptr = NULL;
         if (token == NULL) break;
         intern_stringset(token);
      }
      ++linenr;
   }
}

int main (int argc, char** argv) {

   // goes through each option for debugging
   int t;
   while((t = getopt(argc, argv, "ly@:D:")) != -1)
   {
      switch(t) {

      // does nothing yet for -ly
      case 'l':
         break;
      case 'y':
         break;

      // sets debug flags
      case '@':
         set_debugflags(optarg);
         break;
      case 'D':
         dCheck = true;
         tempOptarg = optarg;
         break;

      // lets user know if they entered something invalid
      case '?':
         fprintf(stderr, "Error: incorrect input");
         return get_exitstatus();
      default: 
         fprintf(stderr, "Error: incorrect input default");
         return get_exitstatus();
      }
   }

   // goes through the options until a file is read
   set_execname (argv[0]);
   for (int argi = optind; argi < argc; ++argi) {
      char* filename = argv[argi];
      string command;

      // preprocesses the program using CPP
      if(dCheck == false) {
    	command = CPP + " " + filename;
      } else {
        command = CPP + " -D " + tempOptarg + " " + filename;
      }
      FILE* pipe = popen (command.c_str(), "r");
      if (pipe == NULL) {
         syserrprintf (command.c_str());
      }else {

         // after file opens, interns all the strings, then dumps
         // the hash table into a file "program".str
         cpplines (pipe, filename);
         int pclose_rc = pclose (pipe);
         eprint_status (command.c_str(), pclose_rc);
         char* filen = basename(filename);
	      std::string str(filen);
	      changeExt(str, "str");
	      FILE* strfile = fopen(str.c_str(), "w");
	      dump_stringset(strfile);
	      fclose(strfile);
      }
   }  
   return get_exitstatus();
}
