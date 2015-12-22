/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */
   
#include "symtable.h"
#include "astree.h"
#include "lyutils.h"
#include "emit.h"

astree* currentFunc = nullptr;
queue<astree*> stringQueue;
queue<symbol*> functionQueue;
symbol_table* globalTable;
symbol_table* structTable = new symbol_table();
int next_block = 1;
vector<int> blnr(1,0);
int traverse_rc = 0;
vector<symbol_table*> symbolStack;
vector<symbol*> allSym;


// creates symbol with all of the traits from node
symbol* createSymbol(astree* node) {
    symbol* sym = new symbol();
    sym->attributes = node->attributes;
    sym->filenr = node->filenr;
    sym->linenr = node->linenr;
    sym->offset = node->offset;
    sym->parameters = nullptr;
    sym->block = nullptr;
    sym->struct_name = node->struct_name;
    sym->fields = node->fields;
    sym->node = node;
    node->symptr = sym;
    allSym.push_back(sym);
    return sym;
}

// Inserts entry into specified symbol table, goes throuh all the options
bool addSymbol (symbol_table *table, symbol_entry entry) {
    if (table == nullptr) {
        table = new symbol_table();
    }
    symbol* sym = entry.second;
    if (table == symbolStack.back()) {
        for (unsigned i = 0; i < blnr.size() - 1; i++)
            fprintf(symfile, "   ");
    }
    fprintf(symfile, "%s (%lu.%lu.%lu) {%lu} ",
        entry.first->c_str(), sym->filenr,
        sym->linenr, sym->offset, sym->blocknr);
    if (sym->attributes.test(ATTR_void)){
        fprintf(symfile, "void ");
    }
    if (sym->attributes.test(ATTR_int)){
        fprintf(symfile, "int ");
    }
    if (sym->attributes.test(ATTR_char)){
        fprintf(symfile, "char ");
    }
    if (sym->attributes.test(ATTR_bool)){
        fprintf(symfile, "bool ");
    }
    if (sym->attributes.test(ATTR_string)){
        fprintf(symfile, "string ");
    }
    if (sym->attributes.test(ATTR_null)){
        fprintf(symfile, "null ");
    }
    if (sym->attributes.test(ATTR_array)){
        fprintf(symfile, "array ");
    }
    if (sym->attributes.test(ATTR_struct)) {
        fprintf(symfile, "struct ");
        if (sym->struct_name != nullptr){
            fprintf(symfile, "\"%s\" ", sym->struct_name->c_str());
        }
    }
    if (sym->attributes.test(ATTR_function)){
        fprintf(symfile, "function ");
    }
    if (sym->attributes.test(ATTR_prototype)){
        fprintf(symfile, "prototype ");
    }
    if (sym->attributes.test(ATTR_variable)){
        fprintf(symfile, "variable ");
    }
    if (sym->attributes.test(ATTR_field)){
        fprintf(symfile, "field ");
    }
    if (sym->attributes.test(ATTR_typeid)){
        fprintf(symfile, "typeid ");
    }
    if (sym->attributes.test(ATTR_param)){
        fprintf(symfile, "param ");
    }
    if (sym->attributes.test(ATTR_lval)){
        fprintf(symfile, "lval ");
    }

    fprintf(symfile, "\n");
    return table->insert(entry).second;
}

// Enter new nested block 
void traverseBlock(astree* block_node) {
    enterBlock(block_node);
    for (size_t child = 0; child < block_node->children.size(); child++) {
        traverseTree(block_node->children[child]);
    }
    
}

void enterBlock(astree* block_node) {
    block_node->blocknr = blnr.back();
    symbolStack.push_back(new symbol_table());
    blnr.push_back(next_block);
    next_block++;
}



