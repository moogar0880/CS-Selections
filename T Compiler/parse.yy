%{
// grammar for the T language
#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "AST.h"
#include "Type.h"
#include "SymbolTable.h"
// this routine is in scan.ll
int getCurrentSourceLineNumber();
// type module is in main.cxx
extern TypeModule* types;
// global symbol table is in main.cxx
extern SymbolTable* symbolTable;
extern std::vector<AST_Class*> globalClassList;
void yyerror(const char *);
void classDump();
extern int yyparse(void);
extern int yylex(void);
extern bool before, after, classes, terminalErrors;
%}

%union {
        int value;
        char* str;
        AST_List* list;
        Type* type;
        AST_ProgramList* pList;
        AST_ExpressionStatement* eStat;
        AST_MainFunction* main;
        AST_StatementList* statementList;
        AST_Block* block;
        AST_Statement* statement;
        AST_Expression* expression;
        AST_Variable* variable;
        AST_Literal* abstractLiteral;
        AST_IntegerLiteral* lit;
        AST_Null* nll;
        AST_VariableList* vlist;
        AST_IfThenElse* ifthenelse;
        AST_While* whilestat;
        AST_Continue* cont;
        AST_Break* breakstat;
        AST_Return* ret;
        AST_CompilationUnit* comp;
        AST_Class* clss;
        AST_ClassList* classList;
        AST_ArgumentsList* args;
        AST_ParameterList* formalParam;
        AST_Parameter* param;
};

%token <value> INTEGER_LITERAL
%token <str> IDENTIFIER
%token '/'
%token '='
%token INT
%token OUT
%token EQ_OP
%token FLOAT
%token PRINT
%token IF
%token ELSE
%token WHILE
%token RETURN
%token BREAK
%token CONTINUE
%token ';'
%token '('
%token ')'
%token BAD
%token MAIN
%token NULL_LITERAL
%token CLASS EXTENDS NEW
%token THIS SUPER DELETE

%type <pList> program
// phase 4
%type <statement> ConstructorDeclaration DestructorDeclaration MethodDeclaration
%type <statement> ConstructorInvocation
%type <statementList> MethodBody
%type <statement> MethodDeclarator DestructorDeclarator ConstructorDeclarator
%type <statement> DeleteStatement
%type <args> ArgumentList
%type <statementList> ConstructorBody DestructorBody
%type <formalParam> FormalParameterList FormalParameters
%type <param> FormalParameter
%type <expression> VariableDeclaratorID MethodInvocation
// phase 3
%type <comp> CompilationUnit
%type <clss> ClassDeclaration
%type <classList> ClassDeclarations
%type <args> Arguments
%type <statementList> ClassBody ClassBodyDeclarations
%type <statement> ClassBodyDeclaration ClassMemberDeclaration FieldDeclaration
%type <type> ReferenceType ClassType
%type <expression> FieldAccess CastExpression
%type <expression> ClassInstanceCreationExpression
// end phase 3
%type <main> MainFunctionDeclaration
%type <block> Block
%type <ifthenelse> IfThenElseStatement
%type <whilestat> WhileStatement
%type <breakstat> BreakStatement
%type <cont> ContinueStatement
%type <ret> ReturnStatement
%type <statementList> MainFunctionBody MainBlock MainBlockStatements BlockStatements
%type <statement> MainBlockStatement BlockStatement Statement OutputStatement
%type <statement> MainVariableDeclarationStatement EmptyStatement
%type <expression> Expression AssignmentExpression
%type <vlist> VariableDeclarators
%type <statement> MainVariableDeclaration
%type <expression> VariableDeclarator ParenExpression
%type <eStat> ExpressionStatement
%type <expression> StatementExpression Assignment LeftHandSide
%type <expression> EqualityExpression RelationalExpression AdditiveExpression
%type <expression> MultiplicativeExpression UnaryExpression
%type <type> Type PrimitiveType NumericType IntegralType
%type <expression> Primary PrimaryNoNewArray
%type <expression> Identifier AssignmentOperator
%type <abstractLiteral> Literal

