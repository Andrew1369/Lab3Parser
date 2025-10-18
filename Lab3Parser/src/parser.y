/* src/parser.y */
%{
#include <cstdio>
#include <cstdlib>
#include <string>
#include <memory>
#include "../include/ast.hpp"

// Глобальний корінь AST (Block)
AST::Block* g_root = nullptr;

int yylex(void);
void yyerror(const char* s);
%}

%define api.pure false

%code requires {
  #include <string>
  #include "../include/ast.hpp" /* а це – для parser.hpp */
}

/* Типи семантичних значень */
%union {
    double num;
    std::string* str;
    AST::Expr* expr;
    AST::Stmt* stmt;
    AST::Block* block;
    int token;
}

/* Токени */
%token KW_INT KW_DOUBLE KW_IF KW_ELSE KW_WHILE KW_PRINT
%token <str> IDENT
%token <num> NUMBER
%token EQ NE LE GE AND OR

/* Пріоритети (для виразів + "висячого else") */
%right '='
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS '!'
%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

/* Типи нетерміналів */
%type <block> program stmts
%type <stmt> stmt vardecl assign print if while block
%type <expr> expr

%start program

%%

program
    : stmts                  { g_root = $1; }
    ;

stmts
    : /* empty */            { $$ = new AST::Block(); }
    | stmts stmt             { $1->add($2); $$ = $1; }
    ;

stmt
    : vardecl ';'            { $$ = $1; }
    | assign  ';'            { $$ = $1; }
    | print   ';'            { $$ = $1; }
    | if                      { $$ = $1; }
    | while                   { $$ = $1; }
    | block                   { $$ = $1; }
    ;

block
    : '{' stmts '}'          { $$ = $2; }
    ;

/* Оголошення змінних */
vardecl
    : KW_INT IDENT           { $$ = new AST::VarDecl(AST::Type::Int, *$2); delete $2; }
    | KW_DOUBLE IDENT        { $$ = new AST::VarDecl(AST::Type::Double, *$2); delete $2; }
    | KW_INT IDENT '=' expr  { $$ = new AST::VarDecl(AST::Type::Int, *$2, $4); delete $2; }
    | KW_DOUBLE IDENT '=' expr { $$ = new AST::VarDecl(AST::Type::Double, *$2, $4); delete $2; }
    ;

/* Присвоєння */
assign
    : IDENT '=' expr         { $$ = new AST::Assign(*$1, $3); delete $1; }
    ;

/* Друк */
print
    : KW_PRINT '(' expr ')'  { $$ = new AST::Print($3); }
    ;

/* if/else з вирішенням "висячого else" */
if
    : KW_IF '(' expr ')' stmt %prec LOWER_THAN_ELSE
        { $$ = new AST::If($3, $5, nullptr); }
    | KW_IF '(' expr ')' stmt KW_ELSE stmt
        { $$ = new AST::If($3, $5, $7); }
    ;

while
    : KW_WHILE '(' expr ')' stmt
        { $$ = new AST::While($3, $5); }
    ;

/* Вирази */
expr
    : expr '+' expr          { $$ = new AST::Binary(AST::BinOp::Add, $1, $3); }
    | expr '-' expr          { $$ = new AST::Binary(AST::BinOp::Sub, $1, $3); }
    | expr '*' expr          { $$ = new AST::Binary(AST::BinOp::Mul, $1, $3); }
    | expr '/' expr          { $$ = new AST::Binary(AST::BinOp::Div, $1, $3); }
    | expr '%' expr          { $$ = new AST::Binary(AST::BinOp::Mod, $1, $3); }
    | expr '<' expr          { $$ = new AST::Binary(AST::BinOp::LT,  $1, $3); }
    | expr LE  expr          { $$ = new AST::Binary(AST::BinOp::LE,  $1, $3); }
    | expr '>' expr          { $$ = new AST::Binary(AST::BinOp::GT,  $1, $3); }
    | expr GE  expr          { $$ = new AST::Binary(AST::BinOp::GE,  $1, $3); }
    | expr EQ  expr          { $$ = new AST::Binary(AST::BinOp::EQ,  $1, $3); }
    | expr NE  expr          { $$ = new AST::Binary(AST::BinOp::NE,  $1, $3); }
    | expr AND expr          { $$ = new AST::Binary(AST::BinOp::And, $1, $3); }
    | expr OR  expr          { $$ = new AST::Binary(AST::BinOp::Or,  $1, $3); }
    | '-' expr %prec UMINUS  { $$ = new AST::Unary(AST::UnOp::Neg, $2); }
    | '!' expr               { $$ = new AST::Unary(AST::UnOp::Not, $2); }
    | '(' expr ')'           { $$ = $2; }
    | NUMBER                 { $$ = new AST::Number($1); }
    | IDENT                  { $$ = new AST::Ident(*$1); delete $1; }
    ;

%%

void yyerror(const char* s) {
    std::fprintf(stderr, "Parse error: %s\n", s);
}
