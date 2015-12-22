#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

class astree;
class SymbolTable {

  int block_nr;
  SymbolTable* parent;
  map<string,string> maptype;
  map<string,int> mapfile;
  map<string,int> mapline;
  map<string,int> mapoffset;
  map<string,SymbolTable*> structs;
  map<string,SymbolTable*> scope;


  
public:

  SymbolTable(SymbolTable* parent);
 // SymbolTable* enterBlock();
  //SymbolTable* enterFunction(string name,
  //                           string signature, int filenr, int linenr, int offset);
  
  void insert_symbol(string name, string type, int filenr, int linenr, int offset);

  //void addStruct(string name);

  void dump_sym(FILE* symfile, int depth);

 // string lookup(string name);

  //SymbolTable* lookup2(string name);
  SymbolTable* blockCheck();

 //string parentFunction(SymbolTable* innerScope);

  void traverse(astree* node, SymbolTable* symtab);
  string typeChecker(astree* node, SymbolTable* symtab);
  void insert_Struct(string n);
  SymbolTable* lookup_Struct(string n);
  string lookup_Map(string n);
  //static int N;

  //static vector<string> parseSignature(string signature);


  
};



#endif