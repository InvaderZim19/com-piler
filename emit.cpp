/* Karl Cassel (1372617) kcassel
   Wesly Lim   (1366779) welim
   Program 5
   11/24/2015 */
   
#include "emit.h"

size_t count;
string type(symbol* sym) {
    string value = "";
    if (sym->attributes.test(ATTR_void)) {
        value += "void ";
        return value;
    }
    if (sym->attributes.test(ATTR_bool) || sym->attributes.test(ATTR_char)) {
    	value += "char ";
    } else if (sym->attributes.test(ATTR_int)) {
        value += "int ";
    } else if (sym->attributes.test(ATTR_string)) {
        value += "char* ";
    } 
    if (sym->attributes.test(ATTR_array)) {
        value += "* ";
    }
    return value;
}

string type(astree* node) {
    string value = "";
    if (node->attributes.test(ATTR_void)) {
        value += "void ";
        return value;
    }
    if (node->attributes.test(ATTR_bool) || node->attributes.test(ATTR_char)) {
        value += "char";
    } else if (node->attributes.test(ATTR_int)) {
        value += "int";
    } else if (node->attributes.test(ATTR_string)) {
        value += "char*";
    }
    if (node->attributes.test(ATTR_array)) {
        value += "* ";
    }
    return value;
}

void emitStructs() {
    for (symtab_it iterator =  structTable->begin(); iterator != structTable->end(); iterator++) {
        fprintf(oilfile, "struct s_%s {\n", iterator->first->c_str());
        symbol_table* field_table = iterator->second->fields;
        for (symtab_it f_iterator =  field_table->begin(); f_iterator != field_table->end(); f_iterator++) {
            fprintf(oilfile, "        ");
            fprintf(oilfile, "%s f_%s_%s;\n", type(f_iterator->second).c_str(),  iterator->first->c_str(), f_iterator->first->c_str());
        }
        fprintf(oilfile, "};\n");
    }
}

void emitStringcon() {
    while (!stringQueue.empty()) {
        astree* node  = stringQueue.front();
        stringQueue.pop();
        const string* value = node->lexinfo;
        fprintf(oilfile, "char* s%lu = %s;\n", count, value->c_str());
    
        count++;
    }
}

void emitVardecls() {
    for (symtab_it iterator =  globalTable->begin(); iterator != globalTable->end(); iterator++) {
        if (iterator->second->attributes.test(ATTR_variable)) {
            fprintf(oilfile, "%s __%s;\n", type(iterator->second).c_str(), iterator->first->c_str());
        }
    }
}

void emitFunction() {
    for (int i = 1; !functionQueue.empty(); i++) {
        symbol *sym  = functionQueue.front();
        functionQueue.pop();
        fprintf(oilfile, "%s __%s (", type(sym).c_str(), sym->node->lexinfo->c_str());
        vector<symbol*>* params = sym->parameters;
        for (unsigned i = 0; i < params->size(); i++) {
            fprintf(oilfile, "\n");
            fprintf(oilfile, "        ");
            fprintf(oilfile, "%s _%lu_%s", type(params->at(i)).c_str(), params->at(i)->blocknr, params->at(i)->node->lexinfo->c_str());
            if (i < params->size() - 1)
                fprintf(oilfile, ",");
        }
        if (sym->attributes.test(ATTR_function)) {
            fprintf(oilfile, ")\n");
            fprintf(oilfile, "{\n");
            emitStatement(sym->block);
            fprintf(oilfile, "}\n");
        } else {
            fprintf(oilfile, ");\n");
        }
    }
}

void emitStatement(astree* node) {
    switch (node->symbol) {
        case TOK_FUNCTION:
            break;
        case TOK_ROOT:
            fprintf(oilfile, "void __ocmain (void)\n");
            fprintf(oilfile, "{\n");
            for (size_t child = 0; child < node->children.size(); child++) {
                emitStatement(node->children[child]);
            }
            fprintf(oilfile, "}\n");
            break;
        case TOK_BLOCK:
            for (size_t child = 0; child < node->children.size(); child++) {
                emitStatement(node->children[child]);
            }
            break;
        case TOK_WHILE:
            emitWhile(node);
            break;
        case TOK_IFELSE:
            emitIfElse(node);
            break;
        case TOK_IF:
            emitIf(node);
            break;
        case TOK_RETURN:
        case TOK_RETURNVOID:
            emitReturn(node);
            break;
        case TOK_VARDECL:
            emitVardecl1(node);
            break;
        default:
            emitExpr(node);
    }
}

