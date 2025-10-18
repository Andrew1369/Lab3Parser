/* A Bison parser, made by GNU Bison 3.7.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_D_STUDY_UNIVERSITY_SYSTEM_PROGRAMMING_LAB3PARSER_LAB3PARSER_GENERATED_PARSER_HPP_INCLUDED
# define YY_YY_D_STUDY_UNIVERSITY_SYSTEM_PROGRAMMING_LAB3PARSER_LAB3PARSER_GENERATED_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 18 "D:\\Study\\University\\System_programming\\Lab3Parser\\Lab3Parser\\src\\parser.y"

  #include <string>
  #include "../include/ast.hpp" /* а це Ц дл€ parser.hpp */

#line 54 "D:\\Study\\University\\System_programming\\Lab3Parser\\Lab3Parser\\generated\\parser.hpp"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    KW_INT = 258,                  /* KW_INT  */
    KW_DOUBLE = 259,               /* KW_DOUBLE  */
    KW_IF = 260,                   /* KW_IF  */
    KW_ELSE = 261,                 /* KW_ELSE  */
    KW_WHILE = 262,                /* KW_WHILE  */
    KW_PRINT = 263,                /* KW_PRINT  */
    IDENT = 264,                   /* IDENT  */
    NUMBER = 265,                  /* NUMBER  */
    EQ = 266,                      /* EQ  */
    NE = 267,                      /* NE  */
    LE = 268,                      /* LE  */
    GE = 269,                      /* GE  */
    AND = 270,                     /* AND  */
    OR = 271,                      /* OR  */
    UMINUS = 272,                  /* UMINUS  */
    LOWER_THAN_ELSE = 273          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 24 "D:\\Study\\University\\System_programming\\Lab3Parser\\Lab3Parser\\src\\parser.y"

    double num;
    std::string* str;
    AST::Expr* expr;
    AST::Stmt* stmt;
    AST::Block* block;
    int token;

#line 98 "D:\\Study\\University\\System_programming\\Lab3Parser\\Lab3Parser\\generated\\parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_D_STUDY_UNIVERSITY_SYSTEM_PROGRAMMING_LAB3PARSER_LAB3PARSER_GENERATED_PARSER_HPP_INCLUDED  */
