// $Id: cppstrtok.cpp,v 1.3 2014-10-07 18:09:11-07 - - $

// Use cpp to scan a file and print line numbers.
// Print out each input line read in, then strtok it for
// tokens.

/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */

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
#include "astree.h"
#include "lyutils.h"
#include "symtable.h"
#include "emit.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
extern int yy_flex_debug;
extern FILE* yyin;
extern astree* yyparse_astree;
FILE* strfile;
FILE* tokfile;
FILE* astfile;
FILE* symfile;
FILE* oilfile;

// used https://www.safaribooksonline.com/library/view/c-cookbook/0596007612/ch10s17.html for reference
// parses the filename without the suffix and adds a new extension
void changeExt(string& s, const string& newExt) {
   string::size_type i = s.rfind('.', s.length()); 
   if (i != string::npos) {
      s.replace(i+1, newExt.length(), newExt);
   }
}

// function to check to see if the typed file is a .oc file
int ocCheck(char* filename){
   std::string temp(filename);
   std::size_t pos = temp.find(".");
   if(temp.substr(pos).compare(".oc")){
      fprintf(stderr, "Error: file is not a .oc file \n");
      return get_exitstatus();
   } else {
      return 1;
   }
}

int main (int argc, char** argv) {

   // goes through each option for debugging
   char *tempOptarg = NULL;
   bool dCheck = false; 
   int t;
   yy_flex_debug = 0;
   while((t = getopt(argc, argv, "ly@:D:")) != -1)
   {
      switch(t) {
      case 'l':
         yy_flex_debug = 1;
         break;
      case 'y':
         yydebug = 1;
         break;
      case '@':
         set_debugflags(optarg);
         break;
      case 'D':
         dCheck = true;
         tempOptarg = optarg;
         break;
      case '?':
         fprintf(stderr, "Error: incorrect input \n");
         return get_exitstatus();
      default: 
         fprintf(stderr, "Error: incorrect input default \n");
         return get_exitstatus();
      }
   }

   // goes through the options until a file is read
   set_execname (argv[0]);
   for (int argi = optind; argi < argc; ++argi) {

      // variables for the CPP
      char* filename = argv[argi];
      string command;
      char* filen = basename(filename);

      std::string ast(filen);
      changeExt(ast, "ast");
      astfile = fopen(ast.c_str(), "w");

      // creates the program.tok file
      std::string tok(filen);
      changeExt(tok, "tok");
      tokfile = fopen(tok.c_str(), "w");

      // creates the program.str file
      std::string str(filen);
      changeExt(str, "str");
      strfile = fopen(str.c_str(), "w");

      // creates the program.str file
      std::string sym(filen);
      changeExt(sym, "sym");
      symfile = fopen(sym.c_str(), "w");

      std::string oil(filen);
      changeExt(oil, "oil");
      oilfile = fopen(oil.c_str(), "w");

      // preprocesses the program using CPP
      if(dCheck == false) {
          command = CPP + " " + filename;
      } else {
          command = CPP + " -D " + tempOptarg + " " + filename;
      }
      yyin = popen (command.c_str(), "r");

      if (yyin == NULL) {
         syserrprintf (command.c_str());
      } else {
         
         // checks to see if the file is specifically a .oc file
         ocCheck(filename);

         // loops through file adding tokens to program.tok and creating astree. stops at end of file
         int z; 
         while((z = yyparse())){
            if(z == YYEOF){
               break;
            }
         }
         
         // after file opens, interns all the strings, then dumps the hash table into a file "program".str
         int pclose_rc = pclose (yyin);
         eprint_status (command.c_str(), pclose_rc);

         int traverse_rc = traverseTree(yyparse_astree);

         // generate .oil file
         if (traverse_rc == 0){
            emitRun(yyparse_astree);
         }     

         dump_stringset(strfile);

         dump_astree(astfile, yyparse_astree);

         // closing all the files afterwards
         fclose(strfile);
         fclose(tokfile);
         fclose(astfile);
         fclose(symfile);
         fclose(oilfile);

         yylex_destroy();
      }
   }  
   return get_exitstatus();
}
