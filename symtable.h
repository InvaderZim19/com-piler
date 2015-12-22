/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */
   
#ifndef __SYMBOLTABLE__
#define __SYMBOLTABLE__

#include <string>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <queue>
using namespace std;
extern FILE *symfile;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
       ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
       ATTR_prototype, ATTR_variable, ATTR_field, ATTR_typeid,
       ATTR_param, ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
       ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_ptr   = symbol*;
using symbol_table = unordered_map<const string*,symbol*>;
using symbol_entry = pair<const string*,symbol*>;

struct astree;

struct symbol {
   attr_bitset attributes;
   symbol_table *fields;
   const string* struct_name;
   size_t filenr, linenr, offset;
   size_t blocknr;
   vector<symbol*>* parameters;
   astree* node;
   astree* block;
};

symbol* createSymbol(astree* node);
bool addSymbol(symbol_table table, symbol_entry entry);
void traverseBlock();
void enterBlock(astree* root);
int traverseTree(astree* root);
astree* checkNodeType(astree* node);
symbol_entry createStruct(astree* struct_node);
symbol_entry createField(astree* field_node);
symbol_entry createVar(astree* vardecl_node);
symbol_entry createProto(astree* proto_node);
symbol_entry createFunc(astree* func_node);
void checkTypeID(astree* node);
void checkVardecl(astree* root);
void checkFunction(const string* lexinfo, symbol* sym);
void checkProto(const string* lexinfo, symbol* sym);

void compVardecl(astree* root);
void compReturn(astree* root);
void compReturnVoid();
void compAsg(astree* root);
void compEQ(astree* root);
void compComparison(astree* root);
void compBinaryArith(astree* root);
void compUniArith(astree* root);

void compOrd(astree* root);
void compChar(astree* root);
void compNew(astree* root);
void compArray(astree* root);
void compCall(astree* root);
void compVar(astree* root);
void compIndex(astree* root);
void compIfWhile(astree* root);
#endif