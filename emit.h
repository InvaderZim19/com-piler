/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */
   
#ifndef __EMIT__
#define __EMIT__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <queue>

#include "symtable.h"
#include "astree.h"
#include "yyparse.h"
using namespace std;

typedef symbol_table::iterator symtab_it;

extern FILE *oilfile;
extern symbol_table* structTable;
extern queue<astree*> stringQueue;
extern queue<symbol*> functionQueue;
extern vector<symbol_table*> symbolStack;
extern symbol_table* globalTable;

void emitRun(astree* root);
void emitStructs();
void emitStringcon();
void emitVardecls();
void emitFunction();

void emitComp(astree* root);
void emitChar(astree* root);
void emitSign(astree* root);
void emitNewStr(astree* root);
void emitIndex(astree* root);

void emitExpr(astree* node);
void emitStatement(astree* node);
void emitWhile(astree* root);
void emitIfElse(astree* root);
void emitIf(astree* root);
void emitReturn(astree* root);
void emitVardecl1(astree* root);
void emitAssign(astree* root);
void emitCall(astree* root);

void emitOp(astree* op);
void emitBinArith(astree* root);
char regType(astree* node);

astree* getID(astree* vardecl_node);
#endif