%start program
%%
program
  : CompilationUnit
  {
    $$ = new AST_ProgramList($1);
    if( before ){
      cerr << "before"<<endl;
      $$->dump();
    }
    $$->analyze();
    if( after ){
      cerr << "after"<<endl;
      $$->dump();
    }

    if( classes ){
      classDump();
    }
    if( !terminalErrors ){
      $$->encode();
    }
    else
      cerr << "Errors in code, aborting compilation\n";
    delete $$;
  }
  ;

CompilationUnit
  : MainFunctionDeclaration
  {
    $$ = new AST_CompilationUnit($1,NULL,NULL);
  }
  | MainFunctionDeclaration ClassDeclarations
  {
    $$ = new AST_CompilationUnit($1,$2,NULL);
  }
  | ClassDeclarations MainFunctionDeclaration
  {
    $$ = new AST_CompilationUnit($2,$1,NULL);
  }
  | ClassDeclarations MainFunctionDeclaration ClassDeclarations
  {
    $$ = new AST_CompilationUnit($2,$1,$3);
  }
  ;

ClassDeclarations
  : ClassDeclaration ClassDeclarations
  {
    $$ = new AST_ClassList($1,$2);
  }
  | ClassDeclaration
  {
    $$ = new AST_ClassList($1,NULL);
  }
  ;

ClassDeclaration
  : CLASS Identifier ClassBody
  {
    $$ = new AST_Class($2,$3,NULL);
  }
  | CLASS Identifier EXTENDS ClassType ClassBody
  {
    $$ = new AST_Class($2,$5,$4);
  }
  ;

ClassBody
  : '{' ClassBodyDeclarations '}'
  {
    $$ = $2;
  }
  | '{' '}'
  {
    $$ = NULL;
  }
  ;

ClassBodyDeclarations
  : ClassBodyDeclaration ClassBodyDeclarations
  {
    $$ = new AST_StatementList($1,$2);
  }
  | ClassBodyDeclaration
  {
    $$ = new AST_StatementList($1,NULL);
  }
  ;

ClassBodyDeclaration
  : ClassMemberDeclaration
  {
    $$ = $1;
  }
  | ConstructorDeclaration
  {
    $$ = $1;
  }
  | DestructorDeclaration
  {
    $$ = $1;
  }
  | ';'
  {
    $$ = NULL;
  }
  ;

ClassMemberDeclaration
  : FieldDeclaration
  {
    $$ = $1;
  }
  | MethodDeclaration
  {
    $$ = $1;
  }
  ;

MethodDeclaration
  : Type MethodDeclarator MethodBody
  {
    $$ = new AST_Method($1, $2, $3, 0);
  }
  ;

MethodDeclarator
  : Identifier FormalParameters
  {
    $$ = new AST_MethodDeclarator($1, $2);
  }
  ;

MethodBody
  : Block
  {
    $$ = new AST_StatementList($1, NULL);
  }
  ;

ConstructorDeclaration//statement
  : ConstructorDeclarator ConstructorBody
  {
    $$ = new AST_Method(NULL, $1, $2, 1);//statement
  }
  ;

ConstructorDeclarator//statement
  : Identifier FormalParameters
  {
    $$ = new AST_MethodDeclarator($1, $2);//statement
  }
  ;

ConstructorBody
  : '{' ConstructorInvocation BlockStatements '}'
  {
    $$ = new AST_StatementList($2, $3);
  }
  | '{' ConstructorInvocation '}'
  {
    $$ = new AST_StatementList($2, NULL);
  }
  | Block
  {
    $$ = new AST_StatementList($1, NULL);
  }
  ;

ConstructorInvocation
  : THIS Arguments ';'
  {
    $$ = new AST_ConstructorInvoke(0, $2);
  }
  | SUPER Arguments ';'
  {
    $$ = new AST_ConstructorInvoke(1, $2);
  }
  ;

DestructorDeclaration
  : DestructorDeclarator DestructorBody
  {
    $$ = new AST_Method(NULL, $1, $2, -1);
  }
  ;

DestructorDeclarator
  : '~' Identifier '(' ')'
  {
    $$ = new AST_MethodDeclarator($2, NULL);
  }
  ;