void emitExpr(astree* node) {
    for (size_t child = 0; child < node->children.size(); child++) {
        emitExpr(node->children[child]);
    }
    switch (node->symbol) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            emitBinArith(node);
            break;
        case '=':
            emitAssign(node);
            break;
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_LE:
        case TOK_GT:
        case TOK_GE:
            emitComp(node);
            break;
        case TOK_CHR:
            emitChar(node);
            break;
        case TOK_POS:
        case TOK_NEG:
            emitSign(node);
            break;
        case TOK_NEWSTRING:
            emitNewStr(node);
            break;
        case TOK_INDEX:
            emitIndex(node);
            break;

        case TOK_CALL:
            emitCall(node);
            break;
    }
}

void emitWhile(astree* node){
    fprintf(oilfile, "while_%lu_%lu_%lu:;\n", node->filenr, node->linenr, node->offset);
    emitExpr(node->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emitOp(node->children.at(0));
    fprintf(oilfile, ") goto break_%lu_%lu_%lu;\n", node->filenr, node->linenr, node->offset);
    emitStatement(node->children.at(1));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "goto while_%lu_%lu_%lu;\n", node->filenr, node->linenr, node->offset);
    fprintf(oilfile, "break_%lu_%lu_%lu:;\n", node->filenr, node->linenr, node->offset);
}

void emitIfElse(astree* node){
    emitExpr(node->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emitOp(node->children.at(0));
    fprintf(oilfile, ") goto else_%lu_%lu_%lu;\n", node->filenr, node->linenr, node->offset);
    emitStatement(node->children.at(1));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "goto fi_%lu_%lu_%lu;\n", node->filenr, node->linenr, node->offset);
    fprintf(oilfile, "else_%lu_%lu_%lu:;\n", node->filenr, node->linenr, node->offset);
    emitStatement(node->children.at(2));
    fprintf(oilfile, "fi_%lu_%lu_%lu:;\n", node->filenr, node->linenr, node->offset);
}

void emitIf(astree* node){
    emitExpr(node->children.at(0));
    fprintf(oilfile, "        ");
    fprintf(oilfile, "if (!");
    emitOp(node->children.at(0));
    fprintf(oilfile, ") goto fi_%lu_%lu_%lu;\n", node->filenr, node->linenr, node->offset);
    emitStatement(node->children.at(1));
    fprintf(oilfile, "fi_%lu_%lu_%lu:;\n", node->filenr, node->linenr, node->offset);
}

void emitReturn(astree* node){
    fprintf(oilfile, "        ");
    if (node->symbol == TOK_RETURN) {
        emitExpr(node->children.at(0));
        fprintf(oilfile, "return ");
        emitOp(node->children.at(0));
        fprintf(oilfile, ";\n");
    } else {
        fprintf(oilfile, "return;\n");
    }
}

void emitVardecl1(astree* node) {
    astree* ident = getID(node);
    astree* expr  = node->children.at(1);
    symbol* sym   = ident->symptr;
    if (expr->attributes.test(ATTR_vreg)  || expr->attributes.test(ATTR_vaddr)){
        emitExpr(expr);
    }
    fprintf(oilfile, "        ");
    if (sym->blocknr > 0) {
        fprintf(oilfile, "%s _%lu_%s = ", type(sym).c_str(), sym->blocknr, ident->lexinfo->c_str());
    } else {
        fprintf(oilfile, "__%s = ", ident->lexinfo->c_str());
    }
    emitOp(expr);
    fprintf(oilfile, ";\n");
}

astree* getID(astree* vardecl_node) {
    astree* type_node = vardecl_node->children.at(0);
    if (type_node->symbol == TOK_ARRAY) {
        return type_node->children.at(1);
    } else {
        return type_node->children.at(0);
    }
}

void emitAssign(astree* node) {
    astree* ident = node->children.at(0);
    astree* expr  = node->children.at(1);
    fprintf(oilfile, "        ");
    if (ident->attributes.test(ATTR_variable)) {
        symbol* sym   = ident->symptr;
        fprintf(oilfile, "_");
        if (sym->blocknr > 0){
            fprintf(oilfile, "%lu", sym->blocknr);
        }
        fprintf(oilfile, "_%s = ", ident->lexinfo->c_str());
    } else {
        emitOp(ident);
        fprintf(oilfile, " = ");
    }
    emitOp(expr);
    fprintf(oilfile, ";\n");
}