// Traverse astree and build symbol tables
int traverseTree(astree* root) {
    if (root->symbol == TOK_FUNCTION) {
        astree* nodetype = root->children.at(0);
        if (nodetype->symbol == TOK_ARRAY) {
            currentFunc = nodetype->children.at(1);
        } else {
            currentFunc = nodetype->children.at(0);
        }
    }
    switch (root->symbol) {
        case TOK_ROOT:
            symbolStack.push_back(new symbol_table());
            globalTable = symbolStack.back();
            break;
        case TOK_STRUCT:
            createStruct(root);
            return traverse_rc;
        case TOK_BLOCK:
            traverseBlock(root);
            return traverse_rc;
        case TOK_VARDECL:
            createVar(root);
            return traverse_rc;
        case TOK_PROTOTYPE:
            createProto(root);
            return traverse_rc;
        case TOK_FUNCTION:
            createFunc(root);
            return traverse_rc;
    }
    for (size_t child = 0; child < root->children.size(); child++) {
        traverseTree(root->children[child]);
    }
    root->blocknr = blnr.back();
    switch (root->symbol) {
        case '=':
            compAsg(root);
            break;
        case TOK_RETURN:
            compReturn(root);
            break;
        case TOK_RETURNVOID:
            compReturnVoid();
            break;
        case TOK_EQ:
        case TOK_NE:
            compEQ(root);
            break;
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            compComparison(root);
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            compBinaryArith(root);
            break;
        case TOK_POS:
        case TOK_NEG:
            compUniArith(root);
            break;
        case TOK_CHR:
            compChar(root);
            break;
        case TOK_NEWSTRING:
            root->attributes.set(ATTR_string);
            root->attributes.set(ATTR_vreg);
            break;
            break;
        case TOK_CALL:
            compCall(root);
            break;
        case TOK_IDENT:
            compVar(root);
            break;
        case TOK_INDEX:
            compIndex(root);
            break;
        case TOK_INTCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_int);
            break;
        case TOK_CHARCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_char);
            break;
        case TOK_STRINGCON:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_string);
            stringQueue.push(root);
            break;
        case TOK_FALSE:
        case TOK_TRUE:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_bool);
        case TOK_NULL:
            root->attributes.set(ATTR_const);
            root->attributes.set(ATTR_null);
            break;
        case TOK_WHILE:
        case TOK_IF:
            compIfWhile(root);
            break;
        case TOK_FIELD:
            root->attributes.set(ATTR_field);
    }
    return traverse_rc;
}

astree* checkNodeType(astree* nodetype) {
    astree* node;
    int type;
    if (nodetype->symbol == TOK_ARRAY) {
        node = nodetype->children.at(1);
        nodetype = nodetype->children.at(0);
        nodetype->blocknr = blnr.back();
        node->attributes.set(ATTR_array);
    } else {
        node = nodetype->children.at(0);
    }
    type = nodetype->symbol;
    switch (type) {
        case TOK_BOOL:
            node->attributes.set(ATTR_bool);
            break;
        case TOK_CHAR:
            node->attributes.set(ATTR_char);
            break;
        case TOK_INT:
            node->attributes.set(ATTR_int);
            break;
        case TOK_STRING:
            node->attributes.set(ATTR_string);
            break;
        case TOK_VOID:
            node->attributes.set(ATTR_void);
            break;
        case TOK_TYPEID:
            checkTypeID(nodetype);
            symbol_table::const_iterator got;
            got = structTable->find(nodetype->lexinfo);
            node->struct_name = nodetype->lexinfo;
            node->fields = got->second->fields;
            node->attributes.set(ATTR_struct);
            break;
    }
    return node;
}

symbol_entry createStruct(astree* structentry) {
    astree* type = structentry->children.at(0);
    type->struct_name = type->lexinfo;
    type->attributes.set(ATTR_typeid);
    symbol* sym = createSymbol(type);
    sym->fields = new symbol_table();
    symbol_entry entry = make_pair(type->lexinfo, sym);
    addSymbol(structTable, entry);
    for (size_t child = 1; child < structentry->children.size(); child++) {
       astree* field = structentry->children.at(child);
        fprintf(symfile, "   ");
        addSymbol(sym->fields, createField(field));
    }
    return entry;
}

symbol_entry createField(astree* field) {
    astree* node = checkNodeType(field);
    node->attributes.set(ATTR_field);
    symbol* sym = createSymbol(node);
    symbol_entry entry = make_pair(node->lexinfo, sym);
    return entry;
}

symbol_entry createVar(astree* vard) {
    astree* nodetype = vard->children.at(0);
    astree* node = checkNodeType(nodetype);
    node->attributes.set(ATTR_variable);
    node->attributes.set(ATTR_lval);
    symbol* sym = createSymbol(node);
    checkVardecl(node);
    symbol_entry entry = make_pair(node->lexinfo, sym);
    addSymbol(symbolStack.back(), entry);
    traverseTree(vard->children.at(1));
    return entry;
}