DestructorBody
  : Block
  {
    $$ = new AST_StatementList($1, NULL);
  }
  ;

FormalParameters
  : '(' FormalParameterList ')'
  {
    $$ = $2;
  }
  | '(' ')'
  {
    $$ = new AST_ParameterList(NULL, NULL);
  }
  ;

FormalParameterList
  : FormalParameter ',' FormalParameterList
  {
    $$ = new AST_ParameterList($1, $3);
  }
  | FormalParameter
  {
    $$ = new AST_ParameterList($1, NULL);
  }
  ;

FormalParameter
  : Type VariableDeclaratorID
  {
    $$ = new AST_Parameter($1, $2);
  }
  ;

VariableDeclaratorID
  : Identifier
  {
    $$ = $1;
  }
  ;

FieldDeclaration
  : Type VariableDeclarators ';'
  {
    $$ = new AST_FieldDeclaration($1,$2);
  }
  ;

MainFunctionDeclaration
  : INT MAIN '(' ')' MainFunctionBody
  {
    $$ = new AST_MainFunction($5);
  }
  ;

MainFunctionBody
  : MainBlock
  {
    $$ = $1;
  }
  ;

Block
  : '{' BlockStatements '}'
  {
    $$ = new AST_Block($2);
  }
  | '{' '}'
  {
    $$ = NULL;
  }
  ;

BlockStatements
  : BlockStatement BlockStatements
  {
    $$ = new AST_StatementList($1, $2);
  }
  | BlockStatement
  {
    $$ = new AST_StatementList($1, NULL);
  }
  ;

VariableDeclarators
  : VariableDeclarator ',' VariableDeclarators
  {
    $$ = new AST_VariableList($1, $3);
  }
  | VariableDeclarator
  {
    $$ = new AST_VariableList($1, NULL);
  }
  ;

VariableDeclarator
  : Identifier
  {
    $$ = $1;
  }
  ;

BlockStatement
  : Statement
  {
    $$ = $1;
  }
  ;

MainBlock
  : '{' MainBlockStatements '}'
  {
    $$ = $2;
  }
  | '{' '}'
  {
    $$ = NULL;
  }
  ;

MainBlockStatements
  : MainBlockStatement MainBlockStatements
  {
    $$ = new AST_StatementList($1, $2);
  }
  | MainBlockStatement
  {
    $$ = new AST_StatementList($1, NULL);
  }
  ;

MainBlockStatement
  : MainVariableDeclarationStatement
  {
    $$ = $1;
  }
  | BlockStatement
  {
    $$ = $1;
  }
  ;

MainVariableDeclarationStatement
  : MainVariableDeclaration ';'
  {
    $$ = $1;
  }
  ;

MainVariableDeclaration
  : Type VariableDeclarators
  {
    $$ = new AST_Declaration($1, $2);
  }
  ;

Statement
  : Block
  {
    $$ = $1;
  }
  | EmptyStatement
  {
    $$ = $1;
  }
  | IfThenElseStatement
  {
    $$ = $1;
  }
  | WhileStatement
  {
    $$ = $1;
  }
  | ReturnStatement
  {
    $$ = $1;
  }
  | BreakStatement
  {
    $$ = $1;
  }
  | ContinueStatement
  {
    $$ = $1;
  }
  | ExpressionStatement
  {
    $$ = $1;
  }
  | OutputStatement
  {
    $$ = $1;
  }
  | DeleteStatement
  {
    $$ = $1;
  }
  ;

DeleteStatement
  : DELETE Expression ';'
  {
    $$ = new AST_Delete($2);
  }
  ;

IfThenElseStatement
  : IF ParenExpression Statement ELSE Statement
  {
    $$ = new AST_IfThenElse($2,$3,$5);
  }
  ;

WhileStatement
  : WHILE ParenExpression Statement
  {
    $$ = new AST_While($2,$3);
  }
  ;

ReturnStatement
  : RETURN ';'
  {
    $$ = new AST_Return(NULL);
  }
  | RETURN Expression ';'
  {
    $$ = new AST_Return($2);
  }
  ;

BreakStatement
  : BREAK ';'
  {
    $$ = new AST_Break();
  }
  ;

