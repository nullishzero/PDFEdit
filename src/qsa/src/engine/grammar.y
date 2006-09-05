%{
/****************************************************************************
** $Id$
**
** Copyright (C) 2001-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt Script for Applications framework (QSA).
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding a valid Qt Script for Applications license may use
** this file in accordance with the Qt Script for Applications License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about QSA Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
*****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include "qsengine.h"
#include "qsnodes.h"
#include "qslexer.h"

/* default values for bison */
#define YYDEBUG 0
#define YYMAXDEPTH 0
#ifdef QSDEBUGGER
#define YYERROR_VERBOSE
#define DBG(l, s, e) { l->setLoc(s.first_line, e.last_line); } // location
#else
#undef YYLSP_NEEDED
#define DBG(l, s, e)
#endif

extern int yylex();
static int yyerror (const char *);
static bool automatic();

%}

%union {
  int                 ival;
  double              dval;
  QString             *ustr;
  QSNode                *node;
  QSStatementNode       *stat;
  QSParameterNode       *param;
  QSFunctionBodyNode    *body;
  QSFuncDeclNode        *func;
  QSClassDefNode        *clss;
  QSProgramNode         *prog;
  QSAssignExprNode      *init;
  QSSourceElementNode   *src;
  QSSourceElementsNode  *srcs;
  QSStatListNode        *slist;
  QSArgumentsNode       *args;
  QSArgumentListNode    *alist;
  QSCaseBlockNode       *cblk;
  QSClauseListNode      *clist;
  QSCaseClauseNode      *ccl;
  QSElementNode         *elm;
  QSElisionNode         *eli;
  QSTypeNode            *type;
  QSTypedVarNode        *tvar;
  QSVarBindingNode      *vbin;
  QSVarBindingListNode  *blist;
  QSAttributeNode       *attrs;
  QSAttribute            attr;
  Operator            op;
}

%start Program

/* expect a shift/reduce conflict from the "dangling else" problem
   when using bison the warning can be supressed */
// %expect 1

/* literals */
%token NULLTOKEN TRUETOKEN FALSETOKEN
%token STRING NUMBER

/* keywords */
%token BREAK CASE DEFAULT FOR NEW VAR CONTINUE
%token FUNCTION RETURN QS_VOID QS_DELETE
%token IF THIS DO WHILE ELSE QS_IN INSTANCEOF TYPEOF IS
%token SWITCH WITH RESERVED
%token THROW TRY CATCH FINALLY
%token CLASS CONSTRUCTOR EXTENDS
%token ABSTRACT FINAL PRIVATE PUBLIC STATIC QS_CONST
%token PACKAGE IMPORT

/* punctuators */
%token EQEQ NE                     /* == and != */
%token STREQ STRNEQ                /* === and !== */
%token LE GE                       /* < and > */
%token OR AND                      /* || and && */
%token PLUSPLUS MINUSMINUS         /* ++ and --  */
%token LSHIFT                      /* << */
%token RSHIFT URSHIFT              /* >> and >>> */
%token PLUSEQUAL MINUSEQUAL        /* += and -= */
%token MULTEQUAL DIVEQUAL          /* *= and /= */
%token LSHIFTEQUAL                 /* <<= */
%token RSHIFTEQUAL URSHIFTEQUAL    /* >>= and >>>= */
%token ANDEQUAL MODEQUAL           /* &= and %= */
%token XOREQUAL OREQUAL            /* ^= and |= */

/* terminal types */
%token <dval> NUMBER
%token <ustr> STRING
%token <ustr> IDENT

/* non-terminal types */
%type <node>  Literal PrimaryExpr Expr MemberExpr FunctionExpr NewExpr CallExpr
%type <node>  ArrayLiteral PropertyName PropertyNameAndValueList
%type <node>  LeftHandSideExpr PostfixExpr UnaryExpr
%type <node>  MultiplicativeExpr AdditiveExpr
%type <node>  ShiftExpr RelationalExpr EqualityExpr
%type <node>  BitwiseANDExpr BitwiseXORExpr BitwiseORExpr
%type <node>  LogicalANDExpr LogicalORExpr
%type <node>  ConditionalExpr AssignmentExpr
%type <node>  ExprOpt
%type <node>  Catch Finally

