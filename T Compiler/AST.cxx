// Jan 2008
// Abstract Syntax Tree implementation for T language
//
// semantic analysis methods are in analyze.cxx

#include <iostream>
using namespace std;
#include "AST.h"
#include "Type.h"
#include <string.h>

// this routine is in scan.ll
int getCurrentSourceLineNumber();
// global symbol table is in main.cxx
extern SymbolTable* symbolTable;
// global type module is in main.cxx
extern TypeModule* types;
// list of all valid classes, containing all their information
std::vector<AST_Class*> globalClassList;

//------------------AST_Node------------------
AST_Node::AST_Node(){
  // by default get the current line number in the scanner
  line = getCurrentSourceLineNumber();
}

AST_Node::~AST_Node(){}

void AST_Node::setLineNumber(int lineNumber){
  line = lineNumber;
}

int AST_Node::getLineNumber(){
  return line;
}

//------------------AST_List------------------
AST_List::AST_List(AST_Node* newItem, AST_List* list){
  item = newItem;
  restOfList = list;
  ownerSet = false;
}

AST_List::~AST_List(){
  delete item;
  if (restOfList != NULL) delete restOfList;
}

void AST_List::dump(){
  cerr << "List Item\n";
  item->dump();
  if (restOfList != NULL) restOfList->dump();
  else cerr << "End of List\n";
}

void AST_List::concat(AST_List* l){
  if( restOfList != NULL )
    restOfList->concat(l);
  else
    restOfList = l;
}

AST_Node* AST_List::getItem(){
  return item;
}

AST_List* AST_List::getRestOfList(){
  return restOfList;
}

void AST_List::setOwner(char* n){
  owner = n;
  ownerSet = true;
}

//------------------AST_Statement------------------
AST_Statement::~AST_Statement(){}

AST_Statement::AST_Statement(){}

//------------------AST_Literal------------------
AST_Literal::AST_Literal(){}

AST_Literal::~AST_Literal(){}

//------------------AST_StatementList------------------
AST_StatementList::AST_StatementList(AST_Statement* s,
  AST_List* l) : AST_List((AST_Node*) s, l){}

AST_StatementList::~AST_StatementList(){
  if( owner != NULL )
    delete owner;
}

//------------------AST_Expression------------------
AST_Expression::~AST_Expression(){}

AST_Expression::AST_Expression(){
  type = types->noType();
}

//------------------AST_ExpressionStatement------------------
AST_ExpressionStatement::AST_ExpressionStatement(AST_Expression* e){
  express = e;
}
AST_ExpressionStatement::~AST_ExpressionStatement(){
  delete express;
}

void AST_ExpressionStatement::dump(){}

//------------------AST_MainFunction------------------
AST_MainFunction::AST_MainFunction(AST_StatementList* l){
  list = l;
}
AST_MainFunction::~AST_MainFunction(){
  delete list;
}
void AST_MainFunction::dump(){
  list->dump();
}

//------------------AST_ProgramList------------------
AST_ProgramList::AST_ProgramList(AST_CompilationUnit* c):AST_List((AST_Node*)c,
  NULL){
  if( c == NULL )
    cerr << "AST_ProgramList::AST_ProgramList called" << endl;
  program = c;
}
AST_ProgramList::~AST_ProgramList(){
  //delete main;
}

//------------------AST_BinaryOperator------------------
AST_BinaryOperator::~AST_BinaryOperator(){
  delete left;
  delete right;
}

AST_BinaryOperator::AST_BinaryOperator(AST_Expression* l, AST_Expression* r){
  left = l;
  right = r;
}

//------------------AST_UnaryOperator------------------
AST_UnaryOperator::~AST_UnaryOperator(){
  delete left;;
}

AST_UnaryOperator::AST_UnaryOperator(AST_Expression* l){
  left = l;
}

//------------------AST_IntegerLiteral------------------
AST_IntegerLiteral::~AST_IntegerLiteral(){}