symbol_entry createProto(astree* p) {
    astree* nodetype = p->children.at(0);
    astree* param_node = p->children.at(1);
    astree* node = checkNodeType(nodetype);
    node->attributes.set(ATTR_prototype);
    symbol* sym = createSymbol(node);
    sym->parameters = new vector<symbol*>();
    functionQueue.push(sym);
    if (param_node->children.size() > 0) {
        enterBlock(param_node);
        for (size_t child = 0; child < param_node->children.size(); child++) {
            astree* ptype_node = param_node->children.at(child);
            astree* pnode = checkNodeType(ptype_node);
            pnode->attributes.set(ATTR_variable);
            pnode->attributes.set(ATTR_lval);
            pnode->attributes.set(ATTR_param);
            symbol* psym = createSymbol(pnode);
            sym->parameters->push_back(psym);
        }
    }
    checkProto(node->lexinfo, sym);
    symbol_entry entry = make_pair(node->lexinfo, sym);
    addSymbol(symbolStack.back(), entry);
    return entry;
}

symbol_entry createFunc(astree* func_node) {
    astree* nodetype = func_node->children.at(0);
    astree* param_node = func_node->children.at(1);
    astree* block_node = func_node->children.at(2);
    astree* node = checkNodeType(nodetype);
    node->attributes.set(ATTR_function);
    symbol* sym = createSymbol(node);
    sym->parameters = new vector<symbol*>();
    sym->block = block_node;
    functionQueue.push(sym);
    symbol_entry entry = make_pair(node->lexinfo, sym);
    enterBlock(param_node);
    for (size_t child = 0; child < param_node->children.size(); child++) {
        astree* ptype_node = param_node->children.at(child);
        astree* pnode = checkNodeType(ptype_node);
        pnode->attributes.set(ATTR_variable);
        pnode->attributes.set(ATTR_lval);
        pnode->attributes.set(ATTR_param);
        symbol* psym = createSymbol(pnode);
        sym->parameters->push_back(psym);
        addSymbol(symbolStack.back(), make_pair(pnode->lexinfo, psym));
    }
    checkFunction(node->lexinfo, sym);
    addSymbol(symbolStack.front(), entry);
    for (size_t child = 0; child < block_node->children.size(); child++) {
        traverseTree(block_node->children[child]);
    }
    
    currentFunc = nullptr;
    return entry;
}

void checkTypeID(astree* node) {
    const string* lexinfo = node->lexinfo;
    symbol_table::const_iterator got = structTable->find(lexinfo);
    symbol *sym;
    if (got == structTable->end()) {
        sym = createSymbol(node);
        addSymbol(structTable, make_pair(lexinfo, sym));
    }
    node->attributes.set(ATTR_typeid);
}

void checkVardecl(astree* root) {
    symbol_table* table = symbolStack.back();
    symbol_table::const_iterator got = table->find(root->lexinfo);
    if (got == symbolStack.back()->end()) {
        return;
    }
}

void checkProto(const string* lexinfo, symbol* p_sym) {
    symbol* sym;
    symbol_table* table = symbolStack.back();
    symbol_table::const_iterator got = table->find(lexinfo);
    if (got == symbolStack.back()->end()) {
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            return;
        } else if (attributes != p_sym->attributes) {
            return;
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes != sym->parameters->at(i)->attributes) {
                return;
            }
        }
    } else if (attributes.test(ATTR_function)) {
        if (p_sym->parameters->size() != sym->parameters->size()) {
            return;
        }
        attributes.set(ATTR_function, 0);
        attributes.set(ATTR_prototype);
        if (attributes != p_sym->attributes) {
            return;
        }
        for (unsigned i = 0; i < p_sym->parameters->size(); i++) {
            if (p_sym->parameters->at(i)->attributes != sym->parameters->at(i)->attributes) {
                return;
            }
        }
    }
}