void emitIndex(astree* node) {
    astree* ident = node->children.at(0);
    astree* index = node->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "%s a%lu = ",  type(ident).c_str(), count);
    count++;
    if (ident->attributes.test(ATTR_vaddr) || (ident->attributes.test(ATTR_vreg) && ident->attributes.test(ATTR_array))) {
        fprintf(oilfile, "&a%lu", ident->regnr);
    } else {
        symbol* sym = ident->symptr;
        fprintf(oilfile, "&_");
        if (sym->blocknr > 0){
 	   		fprintf(oilfile, "%lu", sym->blocknr);
        }
        fprintf(oilfile, "_%s", ident->lexinfo->c_str());
    }
    fprintf(oilfile, "[");
    emitOp(index);
    fprintf(oilfile, "]");
    fprintf(oilfile, ";\n");
}

void emitCall(astree* node){
    astree* ident = node->children.at(0);
    fprintf(oilfile, "        ");
    if (!node->attributes.test(ATTR_void)) {
        fprintf(oilfile, "%s %c%lu = ",  type(ident).c_str(), regType(ident), count);
    
        count++;
    }
    fprintf(oilfile, "__%s(", ident->lexinfo->c_str());
    for (unsigned i = 1; i < node->children.size(); i++) {
        emitOp(node->children.at(i));
        if (i < node->children.size() - 1){
            fprintf(oilfile, ",");
        }
    }
    fprintf(oilfile, ");\n");
}

void emitBinArith(astree* node) {
    astree* op1 = node->children.at(0);
    astree* op2 = node->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "int i%lu = ", count); 
    count++;
    emitOp(op1);
    fprintf(oilfile, " %s ", node->lexinfo->c_str());
    emitOp(op2);
    fprintf(oilfile, ";\n");
}

void emitOp(astree* op) {
    if (op->attributes.test(ATTR_vreg) || (op->attributes.test(ATTR_string) && op->attributes.test(ATTR_const))) {
        fprintf(oilfile, "%c%lu",  regType(op), op->regnr);
    } else if (op->attributes.test(ATTR_vaddr)) {
        fprintf(oilfile, "*%c%lu",  regType(op), op->regnr);
    } else if (op->attributes.test(ATTR_variable)) {
        symbol* sym = op->symptr;
        fprintf(oilfile, "_");
        if (sym->blocknr > 0){
            fprintf(oilfile, "%lu", sym->blocknr);
        }
        fprintf(oilfile, "_%s", op->lexinfo->c_str());
    } else if (op->attributes.test(ATTR_const)) {
        string value = "";
        value.append(*(op->lexinfo));
        if (op->attributes.test(ATTR_int)) {
            while (value.at(0) == '0' && value.compare("0") != 0){
                value.erase(0, 1);
            }
        }
        fprintf(oilfile, "%s", value.c_str());
    }
}

void emitComp(astree* node){
    astree* op1 = node->children.at(0);
    astree* op2 = node->children.at(1);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char b%lu = ", count); 
    count++;
    emitOp(op1);
    fprintf(oilfile, " %s ", node->lexinfo->c_str());
    emitOp(op2);
    fprintf(oilfile, ";\n");
}

void emitChar(astree* node){
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char c%lu = (char) ", count); count++;
    emitOp(node->children.at(0));
    fprintf(oilfile, ";\n");
}
void emitSign(astree* node){
    fprintf(oilfile, "        ");
    if (node->symbol == TOK_POS){
        fprintf(oilfile, "int i%lu = +", count);
    }
    else {
        fprintf(oilfile, "int i%lu = -", count);
    } 
    count++;
    emitOp(node->children.at(0));
    fprintf(oilfile, ";\n");
}

void emitNewStr(astree* node){
    astree* size = node->children.at(0);
    fprintf(oilfile, "        ");
    fprintf(oilfile, "char* p%lu = xcalloc (", count); 
    count++;
    emitOp(size);
    fprintf(oilfile, ", sizeof (char));\n");
}

char regType(astree* node) {
    if (node->symbol == TOK_NEW || node->symbol == TOK_NEWARRAY || node->symbol == TOK_NEWSTRING) {
        return 'p';
    }
    if (node->attributes.test(ATTR_vaddr) || node->attributes.test(ATTR_array)) {
        return 'a';
    }
    if (node->attributes.test(ATTR_int)) {
        return 'i';
    } else if (node->attributes.test(ATTR_char)) {
        return 'c';
    } else if (node->attributes.test(ATTR_bool)) {
        return 'b';
    } else {
        return 's';
    }
}

void emitRun(astree* node) {
    count = 1;
    emitStructs();
    emitStringcon();
    emitVardecls();
    emitFunction();
    emitStatement(node);
}