%type <stat>  Statement Block
%type <stat>  VariableDefinition EmptyStatement ExprStatement
%type <stat>  IfStatement IterationStatement ContinueStatement
%type <stat>  BreakStatement ReturnStatement WithStatement
%type <stat>  SwitchStatement LabelledStatement
%type <stat>  ThrowStatement TryStatement
%type <stat>  PackageDefinition ImportStatement
%type <clss>  ClassDefinition

%type <slist> StatementList
%type <func>  FunctionDeclaration
%type <body>  FunctionBody
%type <src>   SourceElement
%type <srcs>  SourceElements
%type <param> FormalParameterList
%type <op>    AssignmentOperator
%type <prog>  Program
%type <args>  Arguments
%type <alist> ArgumentList
%type <cblk>  CaseBlock
%type <ccl>   CaseClause DefaultClause
%type <clist> CaseClauses  CaseClausesOpt
%type <eli>   Elision ElisionOpt
%type <elm>   ElementList
%type <ival>  VariableDefinitionKind
%type <attr>  Attribute
%type <attrs> Attributes
%type <type>  TypeExpression ResultSignature
%type <tvar>  TypedVariable
%type <vbin>  VariableBinding
%type <blist> VariableBindingList
%type <ustr>  PackageName

%%

Literal:
    NULLTOKEN                      { $$ = new QSNullNode(); }
  | TRUETOKEN                      { $$ = new QSBooleanNode(true); }
  | FALSETOKEN                     { $$ = new QSBooleanNode(false); }
  | NUMBER                         { $$ = new QSNumberNode($1); }
  | STRING                         { $$ = new QSStringNode($1); delete $1; }
  | '/'       /* a RegExp ? */     { QSLexer *l = QSLexer::lexer();
                                     if (!l->scanRegExp()) YYABORT;
                                     $$ = new QSRegExpNode(l->pattern,l->flags);}
;

PrimaryExpr:
    THIS                           { $$ = new QSThisNode(); }
  | IDENT                          { $$ = new QSResolveNode($1);
                                     delete $1; }
  | Literal
  | ArrayLiteral
  | '(' Expr ')'                   { $$ = new QSGroupNode($2); }
  | '{' '}'                        { $$ = new QSObjectLiteralNode(0L); }
  | '{' PropertyNameAndValueList '}'   { $$ = new QSObjectLiteralNode($2); }
;

ArrayLiteral:
    '[' ElisionOpt ']'                 { $$ = new QSArrayNode($2); }
  | '[' ElementList ']'                { $$ = new QSArrayNode($2); }
  | '[' ElementList ',' ElisionOpt ']' { $$ = new QSArrayNode($4, $2); }
;

ElementList:
    ElisionOpt AssignmentExpr      { $$ = new QSElementNode($1, $2); }
  | ElementList ',' ElisionOpt AssignmentExpr
                                   { $$ = new QSElementNode($1, $3, $4); }
;

ElisionOpt:
    /* nothing */                  { $$ = 0L; }
  | Elision
;

Elision:
    ','                            { $$ = new QSElisionNode(0L); }
  | Elision ','                    { $$ = new QSElisionNode($1); }
;

PropertyNameAndValueList:
    PropertyName ':' AssignmentExpr     { $$ = new QSPropertyValueNode($1, $3); }
  | PropertyNameAndValueList ',' PropertyName ':' AssignmentExpr
                                   { $$ = new QSPropertyValueNode($3, $5, $1); }
;

PropertyName:
    IDENT                          { $$ = new QSPropertyNode($1);
                                     delete $1; }
  | STRING                         { $$ = new QSPropertyNode($1); delete $1; }
  | NUMBER                         { $$ = new QSPropertyNode($1); }
;

