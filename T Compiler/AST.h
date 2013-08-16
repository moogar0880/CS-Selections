#ifndef _AST_H
#define _AST_H
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract Syntax Tree (AST) for T language
 *
 * the dump methods are for debugging - they display the AST to stderr.
 *
 * the analyze methods perform semantic analysis
 *
 * the encode methods perform code generation
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include "Type.h"
#include "SymbolTable.h"
#include <algorithm>    // std::find

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_Node class
 *  This abstract class will be the basis for all other AST classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_List class
 *  This abstract class is the basis for all list classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class AST_List: public AST_Node{
  protected:
    AST_Node* item;
    AST_List* restOfList;
  public:
    virtual ~AST_List();

    // default behavior for lists: recurse left, then recurse right
    virtual void dump();
    virtual AST_Node* analyze();
    virtual void encode();
    virtual void concat(AST_List* l);
    virtual AST_Node* getItem();
    virtual AST_List* getRestOfList();

  protected:
    AST_List(AST_Node* newItem, AST_List* list);
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_Statement class
 *  This abstract class is the basis for all statement classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class AST_Statement: public AST_Node{
  public:
    virtual ~AST_Statement();
    int declarationType;

  protected:
    AST_Statement();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_Expression class
 *  This abstract class is the basis for all expression classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class AST_Expression: public AST_Node{
  public:
    virtual ~AST_Expression();

    Type* type;

  protected:
    AST_Expression();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_Expression class
 *  This abstract class is the basis for all literal classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class AST_Literal: public AST_Expression{
  public:
    virtual ~AST_Literal();

  protected:
    AST_Literal();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_StatementList class
 *  This class represents a list of AST_Statements
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class AST_StatementList: public AST_List{
  public:
    ~AST_StatementList();
    AST_StatementList(AST_Statement* statement, AST_List* restOfList);
    AST_StatementList(AST_Statement* statement);
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
  public:
    ~AST_VariableList();
    AST_VariableList(AST_Expression* expr, AST_VariableList* vl);

    void dump();
    void encode();
    AST_Node* analyze(Type* t);
    AST_Node* analyze(Type* t, SymbolTable* s);
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
    ~AST_Variable();
    AST_Variable(char *id);

    void dump();
    AST_Node* analyze();
    AST_Node* analyze(SymbolTable* s);
    //AST_Node* analyze(std::vector<SymbolTableRecord*> s);
    void encode();
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
  public:
    ~AST_Block();
    AST_Block(AST_StatementList* bl);

    AST_StatementList* list;

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
    Type* myType;

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

class AST_Parameter: public AST_Expression{
  protected:
    char* name;
  public:
    ~AST_Parameter();
    AST_Parameter(Type* t, AST_Expression* n);

    int index;
    void dump();
    AST_Node* analyze();
    void encode();
    Type* getType();
    char* getName();
};

class AST_ArgumentsList: public AST_List{
  public:
    ~AST_ArgumentsList();
    AST_ArgumentsList(AST_Expression* v, AST_ArgumentsList* l);

    void dump();
    AST_Node* analyze();
    void encode();
    int getLength();
};

class AST_ParameterList: public AST_List{
  public:
    ~AST_ParameterList();
    // p is the Parameter Object
    AST_ParameterList(AST_Parameter* p, AST_ParameterList* l);

    void dump();
    AST_Node* analyze(TypeMethod* tm);
    void encode();
};

class AST_ConstructorInvoke: public AST_Statement{
  protected:
    // Used to distinguish between this and super calls
    // 0 = THIS, 1 = SUPER
    int identifier;
    AST_ArgumentsList* params;
  public:
    AST_ConstructorInvoke(int i, AST_ArgumentsList* p);
    ~AST_ConstructorInvoke();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_Delete: public AST_Statement{
  protected:
    Type* type;
    AST_Expression* variable;
    char* variableName;
  public:
    AST_Delete(AST_Expression* v);
    ~AST_Delete();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_MethodInvoke: public AST_Expression{
  protected:
    // Used to distinguish between this and super calls
    // 0 = local method, 1 = variable method, -1 = SUPER method
    int identifier;
    AST_Expression* source;
    char* methodName;
    AST_ArgumentsList* params;
    int methodOffset;
  public:
    // i is the status bit to be used to easily distinguish between types of
    // method invocations
    AST_MethodInvoke(int i, AST_Expression* s, AST_Expression* id, AST_ArgumentsList* p);
    ~AST_MethodInvoke();

    void dump();
    AST_Node* analyze();
    void encode();
};

class AST_MethodDeclarator: public AST_Statement{
  public:
    AST_MethodDeclarator(AST_Expression* n, AST_ParameterList* p);
    ~AST_MethodDeclarator();

    AST_ParameterList* params;
    TypeMethod* heldType;
    char* methodName;
    void dump();
    AST_Node* analyze(Type* ret, TypeClass* tc, int f);
    AST_Node* analyze();
    void encode();
    char* getName();
};

class AST_Method: public AST_Statement{
  protected:
    Type* type;
    AST_MethodDeclarator* declarator;
    AST_StatementList* body;
    int flag;
    char* owner;
    char* mungedName;
  public:
    // id is the MethodDeclarator
    // body is the Block holding the statmentlist
    // Flags: 0 - Method, 1 - Constructor, -1 - Destructor
    AST_Method(Type* t, AST_Statement* id, AST_StatementList* b, int f);
    ~AST_Method();

    void dump();
    AST_Node* analyze();
    void encode();
    void setOwner(char* n);
};

class AST_ClassInstance: public AST_Expression{
  protected:
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
    char* owner;
    char* variable;
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