AST_IntegerLiteral::AST_IntegerLiteral(int in){
  type = types->intType();
  value = in;
}

void AST_IntegerLiteral::dump(){
  cerr << "IntegerLiteral " << value << " " << type->toString() << endl;
}

//------------------AST_Variable------------------
AST_Variable::AST_Variable(char* in){
  name = in;
}

AST_Variable::~AST_Variable(){
  delete name;
}

void AST_Variable::dump(){
  cerr << "Variable " << name << endl;
}

void AST_Variable::setOwner(char* n){
  owner = n;
  setMaskedName();
}

// masks the variables actual name for class variables
// being inserted into the symbol table during analysis
void AST_Variable::setMaskedName(){
  int size = strlen(owner) + strlen(name);
  char tmp[size];
  strcpy(tmp, owner);
  strcat(tmp, name);
  maskedName = (char*)tmp;
  maskSet = true;
}

//------------------AST_VariableList------------------
AST_VariableList::AST_VariableList(AST_Expression* e,
  AST_VariableList* l):AST_List((AST_Node*)e, l){
  ownerSet = false;
}

AST_VariableList::~AST_VariableList(){}

void AST_VariableList::dump(){
  ((AST_Expression*)(item))->dump();
  if (restOfList != NULL) restOfList->dump();
  else cerr << "End of List\n";
}

void AST_VariableList::setOwner(char* n){
  owner = n;
  ownerSet = true;
}

//------------------AST_Declaration------------------
AST_Declaration::AST_Declaration(Type* t, AST_VariableList* l){
  type = t;
  list = l;
}

AST_Declaration::~AST_Declaration(){
  delete list;
}

void AST_Declaration::dump(){
  cerr << "Declaration " << type->toString() << "\n" << endl;
  list->dump();
}

//------------------AST_Assignment------------------
AST_Assignment::AST_Assignment(AST_Expression* l, AST_Expression* r){
  lhs = l;
  rhs = r;
}

AST_Assignment::~AST_Assignment(){
  delete lhs;
  delete rhs;
}

void AST_Assignment::dump(){
  cerr << "Assignment" << endl;
  lhs->dump();
  rhs->dump();
}

//------------------AST_Print------------------
AST_Print::AST_Print(AST_Expression* v){
  var = (AST_Expression*) v;
}

AST_Print::~AST_Print(){
  delete var;
}

void AST_Print::dump(){
  cerr << "Print" << endl;
  var->dump();
}