MemberExpr:
    PrimaryExpr
  | FunctionExpr
  | MemberExpr '[' Expr ']'        { $$ = new QSAccessorNode1($1, $3); }
  | MemberExpr '.' IDENT           { $$ = new QSAccessorNode2($1, $3);
                                     delete $3; }
  | NEW MemberExpr Arguments       { $$ = new QSNewExprNode($2, $3); }
;

NewExpr:
    MemberExpr
  | NEW NewExpr                    { $$ = new QSNewExprNode($2); }
;

CallExpr:
    MemberExpr Arguments           { $$ = new QSFunctionCallNode($1, $2); }
  | CallExpr Arguments             { $$ = new QSFunctionCallNode($1, $2); }
  | CallExpr '[' Expr ']'          { $$ = new QSAccessorNode1($1, $3); }
  | CallExpr '.' IDENT             { $$ = new QSAccessorNode2($1, $3);
                                     delete $3; }
;

Arguments:
    '(' ')'                        { $$ = new QSArgumentsNode(0L); }
  | '(' ArgumentList ')'           { $$ = new QSArgumentsNode($2); }
;

ArgumentList:
    AssignmentExpr                  { $$ = new QSArgumentListNode($1); }
  | ArgumentList ',' AssignmentExpr { $$ = new QSArgumentListNode($1, $3); }
;

LeftHandSideExpr:
    NewExpr
  | CallExpr
;

PostfixExpr:    /* TODO: no line terminator here */
    LeftHandSideExpr
  | LeftHandSideExpr PLUSPLUS      { $$ = new QSPostfixNode($1, OpPlusPlus); }
  | LeftHandSideExpr MINUSMINUS    { $$ = new QSPostfixNode($1, OpMinusMinus); }
;

UnaryExpr:
    PostfixExpr
  | QS_DELETE UnaryExpr            { $$ = new QSDeleteNode($2); }
  | QS_VOID UnaryExpr              { $$ = new QSVoidNode($2); }
  | TYPEOF UnaryExpr               { $$ = new QSTypeOfNode($2); }
  | PLUSPLUS UnaryExpr             { $$ = new QSPrefixNode(OpPlusPlus, $2); }
  | MINUSMINUS UnaryExpr           { $$ = new QSPrefixNode(OpMinusMinus, $2); }
  | '+' UnaryExpr                  { $$ = new QSUnaryPlusNode($2); }
  | '-' UnaryExpr                  { $$ = new QSNegateNode($2); }
  | '~' UnaryExpr                  { $$ = new QSBitwiseNotNode($2); }
  | '!' UnaryExpr                  { $$ = new QSLogicalNotNode($2); }
;

MultiplicativeExpr:
    UnaryExpr
  | MultiplicativeExpr '*' UnaryExpr { $$ = new QSMultNode($1, $3, '*'); }
  | MultiplicativeExpr '/' UnaryExpr { $$ = new QSMultNode($1, $3, '/'); }
  | MultiplicativeExpr '%' UnaryExpr { $$ = new QSMultNode($1,$3,'%'); }
;

AdditiveExpr:
    MultiplicativeExpr
  | AdditiveExpr '+' MultiplicativeExpr { $$ = new QSAddNode($1, $3, '+'); }
  | AdditiveExpr '-' MultiplicativeExpr { $$ = new QSAddNode($1, $3, '-'); }
;

ShiftExpr:
    AdditiveExpr
  | ShiftExpr LSHIFT AdditiveExpr  { $$ = new QSShiftNode($1, OpLShift, $3); }
  | ShiftExpr RSHIFT AdditiveExpr  { $$ = new QSShiftNode($1, OpRShift, $3); }
  | ShiftExpr URSHIFT AdditiveExpr { $$ = new QSShiftNode($1, OpURShift, $3); }
;

