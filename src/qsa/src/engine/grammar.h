typedef union {
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
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#define	NULLTOKEN	257
#define	TRUETOKEN	258
#define	FALSETOKEN	259
#define	STRING	260
#define	NUMBER	261
#define	BREAK	262
#define	CASE	263
#define	DEFAULT	264
#define	FOR	265
#define	NEW	266
#define	VAR	267
#define	CONTINUE	268
#define	FUNCTION	269
#define	RETURN	270
#define	QS_VOID	271
#define	QS_DELETE	272
#define	IF	273
#define	THIS	274
#define	DO	275
#define	WHILE	276
#define	ELSE	277
#define	QS_IN	278
#define	INSTANCEOF	279
#define	TYPEOF	280
#define	IS	281
#define	SWITCH	282
#define	WITH	283
#define	RESERVED	284
#define	THROW	285
#define	TRY	286
#define	CATCH	287
#define	FINALLY	288
#define	CLASS	289
#define	CONSTRUCTOR	290
#define	EXTENDS	291
#define	ABSTRACT	292
#define	FINAL	293
#define	PRIVATE	294
#define	PUBLIC	295
#define	STATIC	296
#define	QS_CONST	297
#define	PACKAGE	298
#define	IMPORT	299
#define	EQEQ	300
#define	NE	301
#define	STREQ	302
#define	STRNEQ	303
#define	LE	304
#define	GE	305
#define	OR	306
#define	AND	307
#define	PLUSPLUS	308
#define	MINUSMINUS	309
#define	LSHIFT	310
#define	RSHIFT	311
#define	URSHIFT	312
#define	PLUSEQUAL	313
#define	MINUSEQUAL	314
#define	MULTEQUAL	315
#define	DIVEQUAL	316
#define	LSHIFTEQUAL	317
#define	RSHIFTEQUAL	318
#define	URSHIFTEQUAL	319
#define	ANDEQUAL	320
#define	MODEQUAL	321
#define	XOREQUAL	322
#define	OREQUAL	323
#define	IDENT	324


extern YYSTYPE qsyylval;
