#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "astree.h"
#include "symtable.h"
#include "stringset.h"
#include "lyutils.h"

struct symbol;

using symbol_table = unordered_map<string*,symbol*>;
using symbol_entry = symbol_table::value_type;

SymbolTable *tempTable = new SymbolTable(NULL);

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval,
ATTR_const, ATTR_vreg, ATTR_vaddr, ATTR_bitset_size,
}; 

using attr_bitset = bitset<ATTR_bitset_size>;


struct symbol {
	attr_bitset attributes;
	symbol_table* fields;
	size_t filenr, linenr, offset;
	size_t block_nr;
	vector<symbol*>* parameters;
};

int counter = -1;

SymbolTable::SymbolTable(SymbolTable* parent){
	this->parent = parent;
	this->block_nr = counter++;
}

void SymbolTable::insert_symbol(string name, string type, int filenr, int linenr, int offset) {
	this->maptype[name] = type;
	this->mapfile[name] = filenr;
	this->mapline[name] = linenr;
	this->mapoffset[name] = offset;
}

void SymbolTable::dump_sym(FILE* outfile, int depth){
	std::map<string,string>::iterator iter;
	for(iter = this->maptype.begin(); iter != this->maptype.end(); iter++){
		const char* name = iter->first.c_str();
		const char* type = iter->second.c_str();
		fprintf(outfile, "%*s%s (%d. %d. %d) {%d} %2s\n", (int)depth*3, "", name, this->mapfile[name],
		 		this->mapline[name], this->mapoffset[name], depth, type);
		if(this->scope.count(name) > 0) {
			this->scope[name]->dump_sym(outfile, depth + 1);
		}
	}
	std::map<string, SymbolTable*>::iterator k;
	for(k = this->scope.begin(); k != this->scope.end(); ++k) {
		if(this->maptype.count(k->first) < 1) 
			k->second->dump_sym(outfile, depth + 1);
	}

}

SymbolTable* SymbolTable::blockCheck() {
	SymbolTable* inner = new SymbolTable(this);
	char buffer[1000];
	sprintf(&buffer[0], "%d", inner->block_nr);
	this->scope[buffer] = inner;
	return inner;
}