RelationalExpr:
    ShiftExpr
  | RelationalExpr '<' ShiftExpr
                           { $$ = new QSRelationalNode($1, OpLess, $3); }
  | RelationalExpr '>' ShiftExpr
                           { $$ = new QSRelationalNode($1, OpGreater, $3); }
  | RelationalExpr LE ShiftExpr
                           { $$ = new QSRelationalNode($1, OpLessEq, $3); }
  | RelationalExpr GE ShiftExpr
                           { $$ = new QSRelationalNode($1, OpGreaterEq, $3); }
  | RelationalExpr IS ShiftExpr
                           { $$ = new QSRelationalNode($1, OpIs, $3); }
  | RelationalExpr INSTANCEOF ShiftExpr
                           { $$ = new QSRelationalNode($1, OpInstanceOf, $3); }
  | RelationalExpr QS_IN ShiftExpr
                           { $$ = new QSRelationalNode($1, OpIn, $3); }
;

EqualityExpr:
    RelationalExpr
  | EqualityExpr EQEQ RelationalExpr   { $$ = new QSEqualNode($1, OpEqEq, $3); }
  | EqualityExpr NE RelationalExpr     { $$ = new QSEqualNode($1, OpNotEq, $3); }
  | EqualityExpr STREQ RelationalExpr  { $$ = new QSEqualNode($1, OpStrEq, $3); }
  | EqualityExpr STRNEQ RelationalExpr { $$ = new QSEqualNode($1, OpStrNEq, $3);}
;

BitwiseANDExpr:
    EqualityExpr
  | BitwiseANDExpr '&' EqualityExpr { $$ = new QSBitOperNode($1, OpBitAnd, $3); }
;

BitwiseXORExpr:
    BitwiseANDExpr
  | BitwiseXORExpr '^' BitwiseANDExpr { $$ = new QSBitOperNode($1, OpBitXOr, $3); }
;

BitwiseORExpr:
    BitwiseXORExpr
  | BitwiseORExpr '|' BitwiseXORExpr { $$ = new QSBitOperNode($1, OpBitOr, $3); }
;

LogicalANDExpr:
    BitwiseORExpr
  | LogicalANDExpr AND BitwiseORExpr
                           { $$ = new QSBinaryLogicalNode($1, OpAnd, $3); }
;

LogicalORExpr:
    LogicalANDExpr
  | LogicalORExpr OR LogicalANDExpr
                           { $$ = new QSBinaryLogicalNode($1, OpOr, $3); }
;

ConditionalExpr:
    LogicalORExpr
  | LogicalORExpr '?' AssignmentExpr ':' AssignmentExpr
                           { $$ = new QSConditionalNode($1, $3, $5); }
;

AssignmentExpr:
    ConditionalExpr
  | LeftHandSideExpr AssignmentOperator AssignmentExpr
                           { $$ = new QSAssignNode($1, $2, $3);}
;

AssignmentOperator:
    '='                            { $$ = OpEqual; }
  | PLUSEQUAL                      { $$ = OpPlusEq; }
  | MINUSEQUAL                     { $$ = OpMinusEq; }
  | MULTEQUAL                      { $$ = OpMultEq; }
  | DIVEQUAL                       { $$ = OpDivEq; }
  | LSHIFTEQUAL                    { $$ = OpLShift; }
  | RSHIFTEQUAL                    { $$ = OpRShift; }
  | URSHIFTEQUAL                   { $$ = OpURShift; }
  | ANDEQUAL                       { $$ = OpAndEq; }
  | XOREQUAL                       { $$ = OpXOrEq; }
  | OREQUAL                        { $$ = OpOrEq; }
  | MODEQUAL                       { $$ = OpModEq; }
;

Expr:
    AssignmentExpr
  | Expr ',' AssignmentExpr        { $$ = new QSCommaNode($1, $3); }
;

Statement:
    Block
  | VariableDefinition
  | ClassDefinition                { $$ = $1; }
  | Attributes ClassDefinition     { $2->setAttributes( $1 ); $$ = $2; }
  | PackageDefinition
  | EmptyStatement
  | ExprStatement
  | IfStatement
  | IterationStatement
  | ContinueStatement
  | BreakStatement
  | ReturnStatement
  | WithStatement
  | SwitchStatement
  | LabelledStatement
  | ThrowStatement
  | TryStatement
  | ImportStatement
;

