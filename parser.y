%{
// Dummy parser for scanner project.

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

%token TOK_VOID TOK_BOOL TOK_CHAR TOK_INT TOK_STRING TOK_RETURNVOID
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_FALSE TOK_TRUE TOK_NULL TOK_NEW TOK_ARRAY
%token TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON

%token TOK_BLOCK TOK_CALL TOK_IFELSE TOK_DECLID
%token TOK_POS TOK_NEG TOK_NEWARRAY TOK_TYPEID TOK_FIELD TOK_NEWSTRING
%token TOK_ORD TOK_CHR TOK_ROOT
 
%token TOK_BINOP TOK_UNOP TOK_CONSTANT TOK_BASETYPE TOK_BLOCK TOK_CONSTANT

%right TOK_IF TOK_ELSE
%right '='
%left  TOK_EQ TOK_NE TOK_LT TOK_LE TOK_GT TOK_GE
%left  '+' '-'
%left  '*' '/' '%'
%right TOK_POS TOK_NEG '!' TOK_NEW TOK_ORD TOK_CHR
%left  '[' '.' '('

%start start

%%

start		: program										{yyparse_astree = $1;}
			;

program		: program structdef								{$$ = adopt1($1, $2);}
			| program function								{$$ = adopt1($1, $2);}
			| program statement								{$$ = adopt1($1, $2);}
			| program error '}'								{$$ = $1;}
			| program error ';'								{$$ = $1;}
			|												{$$ = new_parseroot();}
			;


structdef   : TOK_STRUCT TOK_IDENT '{' '}'                  {$$ = adopt1($1, $2); ast_swap($2, TOK_TYPEID); free_ast2($3, $4);} 
            | TOK_STRUCT TOK_IDENT '{' field '}'            {$$ = adopt2($1, $2, $4); ast_swap($2, TOK_TYPEID); free_ast2($3, $5);}
            ;

field       : field fielddecl                               {$$ = adopt1($1, $2);}
            | fielddecl ';'                                 {$$ = new astree(TOK_ROOT, 0, 0, 0, ""); free_ast($1);} 
            ;

fielddecl   : basetype TOK_IDENT                            {$$ = adopt1($1, $2); ast_swap($2, TOK_FIELD);}
            | basetype TOK_ARRAY TOK_IDENT                  {$$ = adopt2($1, $2, $3); ast_swap($3, TOK_FIELD); free_ast($2);}
            ;

basetype    : TOK_VOID                                      {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_VOID);}
            | TOK_BOOL                                      {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_BOOL);}
            | TOK_CHR                                       {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_CHR);}
            | TOK_INT                                       {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_INT);}
            | TOK_STRING                                    {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_STRING);}
            | TOK_IDENT                                     {$$ = new astree(TOK_BASETYPE, 0, 0, 0, ""); ast_swap($1, TOK_TYPEID);}
            ;

function    : identdecl '(' ')' block                       {$$ = adopt1($1, $3); free_ast2($2, $3);}
            | identdecl '(' identdecl ')' block             {free_ast2($2, $4);}
            | identdecl '(' identdecl decllist ')' block    {free_ast2($2, $5);}
            ;

