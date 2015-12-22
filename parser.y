%{
#include "lyutils.h"
#include "astree.h"
#include "stringset.h"
%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_DECLID
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD
%token TOK_ORD TOK_CHR TOK_ROOT

%token TOK_INDEX TOK_NEWSTRING TOK_RETURNVOID TOK_FUNCTION
%token TOK_PARAMLIST TOK_VARDECL TOK_PROTOTYPE

%start program

%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left  '[' '.' '('

%%

program     : program structdef       { $$ = adopt1($1, $2); }
            | program function        { $$ = adopt1($1, $2); }
            | program statement       { $$ = adopt1($1, $2); }
            | program error '}'       { $$ = $1; free_ast($3); }
            | program error ']'       { $$ = $1; free_ast($3); }
            |                         { $$ = new_parseroot(); }
            ;
structdef   : field '}'          { $$ = $1; free_ast($2); }
            ;

field       : field fielddecl ';'    { $$ = adopt1($1, $2); free_ast($3); }
            | TOK_STRUCT TOK_IDENT '{'    { $$ = adopt1($1, $2); free_ast($3); ast_rep($2, TOK_TYPEID); }
            ;

fielddecl   : basetype TOK_ARRAY TOK_IDENT    { $$ = adopt2($2, $1, $3); ast_rep($3, TOK_FIELD); }
            | basetype TOK_IDENT      { $$ = adopt1($1, $2); ast_rep($2, TOK_FIELD); }
            ;

basetype    : TOK_VOID                { $$ = $1; }
            | TOK_BOOL                { $$ = $1; } 
            | TOK_CHAR                { $$ = $1; }
            | TOK_INT                 { $$ = $1; }
            | TOK_STRING              { $$ = $1; }
            | TOK_IDENT               { $$ = $1; ast_rep($1, TOK_TYPEID); }
            ;

function    : identdecl '(' ')' ';'       { $$ = new_proto($1, $2); ast_rep($2, TOK_PARAMLIST); free_ast2($3, $4); }
            | identdecl '(' ')' block     { $$ = new_func($1, $2, $4); ast_rep($2, TOK_PARAMLIST); free_ast($3);  }
            | identdecl decllist ')' ';'    { $$ = new_proto($1, $2); free_ast2($3, $4); }
            | identdecl decllist ')' block  { $$ = new_func($1, $2, $4);  free_ast($3); }
            ;

decllist      : decllist ',' identdecl    { $$ = adopt1($1, $3); free_ast($2); }
            | '(' identdecl               { $$ = ast_swap($1, $2, TOK_PARAMLIST); }
            ;

identdecl   : basetype TOK_ARRAY TOK_IDENT  { $$ = adopt2($2, $1, $3); ast_rep($3, TOK_DECLID); }
            | basetype TOK_IDENT            { $$ = adopt1($1, $2); ast_rep($2, TOK_DECLID); }
            ;

block       : stmtlist '}'           { $$ = $1; free_ast($2); }
            ;

stmtlist    : stmtlist statement     { $$ = adopt1($1, $2); }
            | '{'                     { $$ = $1; ast_rep($1, TOK_BLOCK); }
            ;

statement   : block                   { $$ = $1; }
            | vardecl                 { $$ = $1; }
            | while                   { $$ = $1; }
            | ifelse                  { $$ = $1; }
            | return                  { $$ = $1; }
            | expr ';'                { $$ = $1; free_ast($2); }
            | ';'                     { $$ = $1; }
            ;

vardecl     : identdecl '=' expr ';'  { $$ = ast_swap2($2, $1, $3, TOK_VARDECL); free_ast($4); }
            ;

while       : TOK_WHILE '(' expr ')' statement        { $$ = adopt2($1, $3, $5); free_ast2($2, $4); }
            ;

ifelse      : TOK_IF '(' expr ')' statement TOK_ELSE statement  { $$ = ast_swap3($1, $3, $5, $7, TOK_IFELSE); free_ast2($2, $4); free_ast($6); }
            | TOK_IF '(' expr ')' statement %prec TOK_IF  { $$ = adopt2($1, $3, $5); free_ast2($2, $4); }
            ;

return      : TOK_RETURN ';'          { $$ = $1; free_ast($2); ast_rep($1, TOK_RETURNVOID); }
            | TOK_RETURN expr ';'     { $$ = adopt1($1, $2); free_ast($3); }
            ;

expr        : binop                   { $$ = $1; }
            | unop                    { $$ = $1; }
            | allocator               { $$ = $1; }
            | call                    { $$ = $1; }
            | '(' expr ')'            { $$ = $2; free_ast2($1, $3); }
            | variable                { $$ = $1; }
            | constant                { $$ = $1; }
            ;

binop       : expr '+' expr           { $$ = adopt2($2, $1, $3); }
            | expr '-' expr           { $$ = adopt2($2, $1, $3); }
            | expr '*' expr           { $$ = adopt2($2, $1, $3); }
            | expr '/' expr           { $$ = adopt2($2, $1, $3); }
            | expr '%' expr           { $$ = adopt2($2, $1, $3); }
            | expr '=' expr           { $$ = adopt2($2, $1, $3); }
            | expr TOK_EQ expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_NE expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_LT expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_LE expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_GT expr        { $$ = adopt2($2, $1, $3); }
            | expr TOK_GE expr        { $$ = adopt2($2, $1, $3); }
            ;

unop        : '+' expr                { $$ = ast_swap($1, $2, TOK_POS); }
            | '-' expr                { $$ = ast_swap($1, $2, TOK_NEG); }
            | '!' expr                { $$ = adopt1($1, $2); }
            | TOK_ORD expr            { $$ = adopt1($1, $2); }
            | TOK_CHR expr            { $$ = adopt1($1, $2); }
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')'          { $$ = adopt1($1, $2); free_ast2($3, $4); ast_rep($2, TOK_TYPEID); }
            | TOK_NEW TOK_STRING  '(' expr ')'   { $$ = ast_swap($1, $4, TOK_NEWSTRING); free_ast2($2, $3); free_ast($5); }
            | TOK_NEW basetype '[' expr ']'      { $$ = ast_swap2($1, $2, $4, TOK_NEWARRAY); free_ast2($3, $5); }
            ;

call        : TOK_IDENT '(' ')'       { $$ = ast_swap($2, $1, TOK_CALL); free_ast($3); }
            | exprlist ')'                { $$ = $1; free_ast($2); }
            ;

exprlist    : TOK_IDENT '(' expr      { $$ = ast_swap2($2, $1, $3, TOK_CALL); }
            | exprlist ',' expr           { $$ = adopt1($1, $3); free_ast($2); }
            ;

variable    : TOK_IDENT               { $$ = $1; }
            | expr '[' expr ']'       { $$ = ast_swap2($2, $1, $3, TOK_INDEX); free_ast($4); }
            | expr '.' TOK_IDENT      { $$ = adopt2($2, $1, $3); ast_rep($3, TOK_FIELD); }
            ;

constant    : TOK_INTCON              { $$ = $1; }
            | TOK_CHARCON             { $$ = $1; }
            | TOK_STRINGCON           { $$ = $1; } 
            | TOK_FALSE               { $$ = $1; }
            | TOK_TRUE                { $$ = $1; }
            | TOK_NULL                { $$ = $1; } 
            ;

%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}