Block:
    '{' '}'                        { $$ = new QSBlockNode(0L); DBG($$, @2, @2); }
  | '{' StatementList '}'          { $$ = new QSBlockNode($2); DBG($$, @3, @3); }
;

StatementList:
    Statement                      { $$ = new QSStatListNode($1); }
  | StatementList Statement        { $$ = new QSStatListNode($1, $2); }
;

EmptyStatement:
    ';'                            { $$ = new QSEmptyStatementNode(); }
;

ExprStatement:
    Expr ';'                       { $$ = new QSExprStatementNode($1);
                                     DBG($$, @1, @2); }
  | Expr error                     { if (automatic()) {
                                       $$ = new QSExprStatementNode($1);
				       DBG($$, @1, @1);
                                     } else
				       YYABORT; }
;

IfStatement: /* shift/reduce conflict due to dangling else */
    IF '(' Expr ')' Statement      { $$ = new QSIfNode($3,$5,0L);DBG($$,@1,@4); }
  | IF '(' Expr ')' Statement ELSE Statement
                                   { $$ = new QSIfNode($3,$5,$7);DBG($$,@1,@4); }
;

IterationStatement:
    DO Statement WHILE '(' Expr ')' { $$=new QSDoWhileNode($2,$5);DBG($$,@1,@3);}
  | WHILE '(' Expr ')' Statement   { $$ = new QSWhileNode($3,$5);DBG($$,@1,@4); }
  | FOR '(' ExprOpt ';' ExprOpt ';' ExprOpt ')'
            Statement              { $$ = new QSForNode($3,$5,$7,$9);
	                             DBG($$,@1,@8); }
  | FOR '(' VAR VariableBindingList ';' ExprOpt ';' ExprOpt ')'
            Statement              { $$ = new QSForNode($4,$6,$8,$10);
	                             DBG($$,@1,@9); }
  | FOR '(' LeftHandSideExpr QS_IN Expr ')'
            Statement              { $$ = new QSForInNode($3, $5, $7);
	                             DBG($$,@1,@6); }
  | FOR '(' VAR VariableBinding QS_IN Expr ')'
            Statement              { $$ = new QSForInNode($4,$6,$8);
	                             DBG($$,@1,@7); }
;

ExprOpt:
    /* nothing */                  { $$ = 0L; }
  | Expr
;

ContinueStatement:
    CONTINUE ';'                   { $$ = new QSContinueNode(); DBG($$,@1,@2); }
  | CONTINUE error                 { if (automatic()) {
                                       $$ = new QSContinueNode(); DBG($$,@1,@2);
                                     } else
				       YYABORT; }
  | CONTINUE IDENT ';'             { $$ = new QSContinueNode($2); DBG($$,@1,@3);
                                     delete $2; }
  | CONTINUE IDENT error           { if (automatic()) {
                                       $$ = new QSContinueNode($2);DBG($$,@1,@2);
				       delete $2;
                                     } else
				       YYABORT; }
;

BreakStatement:
    BREAK ';'                      { $$ = new QSBreakNode();DBG($$,@1,@2); }
  | BREAK error                    { if (automatic()) {
                                       $$ = new QSBreakNode(); DBG($$,@1,@1);
                                     } else
				       YYABORT; }
  | BREAK IDENT ';'                { $$ = new QSBreakNode($2); DBG($$,@1,@3);
                                     delete $2; }
  | BREAK IDENT error              { if (automatic()) {
                                       $$ = new QSBreakNode($2); DBG($$,@1,@2);
				       delete $2;
                                     } else
				       YYABORT;
                                   }
;

ReturnStatement:
    RETURN ';'                     { $$ = new QSReturnNode(0L); DBG($$,@1,@2); }
  | RETURN error                   { if (automatic()) {
                                       $$ = new QSReturnNode(0L); DBG($$,@1,@1);
                                     } else
				       YYABORT; }
  | RETURN Expr ';'                { $$ = new QSReturnNode($2); DBG($$,@1,@2); }
  | RETURN Expr error              { if (automatic()) {
                                       $$ = new QSReturnNode($2);
                                       DBG($$,@1,@2);
                                     } else {
				       YYABORT;
				     } 
                                   }
