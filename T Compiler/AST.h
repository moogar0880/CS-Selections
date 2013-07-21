#ifndef _AST_H
#define _AST_H
// Jan 2008
// Abstract Syntax Tree for T language
//
// the dump methods are for debugging - they display the AST to stderr.
//
// the analyze methods perform semantic analysis
//
// the encode methods perform code generation

#include "Type.h"
#include "SymbolTable.h"
#include <algorithm>    // std::find

// abstract class: all AST nodes derived from this
class AST_Node{
  protected:
    int line;

  public:
    virtual ~AST_Node();
    virtual void dump() = 0;
    virtual AST_Node* analyze() = 0;
    virtual void encode() = 0;
    // set the line number of the node
    virtual void setLineNumber( int );
    // get the line number of the node
    virtual int getLineNumber();
    virtual bool areComparable(AST_Node* l, AST_Node* r);
  protected:
    AST_Node();
};

// abstract class: all list nodes derived from this
class AST_List: public AST_Node{
  protected:
    AST_Node* item;
    AST_List* restOfList;
    char* owner;
    bool ownerSet;
  public:
    virtual ~AST_List();

    // default behavior for lists: recurse left, then recurse right
    virtual void dump();
    virtual AST_Node* analyze();
    virtual void encode();
    virtual void concat(AST_List* l);
    virtual AST_Node* getItem();
    virtual AST_List* getRestOfList();
    virtual void setOwner(char* n);

  protected:
    AST_List(AST_Node* newItem, AST_List* list);
};

// abstract class: all statements derived from this
class AST_Statement: public AST_Node{
  public:
    virtual ~AST_Statement();

  protected:
    AST_Statement();
};

// list of statements
class AST_StatementList: public AST_List{
  protected:
    char* owner;
    bool ownerSet;
  public:
    ~AST_StatementList();
    AST_StatementList(AST_Statement* statement, AST_List* restOfList);
    AST_StatementList(AST_Statement* statement);
};

// abstract class: all expression nodes dervived from this
class AST_Expression: public AST_Node{
  public:
    virtual ~AST_Expression();

    Type* type;

  protected:
    AST_Expression();
};

// abstract Literal wrapper Class
class AST_Literal: public AST_Expression{
  public:
    virtual ~AST_Literal();

  protected:
    AST_Literal();
};

class AST_ExpressionStatement: public AST_Statement{
  protected:
    AST_Expression* express;
  public:
    ~AST_ExpressionStatement();
    AST_ExpressionStatement(AST_Expression* e);

    void dump();
    void encode();
    AST_Node* analyze();
};

class AST_VariableList: public AST_List{
  protected:
    char* owner;
    bool ownerSet;
  public:
    ~AST_VariableList();
    AST_VariableList(AST_Expression* expr, AST_VariableList* vl);

    void dump();
    void encode();
    AST_Node* analyze(Type* t);
    void setOwner(char* n);
};

// abstract class: all binary operation expression nodes derived from this
class AST_BinaryOperator: public AST_Expression{
  public:
    virtual ~AST_BinaryOperator();
    AST_Expression* left;
    AST_Expression* right;

  protected:
    AST_BinaryOperator(AST_Expression* l, AST_Expression* r);
};

// abstract class: all unary operation expression nodes dervived from this
class AST_UnaryOperator: public AST_Expression{
  public:
    virtual ~AST_UnaryOperator();
    AST_Expression* left;

  protected:
    AST_UnaryOperator(AST_Expression* l);
};

class AST_MainFunction: public AST_Node{
  protected:
    AST_StatementList* list;

  public:
    AST_MainFunction(AST_StatementList* l);
    ~AST_MainFunction();

  void dump();
  void encode();
  AST_Node* analyze();
};

// variable
class AST_Variable: public AST_Expression{
  public:
    char* name;
    char* owner;
    char* maskedName;
    bool maskSet;
    ~AST_Variable();
    AST_Variable(char *id);

    void dump();
    AST_Node* analyze();
    void encode();
    void setOwner(char* n);
    void setMaskedName();
};

// integer literal
class AST_IntegerLiteral: public AST_Literal{
  protected:
    int value;
  public:
    ~AST_IntegerLiteral();
    AST_IntegerLiteral(int in);

    void dump();
    AST_Node* analyze();
    void encode();
};

// declaration statement
class AST_Declaration: public AST_Statement{
  protected:
    Type* type;
    AST_VariableList* list;

  public:
    ~AST_Declaration();
    AST_Declaration(Type* type, AST_VariableList* l);

    void dump();
    AST_Node* analyze();
    void encode();
};

// assignment statement
class AST_Assignment: public AST_Expression{
  protected:
    AST_Expression* lhs;
    AST_Expression* rhs;

  public:
    ~AST_Assignment();
    AST_Assignment(AST_Expression* lhs, AST_Expression* rhs);

    void dump();
    AST_Node* analyze();
    void encode();
};

// print statement
class AST_Print: public AST_Statement{
  protected:
    // represent with a AST_Expression because a Deref will be added
    AST_Expression* var;

  public:
    ~AST_Print();
    AST_Print(AST_Expression* var);

    void dump();
    AST_Node* analyze();
    void encode();
};

