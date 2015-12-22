/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */
   
#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"
#include "symtable.h"

struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   size_t blocknr;          
   size_t regnr;
   attr_bitset attributes;
   const string* struct_name; 
   symbol_table* fields;
   symbol_ptr symptr;
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
};


astree* new_astree (int symbol, int filenr, int linenr, int offset, const char* lexinfo);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt3 (astree* root, astree* one, astree* two, astree* three);
astree* ast_swap(astree* root, astree* child, int symbol);
astree* ast_swap2(astree* root, astree* left, astree* right, int symbol);
astree* ast_swap3(astree* root, astree* one, astree* two, astree* three, int symbol);
void ast_rep (astree* node, int symbol);
astree* new_func (astree* child1, astree* child2, astree* child3);
astree* new_proto (astree* child1, astree* child2);
void dump_astree (FILE* outfile, astree* root);
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);

#endif