;

WithStatement:
    WITH '(' Expr ')' Statement    { $$ = new QSWithNode($3,$5);
                                     DBG($$, @1, @4); }
;

SwitchStatement:
    SWITCH '(' Expr ')' CaseBlock  { $$ = new QSSwitchNode($3, $5);
                                     DBG($$, @1, @4); }
;

CaseBlock:
    '{' CaseClausesOpt '}'         { $$ = new QSCaseBlockNode($2, 0L, 0L); }
  | '{' CaseClausesOpt DefaultClause CaseClausesOpt '}'
                                   { $$ = new QSCaseBlockNode($2, $3, $4); }
;

CaseClausesOpt:
    /* nothing */                  { $$ = 0L; }
  | CaseClauses
;

CaseClauses:
    CaseClause                     { $$ = new QSClauseListNode($1); }
  | CaseClauses CaseClause         { $$ = $1->append($2); }
;

CaseClause:
    CASE Expr ':'                  { $$ = new QSCaseClauseNode($2, 0L); }
  | CASE Expr ':' StatementList    { $$ = new QSCaseClauseNode($2, $4); }
;

DefaultClause:
    DEFAULT ':'                    { $$ = new QSCaseClauseNode(0L, 0L);; }
  | DEFAULT ':' StatementList      { $$ = new QSCaseClauseNode(0L, $3); }
;

LabelledStatement:
    IDENT ':' Statement            { $3->pushLabel($1);
                                     $$ = new QSLabelNode($1, $3);
                                     delete $1; }
;

ThrowStatement:
    THROW Expr ';'                 { $$ = new QSThrowNode($2); }
;

TryStatement:
    TRY Block Catch                { $$ = new QSTryNode($2, $3); }
  | TRY Block Finally              { $$ = new QSTryNode($2, 0L, $3); }
  | TRY Block Catch Finally        { $$ = new QSTryNode($2, $3, $4); }
;

Catch:
    CATCH '(' IDENT ')' Block      { $$ = new QSCatchNode($3, $5); delete $3; }
;

Finally:
    FINALLY Block                  { $$ = new QSFinallyNode($2); }
;

PackageDefinition:
    PACKAGE PackageName FunctionBody
                                   { $$ = new QSPackageNode($2, $3);
                                     delete $2; }
;

ImportStatement:
    IMPORT PackageName ';'         { $$ = new QSImportNode($2); delete $2; }
;

PackageName:
      IDENT                        { $$ = $1; delete $1; }
    | PackageName '.' IDENT        { *$$ += QString::fromLatin1(".") + *$3; delete $3; }
;

FunctionDeclaration:
    FUNCTION IDENT '(' ')' ResultSignature FunctionBody
                        { $$ = new QSFuncDeclNode($2, 0L, $5, $6); delete $2; }
  | FUNCTION IDENT '(' FormalParameterList ')' ResultSignature FunctionBody
                        { $$ = new QSFuncDeclNode($2, $4, $6, $7); delete $2; }
  | Attributes FUNCTION IDENT '(' ')'  ResultSignature FunctionBody
                        { $$ = new QSFuncDeclNode($3, 0L, $6, $7);
			  $$->setAttributes( $1 ); delete $3; }
  | Attributes FUNCTION IDENT '(' FormalParameterList ')' ResultSignature
    FunctionBody        { $$ = new QSFuncDeclNode($3, $5, $7, $8);
                          $$->setAttributes( $1 ); delete $3; }
;

ResultSignature:
                                   { $$ = 0L; }
  | ':' TypeExpression             { $$ = $2; }
;

FunctionExpr:
    FUNCTION '(' ')' FunctionBody  { $$ = new QSFuncExprNode(0L, $4); }
  | FUNCTION '(' FormalParameterList ')' FunctionBody
                                   { $$ = new QSFuncExprNode($3, $5); }

;