//------------------AST_Divide------------------
AST_Divide::AST_Divide(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Divide::~AST_Divide(){}

void AST_Divide::dump(){
  cerr << "Divide " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_Multiply------------------
AST_Multiply::AST_Multiply(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Multiply::~AST_Multiply(){}

void AST_Multiply::dump(){
  cerr << "Multiply " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_Add------------------
AST_Add::AST_Add(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Add::~AST_Add(){}

void AST_Add::dump(){
  cerr << "Add " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_Subtract------------------
AST_Subtract::AST_Subtract(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Subtract::~AST_Subtract(){}

void AST_Subtract::dump(){
  cerr << "Subtract " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_Equality------------------
AST_Equality::AST_Equality(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Equality::~AST_Equality(){}

void AST_Equality::dump(){
  cerr << "Equality Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_LessThan------------------
AST_LessThan::AST_LessThan(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_LessThan::~AST_LessThan(){}

void AST_LessThan::dump(){
  cerr << "Less Than Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_GreaterThan------------------
AST_GreaterThan::AST_GreaterThan(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_GreaterThan::~AST_GreaterThan(){}

void AST_GreaterThan::dump(){
  cerr << "Greater Than Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

//------------------AST_Negate------------------
AST_Negate::AST_Negate(AST_Expression* l) :
  AST_UnaryOperator(l){}

AST_Negate::~AST_Negate(){}

void AST_Negate::dump(){
  cerr << "Negate " << type->toString() << endl;
  left->dump();
}

//------------------AST_Not------------------
AST_Not::AST_Not(AST_Expression* l) :
  AST_UnaryOperator(l){}

AST_Not::~AST_Not(){}

void AST_Not::dump(){
  cerr << "Logical Not " << type->toString() << endl;
  left->dump();
}

//------------------AST_Block------------------
AST_Block::AST_Block(AST_StatementList* bl){
  list = bl;
}

AST_Block::~AST_Block(){
  delete list;
}

void AST_Block::dump(){
  cerr << "New Block\n";
}

//------------------AST_IfThenElse------------------
AST_IfThenElse::AST_IfThenElse(AST_Expression* e, AST_Statement* ifs,
  AST_Statement* els){
  condition = e;
  ifstat = ifs;
  elstat = els;
}

AST_IfThenElse::~AST_IfThenElse(){
  delete ifstat;
  delete elstat;
}

void AST_IfThenElse::dump(){
  cerr << "If " << condition << " else\n";
}

//------------------AST_While------------------
AST_While::AST_While(AST_Expression* e, AST_Statement* s){
  condition = e;
  stat = s;
  depth = 0;
}

AST_While::~AST_While(){
  delete condition;
  delete stat;
}

void AST_While::dump(){
  cerr << "While " << condition << "\n";
}

//------------------AST_Return------------------
AST_Return::AST_Return(AST_Expression* e){
  var = e;
}

AST_Return::~AST_Return(){
  delete var;
}

void AST_Return::dump(){
  cerr << "Return " << var << "\n";
}

//------------------AST_Break------------------
AST_Break::AST_Break(){}

AST_Break::~AST_Break(){}

void AST_Break::dump(){
  cerr << "Break\n";
}

//------------------AST_Continue------------------
AST_Continue::AST_Continue(){}

AST_Continue::~AST_Continue(){}

void AST_Continue::dump(){
  cerr << "Continue\n";
}

//------------------AST_Class------------------
AST_Class::AST_Class(AST_Expression* n, AST_StatementList* l, Type* p){
  name = ((AST_Variable*)(n))->name;
  parent = p;
  fields = l;

  if( fields != NULL )
    fields->setOwner(name);

  if( !types->createNewClassType(name) ){
    int l = ((AST_Node*)this)->getLineNumber();
    cerr << l << ": Redeclaration of ClassType " << name << endl;
  }
}

AST_Class::~AST_Class(){
  delete name;
  delete parent;
  delete fields;
}

void AST_Class::dump(){
  cerr << "Class " << name << "\n";
}

char* AST_Class::getName(){
  return name;
}

Type* AST_Class::getParent(){
  return parent;
}

AST_StatementList* AST_Class::getFields(){
  return fields;
}

//------------------AST_StatementList------------------
AST_ClassList::AST_ClassList(AST_Class* c,
  AST_ClassList* l) : AST_List((AST_Node*) c, l){}

AST_ClassList::~AST_ClassList(){}

//------------------AST_CompilationUnit----------------
AST_CompilationUnit::AST_CompilationUnit(AST_MainFunction* m,
  AST_ClassList* l1, AST_ClassList* l2)
{
  main = m;
  list = l1; //could be null if no classes defined
  if( l2 != NULL ){ //if classes are declared around main
    list->concat(l2);
  }

  AST_Variable* objectName = new AST_Variable(((char*) "Object"));
  // Object class has no parent and no non-method fields for phase 3
  AST_Class* objectClass = new AST_Class( objectName, NULL, NULL );
  globalClassList.push_back( objectClass );
  if( list != NULL )
    listConvert(list);
}

void AST_CompilationUnit::listConvert(AST_List* l){
  if( l != NULL ){
    globalClassList.push_back((AST_Class*)(l->getItem()));
    listConvert(l->getRestOfList());
  }
}

bool AST_Node::areComparable(AST_Node* l, AST_Node* r){
  AST_Class* left = (AST_Class*) l;
  AST_Class* right = (AST_Class*) r;
  std::vector<AST_Class*>::iterator it;
  it = std::find(globalClassList.begin(), globalClassList.end(), left);
  if( (*it)->getParent() == NULL )
    return false; // reached object and parent not found
  else{
    while( it != globalClassList.end() ){
      if( (*it)->getParent() == types->classType(right->getName()))
        return true;
      ++it;
    }
  }
  return false; // classes are not related
}

AST_CompilationUnit::~AST_CompilationUnit(){}

void AST_CompilationUnit::dump(){
  cerr << "CompilationUnit\n";
}

//------------------AST_Cast------------------
AST_Cast::AST_Cast(AST_Expression* ex, AST_Expression* c){
  expr = ex;
  cast = c;
}

AST_Cast::~AST_Cast(){}

void AST_Cast::dump(){
  cerr << "Casting from " << cast << " to " << expr << endl;
}

//------------------AST_ArgumentsList------------------
AST_ArgumentsList::AST_ArgumentsList(AST_Node* gonnaBeNull){}

AST_ArgumentsList::~AST_ArgumentsList(){}

void AST_ArgumentsList::dump(){
  cerr << "Argument List" << endl;
}

//------------------AST_ClassInstance------------------
AST_ClassInstance::AST_ClassInstance(Type* t, AST_ArgumentsList* args){
  type = t;
  arguments = args;
}

AST_ClassInstance::~AST_ClassInstance(){
  delete type;
  delete arguments;
}

void AST_ClassInstance::dump(){
  cerr << "Class Instance of type " << type << endl;
}

//------------------AST_FieldDeclaration------------------
AST_FieldDeclaration::AST_FieldDeclaration(Type* t, AST_VariableList* l){
  type = t;
  list = l;
  ownerSet = false;
}

AST_FieldDeclaration::~AST_FieldDeclaration(){}

void AST_FieldDeclaration::dump(){
  cerr << "Field Declaration of type " << type << "\n";
}

void AST_FieldDeclaration::setOwner(char* n){
  owner = n;
  ownerSet = true;
}

//------------------AST_FieldReference------------------
AST_FieldReference::AST_FieldReference(AST_Expression* typ, AST_Expression* id){
  // type = types->classType(((AST_Variable*)(typ))->name);
  cerr << ((AST_Variable*)(id))->name << endl;
  symbolTable->lookup(((AST_Variable*)(typ))->name, type);
  cerr << "Type = " << type->toString() << endl;
  if( type == NULL )
    type = types->noType();
}

AST_FieldReference::~AST_FieldReference(){
  delete type;
}

void AST_FieldReference::dump(){
  cerr << "Field Reference of type " << type << "\n";
}

//------------------AST_ClassTypeID------------------
AST_ClassTypeID::AST_ClassTypeID(AST_Expression* id){
  name = ((AST_Variable*)(id))->name;
}

AST_ClassTypeID::~AST_ClassTypeID(){}

void AST_ClassTypeID::dump(){
  cerr << "Class ID Converter\n";
}

char* AST_ClassTypeID::toString(){
  return name;
}

//------------------AST_Null------------------
AST_Null::AST_Null(){
  // type = types->nullType();
}

AST_Null::~AST_Null(){}

void AST_Null::dump(){
  cerr << "Null Literal\n";
}

//----------------AST_EmptyStatement----------------
AST_EmptyStatement::AST_EmptyStatement(){}
AST_EmptyStatement::~AST_EmptyStatement(){}

void AST_EmptyStatement::dump(){
  cerr << "Empty\n";
}

//------------------AST_Convert------------------
AST_Convert::AST_Convert(AST_Expression* l) : AST_UnaryOperator(l){}

AST_Convert::~AST_Convert(){}

void AST_Convert::dump(){
  cerr << "Convert " << type->toString() << endl;
  left->dump();
}

//------------------AST_Deref------------------
AST_Deref::AST_Deref(AST_Expression* l) : AST_UnaryOperator(l){}

AST_Deref::~AST_Deref(){}

void AST_Deref::dump(){
  cerr << "Deref " << type->toString() << endl;
  left->dump();
}