decllist    : decllist identdecl                            {$$ = adopt1($1, $2);}
            | identdecl                                     {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            ;

identdecl   : basetype TOK_IDENT                            {$$ = adopt1($1, $2); ast_swap($2, TOK_DECLID);}
            | basetype TOK_ARRAY TOK_IDENT                  {$$ = adopt2($1, $2, $3); ast_swap($2, TOK_DECLID);}
            ;



block       : '{' '}'                                       {free_ast2($1, $2);}
            | '{' stmtlist '}'                              {$$ = new astree(TOK_BLOCK, 0, 0, 0, ""); free_ast2($1, $3);}
            | ';'                                           {free_ast($1);}
            ;

stmtlist    : stmtlist statement                            {$$ = adopt1($1, $2);}
            | statement                                     {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            ;

statement   : block                                         {$$ = $1;}
            | vardecl                                       {$$ = $1;}
            | while                                         {$$ = $1;}
            | ifelse                                        {$$ = $1;}
            | return                                        {$$ = $1;}
            | expr ';'                                      {$$ = $1;}
            ;

vardecl     : identdecl '=' expr ';'                        {$$ = adopt2($2, $1, $3); ast_swap($3, TOK_DECLID); free_ast($4);}
            ;

while       : TOK_WHILE '(' expr ')' statement              {$$ = adopt2($1, $2, $3); free_ast2($2, $4);}
            ;

ifelse      : TOK_IF '(' expr ')' statement                 {$$ = adopt1($3, $5); free_ast2($2, $4);}
            | TOK_IF '(' expr ')' statement TOK_ELSE statement  {$$ = adopt1(adopt1($3, $5), $7); ast_swap($1, TOK_IFELSE); free_ast2($2, $4); free_ast($6);}
            ;

return      : TOK_RETURN ';'                                {$$ = new astree(TOK_ROOT, 0, 0, 0, ""); ast_swap($1, TOK_RETURNVOID); free_ast($2);}
            | TOK_RETURN expr ';'                           {$$ = adopt1($1, $2); free_ast($3);}
            ;

expr        : expr binop expr                               {$$ = adopt2($2, $1, $3);}
            | unop expr                                     {$$ = adopt1($1, $2);}
            | allocator                                     {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            | call                                          {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            | '(' expr ')'                                  {$$ = new astree(TOK_ROOT, 0, 0, 0, ""); free_ast2($1, $3);}
            | variable                                      {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            | constant                                      {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            ;

binop       : '+'                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | '-'                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | '/'                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | '*'                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | '='                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | '%'                                           {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_EQ                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_GE                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_GT                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_LE                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_LT                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            | TOK_NE                                        {$$ = new astree(TOK_BINOP, 0, 0, 0, "");}
            ;

unop        : '+'                                           {$$ = new astree(TOK_UNOP, 0, 0, 0, ""); ast_swap($1, TOK_POS);}
            | '-'                                           {$$ = new astree(TOK_UNOP, 0, 0, 0, ""); ast_swap($1, TOK_NEG);}
            ;

allocator   : TOK_NEW TOK_IDENT '(' ')'                     {$$ = adopt1($1, $2); ast_swap($2, TOK_TYPEID); free_ast2($3, $4);}
            | TOK_NEW TOK_STRING '(' expr ')'               {ast_swap($1, TOK_NEWSTRING); free_ast2($3, $5);}
            | TOK_NEW basetype '[' expr ']'                 {ast_swap($1, TOK_NEWSTRING); free_ast2($3, $5);}
            ;

call        : TOK_IDENT '(' ')'                             {free_ast2($2, $3);}
            | TOK_IDENT '(' expr ')'                        {free_ast2($2, $4);}
            | TOK_IDENT '(' expr exprlist ')'               {free_ast2($2, $5);}
            ;

exprlist    : exprlist expr                                 {$$ = adopt1($1, $2);}
            | expr                                          {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            ;

variable    : TOK_IDENT                                     {$$ = new astree(TOK_ROOT, 0, 0, 0, "");}
            | expr '[' ']'                                  {$$ = new astree(TOK_ROOT, 0, 0, 0, ""); free_ast2($2, $3);}
            | expr '[' expr ']'                             {$$ = adopt1($1, $3); free_ast2($2, $4);}
            | expr '.' TOK_IDENT                            {$$ = adopt1($1, $3);}
            ;

constant    : TOK_INTCON                                    {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            | TOK_CHARCON                                   {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            | TOK_STRINGCON                                 {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            | TOK_TRUE                                      {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            | TOK_FALSE                                     {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            | TOK_NULL                                      {$$ = new astree(TOK_CONSTANT, 0, 0, 0, "");}
            ;



%%

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}


bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}