ContinueStatement
  : CONTINUE ';'
  {
    $$ = new AST_Continue();
  }
  ;

EmptyStatement
  : ';'
  {
    $$ = new AST_EmptyStatement();
  }
  ;

OutputStatement
  : OUT Expression ';'
  {
    $$ = new AST_Print($2);
  }
  ;

ParenExpression
  : '(' Expression ')'
  {
    $$ = $2;
  }
  ;

ExpressionStatement
  : StatementExpression ';'
  {
    $$ = new AST_ExpressionStatement($1);
  }
  ;

StatementExpression
  : Assignment
  {
    $$ = $1;
  }
  | MethodInvocation
  {
    $$ = $1;
  }
  ;

Expression
  : AssignmentExpression
  {
    $$ = $1;
  }
  ;

AssignmentExpression
  : Assignment
  {
    $$ = $1;
  }
  | EqualityExpression
  {
    $$ = $1;
  }
  ;

Assignment
  : LeftHandSide AssignmentOperator AssignmentExpression
  {
    $$ = new AST_Assignment($1, $3);
  }
  ;

LeftHandSide
  : Identifier
  {
    $$ = $1;
  }
  | FieldAccess
  {
    $$ = $1;
  }
  ;

EqualityExpression
  : EqualityExpression EQ_OP RelationalExpression
  {
    $$ = new AST_Equality($1, $3);
  }
  | RelationalExpression
  {
    $$ = $1;
  }
  ;

RelationalExpression
  : RelationalExpression '<' AdditiveExpression
  {
    $$ = new AST_LessThan($1, $3);
  }
  | RelationalExpression '>' AdditiveExpression
  {
    $$ = new AST_GreaterThan($1, $3);
  }
  | AdditiveExpression
  {
    $$ = $1;
  }
  ;

AdditiveExpression
  : AdditiveExpression '+' MultiplicativeExpression
  {
    $$ = new AST_Add($1, $3);
  }
  | AdditiveExpression '-' MultiplicativeExpression
  {
    $$ = new AST_Subtract($1, $3);
  }
  | MultiplicativeExpression
  {
    $$ = $1;
  }
  ;

MultiplicativeExpression
  : MultiplicativeExpression '*' UnaryExpression
  {
    $$ = new AST_Multiply($1, $3);
  }
  | MultiplicativeExpression '/' UnaryExpression
  {
    $$ = new AST_Divide($1, $3);
  }
  | UnaryExpression
  {
    $$ = $1;
  }
  ;

UnaryExpression
  : '-' UnaryExpression
  {
    $$ = new AST_Negate($2);
  }
  | '!' UnaryExpression
  {
    $$ = new AST_Not($2);
  }
  | CastExpression
  {
    $$ = $1;
  }
  ;

CastExpression
  : ParenExpression CastExpression
  {
    $$ = new AST_Cast($1,$2);
  }
  | Primary
  {
    $$ = $1;
  }
  ;

Primary
  : Identifier
  {
    $$ = $1;
  }
  | PrimaryNoNewArray
  {
    $$ = $1;
  }
  ;

PrimaryNoNewArray
  : ParenExpression
  {
    $$ = $1;
  }
  | FieldAccess
  {
    $$ = $1;
  }
  | ClassInstanceCreationExpression
  {
    $$ = $1;
  }
  | Literal
  {
    $$ = $1;
  }
  | THIS
  {
    $$ = new AST_Variable((char*)"THIS");
  }
  | MethodInvocation
  {
    $$ = $1;
  }
  ;

MethodInvocation
  : Identifier Arguments
  {
    $$ = new AST_MethodInvoke(0, NULL, $1, $2);
  }
  | Primary '.' Identifier Arguments
  {
    $$ = new AST_MethodInvoke(1, $1, $3, $4);
  }
  | SUPER '.' Identifier Arguments
  {
    $$ = new AST_MethodInvoke(-1, NULL, $3, $4);
  }
  ;

AssignmentOperator
  : '='
  {
    //no rules
  }
  ;

ClassInstanceCreationExpression
  : NEW ClassType Arguments
  {
    $$ = new AST_ClassInstance($2,$3);
  }
  ;