FormalParameterList:
    IDENT                          { $$ = new QSParameterNode($1, 0L);
                                     delete $1; }
  | IDENT ':' TypeExpression       { $$ = new QSParameterNode($1, $3);
                                     delete $1; }
  | FormalParameterList ',' IDENT  { $$ = $1->append($3, 0L); delete $3; }
  | FormalParameterList ',' IDENT ':' TypeExpression
                                   { $$ = $1->append($3, $5); delete $3; }
;

FunctionBody:
    '{' '}'  /* TODO: spec ??? */  { $$ = new QSFunctionBodyNode(0L); }
  | '{' SourceElements '}'         { $$ = new QSFunctionBodyNode($2); }
;

ClassDefinition:
    CLASS IDENT ';'                { $$ = new QSClassDefNode($2, 0L, 0L);
                                     delete $2; }
  | CLASS IDENT FunctionBody       { $$ = new QSClassDefNode($2, 0L, $3);
                                     delete $2; }
  | CLASS IDENT EXTENDS TypeExpression FunctionBody
                                   { $$ = new QSClassDefNode($2, $4, $5);
                                     delete $2; }
;

Program:
    /* empty */                    { $$ = new QSProgramNode(0L); }
  | SourceElements                 { $$ = new QSProgramNode($1); }
;

SourceElements:
    SourceElement                  { $$ = new QSSourceElementsNode($1); }
  | SourceElements SourceElement   { $$ = new QSSourceElementsNode($1, $2); }
;

SourceElement:
    Statement                      { $$ = new QSSourceElementNode($1); }
  | FunctionDeclaration            { $$ = new QSSourceElementNode($1); }
;

Attributes:
    Attribute                      { $$ = new QSAttributeNode($1); }
  | Attributes Attribute           { $1->add($2); $$ = $1; }
;

Attribute:
    ABSTRACT                       { $$ = AttributeAbstract; }
  | FINAL                          { $$ = AttributeFinal; }
  | PRIVATE                        { $$ = AttributePrivate; }
  | PUBLIC                         { $$ = AttributePublic; }
  | STATIC                         { $$ = AttributeStatic; }
  | TRUETOKEN                      { $$ = AttributeTrue; }
  | FALSETOKEN                     { $$ = AttributeFalse; }
  | CONSTRUCTOR                    { $$ = AttributeConstructor; }
;

/************************* Variable Definition ******************************/

VariableDefinition:
    VariableDefinitionKind VariableBindingList
                             { $$ = new QSVarDefNode($1, $2); DBG($$, @1, @2); }
  | Attributes VariableDefinitionKind VariableBindingList
                             { $$ = new QSVarDefNode($2, $3); DBG($$, @1, @3);
                               ((QSVarDefNode*)$$)->setAttributes( $1 ); }
;

VariableDefinitionKind:
    VAR 					{ $$ = 0; }
  | QS_CONST 					{ $$ = 1; }
;

VariableBindingList:
    VariableBinding                { $$ = new QSVarBindingListNode(0L, $1); }
  | VariableBindingList ',' VariableBinding
                                   { $$ = new QSVarBindingListNode($1, $3); }
;

VariableBinding:
    TypedVariable                      { $$ = new QSVarBindingNode($1, 0L); }
  | TypedVariable '=' AssignmentExpr   { $$ = new QSVarBindingNode($1, $3); }
;

TypedVariable:
    IDENT                          { $$ = new QSTypedVarNode($1, 0L);
                                     delete $1; }
  | IDENT ':' TypeExpression       { $$ = new QSTypedVarNode($1, $3);
                                     delete $1; }
;

/************************* Type Expressions *********************************/

TypeExpression:
    IDENT                          { $$ = new QSTypeNode($1); delete $1; }
;

%%

int yyerror ( const char *errstr )  /* Called by yyparse on error */
{
  QSLexer::lexer()->setErrorMessage( errstr );
  return 1;
}

/* may we automatically insert a semicolon ? */
bool automatic()
{
  if ( yychar == '}' || yychar == 0 )
    return true;
  else if ( QSLexer::lexer()->prevTerminator() )
    return true;

  return false;
}