// divide
class AST_Divide: public AST_BinaryOperator{
  public:
    ~AST_Divide();
    AST_Divide(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// multiply
class AST_Multiply: public AST_BinaryOperator{
  public:
    ~AST_Multiply();
    AST_Multiply(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// add
class AST_Add: public AST_BinaryOperator{
  public:
    ~AST_Add();
    AST_Add(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// subtract
class AST_Subtract: public AST_BinaryOperator{
  public:
    ~AST_Subtract();
    AST_Subtract(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// equality
class AST_Equality: public AST_BinaryOperator{
  public:
    ~AST_Equality();
    AST_Equality(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// less than
class AST_LessThan: public AST_BinaryOperator{
  public:
    ~AST_LessThan();
    AST_LessThan(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// greater than
class AST_GreaterThan: public AST_BinaryOperator{
  public:
    ~AST_GreaterThan();
    AST_GreaterThan(AST_Expression* left, AST_Expression* right);

    void dump();
    AST_Node* analyze();
    void encode();
};

// negate
class AST_Negate: public AST_UnaryOperator{
  public:
    ~AST_Negate();
    AST_Negate(AST_Expression* left);

    void dump();
    AST_Node* analyze();
    void encode();
};

// logical not
class AST_Not: public AST_UnaryOperator{
  public:
    ~AST_Not();
    AST_Not(AST_Expression* left);

    void dump();
    AST_Node* analyze();
    void encode();
};

// Block
class AST_Block: public AST_Statement{
  protected:
    AST_StatementList* list;
  public:
    ~AST_Block();
    AST_Block(AST_StatementList* bl);

    void dump();
    void encode();
    AST_Node* analyze();
};

// if then else block
class AST_IfThenElse: public AST_Statement{
  protected:
    AST_Expression* condition;
    AST_Statement* ifstat;
    AST_Statement* elstat;
  public:
    ~AST_IfThenElse();
    AST_IfThenElse(AST_Expression* e, AST_Statement* ifs, AST_Statement* els);

    void dump();
    AST_Node* analyze();
    void encode();
};

// while block
class AST_While: public AST_Statement{
  protected:
    AST_Expression* condition;
    AST_Statement* stat;
    int depth;
  public:
    ~AST_While();
    AST_While(AST_Expression* e, AST_Statement* s);

    void dump();
    AST_Node* analyze();
    void encode();
};

// return
class AST_Return: public AST_Statement{
  protected:
    AST_Expression* var;
  public:
    ~AST_Return();
    AST_Return(AST_Expression* var);

    void dump();
    AST_Node* analyze();
    void encode();
};

// break
class AST_Break: public AST_Statement{
  public:
    ~AST_Break();
    AST_Break();

    void dump();
    AST_Node* analyze();
    void encode();
};

// continue
class AST_Continue: public AST_Statement{
  public:
    ~AST_Continue();
    AST_Continue();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_Class: public AST_Statement{
  protected:
    char* name;
    Type* parent;
    AST_StatementList* fields;

  public:
    ~AST_Class();
    AST_Class(AST_Expression* e, AST_StatementList* l, Type* p);

    void dump();
    AST_Node* analyze();
    void encode();
    char* getName();
    Type* getParent();
    AST_StatementList* getFields();
};

class AST_ClassList: public AST_List{
  public:
    ~AST_ClassList();
    AST_ClassList(AST_Class* c, AST_ClassList* rol);
    AST_ClassList(AST_Class* c);
};

class AST_CompilationUnit: public AST_Node{
  protected:
    AST_ClassList* list;
    AST_MainFunction* main;

  public:
    ~AST_CompilationUnit();
    AST_CompilationUnit(AST_MainFunction* m, AST_ClassList* l1,
      AST_ClassList* l2);

    void dump();
    AST_Node* analyze();
    void encode();
    void listConvert(AST_List* l);
};

class AST_Cast: public AST_Expression{
  protected:
    AST_Expression* expr;
    AST_Expression* cast;
  public:
    ~AST_Cast();
    AST_Cast(AST_Expression* ex, AST_Expression* c);

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_ArgumentsList: public AST_Node{
  public:
    ~AST_ArgumentsList();
    AST_ArgumentsList(AST_Node* gonnaBeNull);

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_ClassInstance: public AST_Expression{
  protected:
    Type* type;
    AST_ArgumentsList* arguments;
  public:
    ~AST_ClassInstance();
    AST_ClassInstance(Type* t, AST_ArgumentsList* args);

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_FieldDeclaration: public AST_Statement{
  protected:
    char* owner;
    bool ownerSet;
    Type* type;
    AST_VariableList* list;
  public:
    AST_FieldDeclaration(Type* t, AST_VariableList* l);
    ~AST_FieldDeclaration();

    void dump();
    AST_Node* analyze();
    void encode();
    void setOwner(char* n);
};

class AST_FieldReference: public AST_Expression{
  protected:
    Type* type;
  public:
    AST_FieldReference(AST_Expression* typ, AST_Expression* id);
    ~AST_FieldReference();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_ClassTypeID: public Type{
  protected:
    char* name;
  public:
    AST_ClassTypeID(AST_Expression* id);
    ~AST_ClassTypeID();

    AST_Node* analyze();
    void encode();
    void dump();
    char* toString();
};

class AST_Null: public AST_Literal{
  protected:
    Type* type;
  public:
    ~AST_Null();
    AST_Null();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_ProgramList: public AST_List{
  protected:
    AST_CompilationUnit* program;
  public:
    AST_ProgramList(AST_CompilationUnit* c);
    ~AST_ProgramList();
};

class AST_EmptyStatement: public AST_Statement{
  public:
    AST_EmptyStatement();
    ~AST_EmptyStatement();
    void dump();
    AST_Node* analyze();
    void encode();
};

// convert unary operator
class AST_Convert: public AST_UnaryOperator{
  public:
    ~AST_Convert();
    AST_Convert(AST_Expression* left);

    void dump();
    AST_Node* analyze();
    void encode();
};

// dereference unary operator
class AST_Deref: public AST_UnaryOperator{
  public:
    ~AST_Deref();
    AST_Deref(AST_Expression* left);

    void dump();
    AST_Node* analyze();
    void encode();
};
#endif