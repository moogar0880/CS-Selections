/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract Syntax Tree (AST) implementation for T language
 * semantic analysis methods are in analyze.cxx
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
extern bool terminalErrors;
// list of all valid classes, containing all their information
std::vector<AST_Class*> globalClassList;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_Node class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract AST_List Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_List::AST_List(AST_Node* newItem, AST_List* list){
  item = newItem;
  restOfList = list;
}

AST_List::~AST_List(){
  if( item != NULL )
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Statement Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Statement::~AST_Statement(){}

AST_Statement::AST_Statement(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Literal Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Literal::AST_Literal(){}

AST_Literal::~AST_Literal(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_StatementList Class
 *  Implemented as a linked list of AST_Statements
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_StatementList::AST_StatementList(AST_Statement* s,
  AST_List* l) : AST_List((AST_Node*) s, l){}

AST_StatementList::~AST_StatementList(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Expression Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Expression::~AST_Expression(){}

AST_Expression::AST_Expression(){
  type = types->noType();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ExpressionStatement Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_ExpressionStatement::AST_ExpressionStatement(AST_Expression* e){
  express = e;
}
AST_ExpressionStatement::~AST_ExpressionStatement(){
  delete express;
}

void AST_ExpressionStatement::dump(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MainFunction Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_MainFunction::AST_MainFunction(AST_StatementList* l){
  list = l;
}
AST_MainFunction::~AST_MainFunction(){
  delete list;
}
void AST_MainFunction::dump(){
  list->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ProgramList Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_ProgramList::AST_ProgramList(AST_CompilationUnit* c):AST_List((AST_Node*)c,
  NULL){
  if( c == NULL )
    cerr << "AST_ProgramList::AST_ProgramList called" << endl;
  program = c;
}
AST_ProgramList::~AST_ProgramList(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_BinaryOperator Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_BinaryOperator::~AST_BinaryOperator(){
  delete left;
  delete right;
}

AST_BinaryOperator::AST_BinaryOperator(AST_Expression* l, AST_Expression* r){
  left = l;
  right = r;

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Unary Operator Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_UnaryOperator::~AST_UnaryOperator(){
  delete left;
}

AST_UnaryOperator::AST_UnaryOperator(AST_Expression* l){
  left = l;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IntegerLiteral Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_IntegerLiteral::~AST_IntegerLiteral(){}

AST_IntegerLiteral::AST_IntegerLiteral(int in){
  type = types->intType();
  value = in;
}

void AST_IntegerLiteral::dump(){
  cerr << "IntegerLiteral " << value << " " << type->toString() << endl;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Variable Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Variable::AST_Variable(char* in){
  name = in;
}

AST_Variable::~AST_Variable(){
  delete name;
}

void AST_Variable::dump(){
  cerr << "Variable " << name << endl;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_VariableList Class
 *  Implemented as a linked list of AST_Expressions
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_VariableList::AST_VariableList(AST_Expression* e,
  AST_VariableList* l):AST_List((AST_Node*)e, l){}

AST_VariableList::~AST_VariableList(){}

void AST_VariableList::dump(){
  ((AST_Expression*)(item))->dump();
  if (restOfList != NULL) restOfList->dump();
  else cerr << "End of List\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Declaration Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Assignment Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Print Class
 *  Class for print statements
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Divide Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Divide::AST_Divide(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Divide::~AST_Divide(){}

void AST_Divide::dump(){
  cerr << "Divide " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Multiply Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Multiply::AST_Multiply(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Multiply::~AST_Multiply(){}

void AST_Multiply::dump(){
  cerr << "Multiply " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Add Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Add::AST_Add(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Add::~AST_Add(){}

void AST_Add::dump(){
  cerr << "Add " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Subtract Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Subtract::AST_Subtract(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Subtract::~AST_Subtract(){}

void AST_Subtract::dump(){
  cerr << "Subtract " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Equality Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Equality::AST_Equality(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_Equality::~AST_Equality(){}

void AST_Equality::dump(){
  cerr << "Equality Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_LessThan Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_LessThan::AST_LessThan(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_LessThan::~AST_LessThan(){}

void AST_LessThan::dump(){
  cerr << "Less Than Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_GreaterThan Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_GreaterThan::AST_GreaterThan(AST_Expression* l, AST_Expression* r) :
  AST_BinaryOperator(l, r){}

AST_GreaterThan::~AST_GreaterThan(){}

void AST_GreaterThan::dump(){
  cerr << "Greater Than Expression " << type->toString() << endl;
  left->dump();
  right->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Negate Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Negate::AST_Negate(AST_Expression* l) :
  AST_UnaryOperator(l){}

AST_Negate::~AST_Negate(){}

void AST_Negate::dump(){
  cerr << "Negate " << type->toString() << endl;
  left->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Not Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Not::AST_Not(AST_Expression* l) :
  AST_UnaryOperator(l){}

AST_Not::~AST_Not(){}

void AST_Not::dump(){
  cerr << "Logical Not " << type->toString() << endl;
  left->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Block Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Block::AST_Block(AST_StatementList* bl){
  list = bl;
}

AST_Block::~AST_Block(){
  delete list;
}

void AST_Block::dump(){
  cerr << "New Block\n";
  list->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IfThenElse Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
  ifstat->dump();
  elstat->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_While Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
  stat->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Return Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Return::AST_Return(AST_Expression* e){
  var = e;
}

AST_Return::~AST_Return(){
  delete var;
}

void AST_Return::dump(){
  cerr << "Return\n";
  var->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Break Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Break::AST_Break(){}

AST_Break::~AST_Break(){}

void AST_Break::dump(){
  cerr << "Break\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Continue Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Continue::AST_Continue(){}

AST_Continue::~AST_Continue(){}

void AST_Continue::dump(){
  cerr << "Continue\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Parameter Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Parameter::AST_Parameter(Type* t, AST_Expression* n){
  type = t;
  name = ((AST_Variable*)(n))->name;
}

AST_Parameter::~AST_Parameter(){
  delete type;
  delete name;
}

void AST_Parameter::dump(){
  cerr << type->toString() << " " << name << "\n";
}

Type* AST_Parameter::getType(){
  return type;
}

char* AST_Parameter::getName(){
  return name;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Class Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Class::AST_Class(AST_Expression* n, AST_StatementList* l, Type* p){
  name = ((AST_Variable*)(n))->name;
  parent = p;
  fields = l;

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
  fields->dump();
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassList Class
 *  Implemented as a linked list of AST_Class objects
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_ClassList::AST_ClassList(AST_Class* c,
  AST_ClassList* l) : AST_List((AST_Node*) c, l){}

AST_ClassList::~AST_ClassList(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_CompilationUnit Class
 *  This class contains the entirety of the AST
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_CompilationUnit::AST_CompilationUnit(AST_MainFunction* m,
  AST_ClassList* l1, AST_ClassList* l2)
{
  main = m;
  list = l1; //could be null if no classes defined
  if( l2 != NULL ){ //if classes are declared around main
    list->concat(l2);
  }

  //Object class has no parent and no non-method fields for phase 3
  TypeClass* obj = (TypeClass*)(types->createNewClassType((char *)"Object"));
  if( obj != NULL ){
    obj->setParent(NULL);
    TypeMethod* objConstructor = new TypeMethod((char*)"Object", NULL, true, false);
    TypeMethod* objDestructor  = new TypeMethod((char*)"Object", NULL, false, true);
    TypeMethod* equals         = new TypeMethod((char*)"equals", types->intType(), false, false);
    equals->addParam((char*)"o", obj);
    obj->add((char*)"Object", objConstructor);
    obj->add((char*)"Object", objDestructor);
    obj->add((char*)"equals", equals);
  }
  if( list != NULL )
    listConvert(list);
}

// Helper function to convert an AST_List into a Vector
void AST_CompilationUnit::listConvert(AST_List* l){
  if( l != NULL ){
    AST_Class* classObject = (AST_Class*)(l->getItem());
    TypeClass* c = (TypeClass*)(types->createNewClassType(classObject->getName()));
    if( c != NULL ){
      c->setParent(classObject->getParent());
    }
    listConvert(l->getRestOfList());
  }
}

AST_CompilationUnit::~AST_CompilationUnit(){}

void AST_CompilationUnit::dump(){
  cerr << "CompilationUnit\n";
  list->dump();
  main->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Cast Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Cast::AST_Cast(AST_Expression* ex, AST_Expression* c){
  expr = ex;
  cast = c;
}

AST_Cast::~AST_Cast(){}

void AST_Cast::dump(){
  cerr << "Casting from " << cast << " to " << expr << endl;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ArgumentsList Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_ArgumentsList::AST_ArgumentsList(AST_Expression* v, AST_ArgumentsList* l)
  : AST_List((AST_Node*) v, l){}

AST_ArgumentsList::~AST_ArgumentsList(){}

void AST_ArgumentsList::dump(){
  cerr << "Argument List" << endl;
  item->dump();
  restOfList->dump();
}

int AST_ArgumentsList::getLength(){
  int i = 0;
  AST_Node* scan = item;
  while(scan != NULL){
    i++;
    scan = restOfList->getItem();
  }
  return i;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassInstance Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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
  arguments->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldDeclaration Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_FieldDeclaration::AST_FieldDeclaration(Type* t, AST_VariableList* l){
  type = t;
  list = l;
  ownerSet = false;
  declarationType = 0;
}

AST_FieldDeclaration::~AST_FieldDeclaration(){}

void AST_FieldDeclaration::dump(){
  cerr << "Field Declaration of type " << type->toString() << endl;
}

void AST_FieldDeclaration::setOwner(char* n){
  owner = n;
  ownerSet = true;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldReference Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 AST_FieldReference::AST_FieldReference(AST_Expression* o, AST_Expression* v){
  // type = types->classType(((AST_Variable*)(typ))->name);
  type = types->noType();
  owner = ((AST_Variable*)(o))->name;
  variable = ((AST_Variable*)(v))->name;
}

AST_FieldReference::~AST_FieldReference(){
  delete type;
}

void AST_FieldReference::dump(){
  cerr << "Field Reference of type " << type->toString() << "\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassTypeID Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Null Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Null::AST_Null(){
  // type = types->nullType();
}

AST_Null::~AST_Null(){}

void AST_Null::dump(){
  cerr << "Null Literal\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_EmptyStatement Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_EmptyStatement::AST_EmptyStatement(){}
AST_EmptyStatement::~AST_EmptyStatement(){}

void AST_EmptyStatement::dump(){
  cerr << "Empty\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ParameterList Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_ParameterList::AST_ParameterList(AST_Parameter* p, AST_ParameterList* l)
  : AST_List((AST_Node*) p, l){}

AST_ParameterList::~AST_ParameterList(){}

void AST_ParameterList::dump(){
  ((AST_Parameter*)(item))->dump();
  if( restOfList != NULL ){
    cerr << ", ";
    ((AST_ParameterList*)(restOfList))->dump();
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  AST_ConstructorInvoke
 *  The identifier int will be set to either 0 (THIS) or 1 (SUPER) to
 *    specify which constructor is to be called. methodName will be NULL,
 *    as the constructor name is the class name and the params could be
 *    NULL, depending on the constructor being called.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

AST_ConstructorInvoke::AST_ConstructorInvoke(int i, AST_ArgumentsList* p){
  identifier = i;
  params = p;
}

AST_ConstructorInvoke::~AST_ConstructorInvoke(){
  delete params;
}

void AST_ConstructorInvoke::dump(){
  cerr << "Constructor Invoke " << endl;
  if( params != NULL )
    params->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  AST_Delete aka DestructorInvoke
 *  If flag is set to -1 a Destructor is being invoked.
 *    This means that the DELETE keyword has been used. Because
 *    destructors have no arguments the identifier, methodName, and params
 *    will all be NULL. The destructor for the TypeClass of the source
 *    variable will need to be invoked.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Delete::AST_Delete(AST_Expression* v){
  variableName = ((AST_Variable*)v)->name;
  variable = v;
}

AST_Delete::~AST_Delete(){
  delete variableName;
  delete variable;
}

void AST_Delete::dump(){
  cerr << "Delete" << endl;
  variable->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodInvoke Class
 *  Used to distinguish between Method/Constructor/Destructor
 *    0 = local method, 1 = variable method, -1 = SUPER method
 *  This means that the source (s, variable calling method) will not be
 *    null. The methodName (id) will be the name of the method being
 *    invoked. Params (p) could or could not be null depending on the
 *    method being called. The identifier will not matter for methods as
 *    it is used to distinguish between SUPER and THIS calls.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_MethodInvoke::AST_MethodInvoke(int i, AST_Expression* s, AST_Expression* id, AST_ArgumentsList* p){
  identifier = i;
  source     = s; // could be NULL
  methodName = ((AST_Variable*)id)->name;
  params     = p;
}

AST_MethodInvoke::~AST_MethodInvoke(){
  delete source;
  delete methodName;
  delete params;
}

void AST_MethodInvoke::dump(){
  cerr << "MethodInvoke\n";
  if( !identifier || identifier < 0 )
    cerr << methodName << "\n";
  else{
    source->dump();
    cerr << methodName << "\n";
  }
  params->dump();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodDeclarator Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_MethodDeclarator::AST_MethodDeclarator(AST_Expression* n, AST_ParameterList* p){
  methodName = ((AST_Variable*)(n))->name;
  if(p->getItem() != NULL)
    params = p;
}

AST_MethodDeclarator::~AST_MethodDeclarator(){
  delete methodName;
  delete params;
  delete heldType;
}

void AST_MethodDeclarator::dump(){
  cerr << "Method Declaration " << methodName << "\n";
  if( params != NULL )
    params->dump();
}

char* AST_MethodDeclarator::getName(){
  return methodName;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Method::AST_Method(Type* t, AST_Statement* id, AST_StatementList* b, int f){
  type = t;
  flag = f;
  declarator = ((AST_MethodDeclarator*)(id));
  if( b != NULL )
    body = ((AST_Block*)(b))->list;
  declarationType = 1;
}

AST_Method::~AST_Method(){
  delete type;
  delete declarator;
  delete body;
}

void AST_Method::dump(){
  cerr << "Method " << type->toString() << endl ;
  declarator->dump();
  body->dump();
}

void AST_Method::setOwner(char* n){
  owner = n;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Deref Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Deref::AST_Deref(AST_Expression* l) : AST_UnaryOperator(l){}

AST_Deref::~AST_Deref(){}

void AST_Deref::dump(){
  cerr << "Deref " << left->type->toString() << endl;
  left->dump();
}