void checkFunction(const string* lexinfo, symbol* f_sym) {
    symbol* sym;
    symbol_table* table = symbolStack.back();
    symbol_table::const_iterator got = table->find(lexinfo);
    if (got == symbolStack.back()->end()) {
        return;
    }
    sym = got->second;
    attr_bitset attributes(sym->attributes);
    if (attributes.test(ATTR_prototype)) {
        if (f_sym->parameters->size() != sym->parameters->size()) {
            return;
        }
        attributes.set(ATTR_prototype, 0);
        attributes.set(ATTR_function);
        if (attributes != f_sym->attributes) {
            return;
        }
        for (unsigned i = 0; i < f_sym->parameters->size(); i++) {
            if (f_sym->parameters->at(i)->attributes != sym->parameters->at(i)->attributes) {
                return;
            }
        }
    }
}

bool types_compatible(astree* n1, astree* n2) {
    attr_bitset attr1 = n1->attributes;
    attr_bitset attr2 = n2->attributes;
    if ((attr1.test(ATTR_null) &&  (attr2.test(ATTR_string) || attr2.test(ATTR_struct) || attr2.test(ATTR_array)))|| (attr2.test(ATTR_null) &&  (attr1.test(ATTR_string) || attr1.test(ATTR_struct)|| attr1.test(ATTR_array)))){
        return true;
    }
    if ((attr1.test(ATTR_array) && !attr2.test(ATTR_array)) || (attr1.test(ATTR_array) && !attr2.test(ATTR_array))){
        return false;
    }
    if ((attr1.test(ATTR_int) && attr2.test(ATTR_int)) || (attr1.test(ATTR_char) && attr2.test(ATTR_char)) || (attr1.test(ATTR_string) && attr2.test(ATTR_string)) || (attr1.test(ATTR_bool) && attr2.test(ATTR_bool))) {
        return true;
    } else if (attr1.test(ATTR_struct) && attr2.test(ATTR_struct)) {
        if (n1->struct_name == n2->struct_name){
            return true;
        }
    }
    return false;
}

bool types_compatible(astree* n, symbol* s) {
    attr_bitset attr1 = n->attributes;
    attr_bitset attr2 = s->attributes;
    if ((attr1.test(ATTR_null) &&  (attr2.test(ATTR_string) || attr2.test(ATTR_struct) || attr2.test(ATTR_array))) || (attr2.test(ATTR_null) &&  (attr1.test(ATTR_string) || attr1.test(ATTR_struct) || attr1.test(ATTR_array)))){
        return true;
    }
    if ((attr1.test(ATTR_array) && !attr2.test(ATTR_array)) || (attr1.test(ATTR_array) && !attr2.test(ATTR_array))){
        return false;
    }
    if ((attr1.test(ATTR_int) && attr2.test(ATTR_int)) || (attr1.test(ATTR_char) && attr2.test(ATTR_char)) || (attr1.test(ATTR_string) && attr2.test(ATTR_string)) || (attr1.test(ATTR_bool) && attr2.test(ATTR_bool))) {
        return true;
    } else if (attr1.test(ATTR_struct) && attr2.test(ATTR_struct)) {
        if (n->struct_name == s->struct_name){
            return true;
        }
    }
    return false;
}

bool type_exists(const string* struct_name) {
    symbol_table::const_iterator got = structTable->find(struct_name);
    if (got == structTable->end()){
        return false;
    }
    return true;
}

void compVardecl(astree* root) { 
    astree* node;
    astree* nodetype = root->children.at(0);
    astree* expr_node = root->children.at(1);
    attr_bitset type_bits;
    if (nodetype->symbol == TOK_ARRAY) {
        node = nodetype->children.at(1);
    } else {
        node = nodetype->children.at(0);
    }
    if (expr_node->attributes.test(ATTR_null)) {
        if (node->attributes.test(ATTR_string) || node->attributes.test(ATTR_struct) || node->attributes.test(ATTR_array)) {
            return;
        } 
    } else if (types_compatible(expr_node, node)) {
        return;
    }
    traverse_rc = 1;
}
void compAsg(astree* root) { 
    if (!root->children.at(0)->attributes.test(ATTR_lval)) {
        traverse_rc = 1;
        return;
    }
    if (!types_compatible(root->children.at(0), root->children.at(1))) {
        traverse_rc = 1;
        return;
    }
    root->attributes = attr_bitset(root->children.at(0)->attributes);
    root->attributes.set(ATTR_lval, 0);
    root->attributes.set(ATTR_variable, 0);
    root->attributes.set(ATTR_vreg);
    root->attributes.set(ATTR_vaddr, 0);
}