void SymbolTable::traverse(astree* node, SymbolTable* symtab){
	if(node == NULL){
		return;
	}
	for(size_t i = 0; i < node->children.size(); i++){
		const char* current = get_yytname(node->children[i]->symbol);
		// checking for variable declarations
		if(strcmp(current, "TOK_VARDECL") == 0){
			astree* var = node->children[i]->children[0]->children[0];

			// going through all of the cases of types
				if(strcmp(get_yytname(var->symbol), "TOK_ARRAY") == 0) {
				string type = node->children[i]->children[0]->lexinfo->c_str();
				type = type + "[]";
				symtab->insert_symbol(node->children[i]->children[0]->children[1]->lexinfo->c_str(), type, 
				var->filenr, var->linenr, var->offset);
			} else if(strcmp(get_yytname(node->children[i]->children[0]->symbol), "TOK_INT") == 0) {
				if(strcmp(get_yytname(node->children[i]->children[1]->symbol), "TOK_INTCON") == 0){
					string intype = node->children[i]->children[0]->lexinfo->c_str();
					intype = intype + " variable lval";
					symtab->insert_symbol(var->lexinfo->c_str(), intype, var->filenr, var->linenr, var->offset);
				}

			} else if(strcmp(get_yytname(node->children[i]->children[0]->symbol), "TOK_CHAR") == 0) {
				if(strcmp(get_yytname(node->children[i]->children[1]->symbol), "TOK_CHARCON") == 0){
					string intype = node->children[i]->children[0]->lexinfo->c_str();
					intype = intype + " variable lval";
					symtab->insert_symbol(var->lexinfo->c_str(), intype, var->filenr, var->linenr, var->offset);
				}

			} else if(strcmp(get_yytname(node->children[i]->children[0]->symbol), "TOK_STRING") == 0){
				if(strcmp(get_yytname(node->children[i]->children[1]->symbol), "TOK_STRINGCON") == 0){
					string intype = node->children[i]->children[0]->lexinfo->c_str();
					intype = intype + " variable lval";
					symtab->insert_symbol(var->lexinfo->c_str(), intype, var->filenr, var->linenr, var->offset);
				}

			} else if(strcmp(get_yytname(node->children[i]->children[0]->symbol), "TOK_BOOL") == 0){
				if(strcmp(get_yytname(node->children[i]->children[1]->symbol), "TOK_TRUE") == 0){
					string intype = node->children[i]->children[0]->lexinfo->c_str();
					intype = intype + " const";
					symtab->insert_symbol(var->lexinfo->c_str(), intype, var->filenr, var->linenr, var->offset);
				}
			}

		} else if(strcmp(current, "TOK_BLOCK") == 0) {
			traverse(node->children[i], node->children[i]->block = symtab->blockCheck());
			
		} else if(strcmp(current, "TOK_STRUCT") == 0) {
			string indent;
			for(size_t k = 0; k < node->children[i]->children.size(); k++) {
				const char* currentChild = get_yytname(node->children[i]->children[k]->symbol);
				if(strcmp(currentChild, "TOK_TYPEID") == 0) {
					indent = node->children[i]->children[k]->lexinfo->c_str();
					symtab->insert_Struct(indent);
				} 
				else if(strcmp(currentChild, "TOK_INT") == 0 || strcmp(currentChild, "TOK_BOOL") == 0 ||  
					strcmp(currentChild, "TOK_CHAR") == 0 ||  strcmp(currentChild, "TOK_STRING") == 0){
					symtab->insert_symbol(node->children[i]->children[k]->children[0]->lexinfo->c_str(),
						node->children[i]->children[k]->lexinfo->c_str(),
						node->children[i]->children[k]->children[0]->filenr, node->children[i]->children[k]->
						children[0]->linenr, node->children[i]->children[k]->children[0]->offset);
				}
			}
		} else if(strcmp(current, "TOK_FUNCTION") == 0) {
			for(size_t n = 0; n < node->children[i]->children.size(); n++){
				astree* child = node->children[i]->children[n];
				if(strcmp(get_yytname(child->symbol), "TOK_INT") == 0 || strcmp(get_yytname(child->symbol), "TOK_BOOL") == 0 ||
					strcmp(get_yytname(child->symbol), "TOK_CHAR") == 0 ||  strcmp(get_yytname(child->symbol), "TOK_STRING") == 0){
					symtab->insert_symbol(child->children[0]->lexinfo->c_str(), child->lexinfo->c_str(), child->children[0]->filenr, 
						child->children[0]->linenr, child->children[0]->offset);
				} else if(strcmp(get_yytname(child->symbol), "TOK_PARAMLIST") == 0){
					for(size_t p = 0; p < child->children.size(); p++){
						string intype = child->children[p]->lexinfo->c_str();
						if(intype == "TOK_BOOL"){
							intype = intype + " const";
						} else {
							intype = intype + " variable lval param";
						}
						symtab->insert_symbol(child->children[p]->children[0]->lexinfo->c_str(), intype, 
							child->children[p]->children[0]->filenr, child->children[p]->children[0]->linenr, 
							child->children[p]->children[0]->offset);
					}
				} else if(strcmp(get_yytname(child->symbol), "TOK_BLOCK") == 0){
					traverse(child, child->block = symtab->blockCheck());
				}
			}
		} else if(strcmp(current, "TOK_WHILE") == 0){
			astree* c = node->children[i]->children[1];
			traverse(c, c->block = symtab->blockCheck());
		} else if(strcmp(current, "TOK_IF") == 0){
			astree* c = node->children[i]->children[1];
			traverse(c, c->block = symtab->blockCheck());
		} else if(strcmp(current, "TOK_IFELSE") == 0){
			astree* c = node->children[i]->children[1];
			traverse(c, c->block = symtab->blockCheck());
		
		} else {
			traverse(node->children[i], symtab);
		}
	}
}

void SymbolTable::insert_Struct(string n) {
	this->structs[n] = new SymbolTable(NULL);
}

SymbolTable* SymbolTable::lookup_Struct(string n) {
	if(this->structs.count(n) > 0) {
		return this->structs[n];
	}
	if(this->parent != NULL) {
		return this->parent->lookup_Struct(n);
	} else {
		errprintf("Struct invalid.\n");
		return NULL;
	}
}

string SymbolTable::lookup_Map(string n) {
	if(this->mapfile.count(n) > 0) {
		return this->maptype[n];
	}
	if(this->parent != NULL) {
		return this->parent->lookup_Map(n);
	} else {
		errprintf("Invalid Identifier: %s\n", n.c_str());
		return "";
	}
}