FieldAccess
  : Primary '.' Identifier
  {
    $$ = new AST_FieldReference($1,$3);
  }
  | SUPER '.' Identifier
  {
    AST_Variable* superExpress = new AST_Variable((char*)"SUPER");
    $$ = new AST_FieldReference(superExpress, $3);
  }
  ;

Arguments
  : '(' ArgumentList ')'
  {
    $$ = $2;
  }
  | '(' ')'
  {
    $$ = NULL;
  }
  ;

ArgumentList
  : Expression ',' ArgumentList
  {
    $$ = new AST_ArgumentsList($1, $3);
  }
  | Expression
  {
    $$ = new AST_ArgumentsList($1, NULL);
  }
  ;

Type
  : ReferenceType
  {
    $$ = $1;
  }
  | PrimitiveType
  {
    $$ = $1;
  }
  ;

PrimitiveType
  : NumericType
  {
    $$ = $1;
  }
  ;

NumericType
  : IntegralType
  {
    $$ = $1;
  }
  ;

ReferenceType
  : ClassType
  {
    $$ = $1;
  }
  ;

ClassType
  : Identifier
  {
    $$ = new AST_ClassTypeID($1);
  }
  ;

IntegralType
  : INT
  {
    $$ = types->intType();
  }
  ;

Identifier
  : IDENTIFIER
  {
    $$ = new AST_Variable($1);
  }
  ;

Literal
  : INTEGER_LITERAL
  {
    $$ = new AST_IntegerLiteral($1);
  }
  | NULL_LITERAL
  {
    $$ = new AST_Null();
  }
  ;
%%

void yyerror(const char *s){
  cerr << getCurrentSourceLineNumber() << ": parse error" << endl;
}

void classDump(){
  cerr << "Classes\n--------\n";
  std::vector<TypeClass*> classes = types->getClassList();
  std::vector<TypeClass*>::iterator it;
  for(it = classes.begin(); it != classes.end(); ++it ){
    cerr << "Class " << (*it)->getName() << endl;
    if( (*it)->getParent() != NULL ){
      cerr << "\tParent: " << ((*it)->getParent())->toString() << endl;
    }
    else{
      cerr << "\tParent: NULL" << endl;
    }
    if( (*it)->getDestructor() != NULL )
      cerr << "\tDestructor: ~" << (*it)->getDestructor()->getName() << "()" << endl;
    else
      cerr << "\tDestructor: ~Default()" << endl;
    cerr << "\tMethods/Constructors:" << endl;
    SymbolTableRecord* scan = (*it)->getSymbolTable()->methodHead;
    TypeMethod* tMeth = NULL;
    while(scan != NULL){
      cerr << "\t\t";
      tMeth = (TypeMethod*)scan->type;
      if( tMeth->isConstructor ){
        cerr << tMeth->getName();
        std::vector<Type*> tScan = tMeth->getSig();
        if( tScan.size() == 1 ){
          cerr << "()" << endl;
        }
        else{
          cerr << "(";
          std::vector<Type*>::iterator typeIt;
          for(typeIt = tScan.begin() + 1; typeIt != tScan.end(); ++typeIt){
            cerr << (*typeIt)->toString() << " ";
          }
          cerr << ");\n";
        }
      }
      else{
        if( tMeth->returnType == NULL ){
          cerr << "void " << tMeth->getName() << "(";
        }
        else{
          cerr << tMeth->returnType->toString() << " " << tMeth->getName();
          std::vector<Type*> tScan = tMeth->getSig();
          if( tScan.size() == 1 ){
            cerr << "()" << endl;
          }
          else{
            cerr << "(";
            std::vector<Type*>::iterator typeIt;
            for(typeIt = tScan.begin() + 1; typeIt != tScan.end(); ++typeIt){
              cerr << (*typeIt)->toString() << " ";
            }
            cerr << ");\n";
          }
        }
      }
      scan = scan->next;
    }
    cerr << "\tFields:" << endl;
    scan = (*it)->getSymbolTable()->getList();
    while(scan != NULL){
      cerr << "\t\t" << scan->type->toString() << " " << scan->name << endl;
      scan = scan->next;
    }
  }
}