void compReturn(astree* root) {
    if (currentFunc == nullptr) {
        traverse_rc = 1;
        return;
    }
    astree* expr_node = root->children.at(0);
    if (expr_node->attributes.test(ATTR_null)) {
        if (currentFunc->attributes.test(ATTR_string) || currentFunc->attributes.test(ATTR_struct) || currentFunc->attributes.test(ATTR_array))
            return;
    } else if (types_compatible(expr_node, currentFunc)) {
        return;
    }
}

void compReturnVoid() {
    if (currentFunc != nullptr) {
        if (currentFunc->attributes.test(ATTR_void)) {
            return;
        }
    }
}

void compEQ(astree* root) { 
    if (types_compatible(root->children.at(0), root->children.at(1))) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
    }
}

void compComparison(astree* root) { 
    attr_bitset attr1 = root->children.at(0)->attributes;
    attr_bitset attr2 = root->children.at(1)->attributes;
    if (attr1.test(ATTR_int) && attr2.test(ATTR_int)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (attr1.test(ATTR_char) && attr2.test(ATTR_char)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    } else if (attr1.test(ATTR_bool) && attr2.test(ATTR_bool)) {
        root->attributes.set(ATTR_bool);
        root->attributes.set(ATTR_vreg);
        return;
    }
}

void compBinaryArith(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int) && root->children.at(1)->attributes.test(ATTR_int) && !root->children.at(0)->attributes.test(ATTR_array) && !root->children.at(1)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
        return;
    }
}

void compUniArith(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int) && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_int);
    }
    return; 
}

void compChar(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_int) && !root->children.at(0)->attributes.test(ATTR_array)) {
        root->attributes.set(ATTR_vreg);
        root->attributes.set(ATTR_char);
    }
    return; 
}

void compCall(astree* root) { 
    symbol_table *table;
    symbol_table::const_iterator got;
    for(int i = (int) symbolStack.size() - 1; i >= 0; i--) {
        table = symbolStack.at(i);
        got   = table->find(root->children.at(0)->lexinfo);
        if (got != table->end()) {
            vector<astree*> args = root->children;
            root->attributes = attr_bitset(got->second->attributes);
            root->attributes.set(ATTR_vreg);
            if (root->attributes.test(ATTR_struct)) {
                root->struct_name = got->second->struct_name;
                root->fields = got->second->fields;
            }
            return;
        }
    }
    traverse_rc = 1;
}
void compVar(astree* root) { 
    symbol_table *table;
    symbol_table::const_iterator got;
    for(int i = (int) symbolStack.size() - 1; i >= 0; i--) {
        table = symbolStack.at(i);
        got = table->find(root->lexinfo);
        if (got != table->end()) {
            root->attributes = attr_bitset(got->second->attributes);
            root->symptr = got->second;
            if (root->attributes.test(ATTR_struct)) {
                root->fields = got->second->fields;
                root->struct_name = got->second->struct_name;
            }
            return;
        }
    }
    traverse_rc = 1;
    
}
void compIndex(astree* root) { 
    if (root->children.at(0)->attributes.test(ATTR_bool)) {
        root->attributes.set(ATTR_bool);
    } else if (root->children.at(0)->attributes.test(ATTR_int)) { 
        root->attributes.set(ATTR_int);
    } else if (root->children.at(0)->attributes.test(ATTR_char)) {
        root->attributes.set(ATTR_char);
    } else if (root->children.at(0)->attributes.test(ATTR_string)) {
        if (root->children.at(0)->attributes.test(ATTR_array)) {
            root->attributes.set(ATTR_string);
        } else {
            root->attributes.set(ATTR_char);
        }
    } else if (root->children.at(0)->attributes.test(ATTR_struct)) {
        root->attributes.set(ATTR_struct);
        root->struct_name = root->children.at(0)->lexinfo;
        root->fields = root->children.at(0)->fields;
    }
    root->attributes.set(ATTR_vaddr);
    root->attributes.set(ATTR_lval);
}

void compIfWhile(astree* root) {
    if (!root->children.at(0)->attributes.test(ATTR_bool)) {
        traverse_rc = 1;
        return;
    }
}