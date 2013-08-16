/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST methods for semantic analysis
 * code gen is found in encode.cxx
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;

#include "AST.h"
#include "SymbolTable.h"
#include "Type.h"

// global symbol table is in main.cxx
extern SymbolTable* symbolTable;
// global type module is in main.cxx
extern TypeModule* types;
extern bool terminalErrors;
int whileLoopCount = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_List semantic analysis
 *  Call analyze on the list item, if the rest of the list is not NULL
 *  recurse down and call analyze on the rest of the list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_List::analyze(){
  item = item->analyze();
  if (restOfList != NULL) restOfList = (AST_List*) restOfList->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IntegerLiteral semantic analysis
 *  Nothing to be done for Integer Literal analysis, type already set
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_IntegerLiteral::analyze(){
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Variable semantic analysis
 *  Do symbol table lookup for variable name and pull type out if found
 *  If not found in symbol table set to errorType
 *
 *  Note: Always put Deref node on top of a variable
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Variable::analyze(){
  Type* typeFromSymbolTable;
  if(symbolTable->lookup(name, typeFromSymbolTable)){
    type = typeFromSymbolTable;
    if( type != types->intType() ){
      type = types->classType(type->toString());
    }
  }
  else{
    cerr << line << ": variable " << name << " is not declared!\n";
    type = types->errorType();
    terminalErrors = true;
  }
  //Set Deref node
  AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
  ret->type = types->derefType();
  return (AST_Node*) ret;
}

//Class field analyzer
AST_Node* AST_Variable::analyze(SymbolTable* s){
  Type* typeFromSymbolTable;
  if(s->lookup(name, typeFromSymbolTable)){
    type = typeFromSymbolTable;
    if( type != types->intType() ){
      type = types->classType(type->toString());
    }
  }
  else{
    cerr << line << ": variable " << name << " is not declared!\n";
    type = types->errorType();
    terminalErrors = true;
  }
  //Set Deref node
  AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
  ret->type = types->derefType();
  return (AST_Node*) ret;
}

/*AST_Node* AST_Variable::analyze(std::vector<SymbolTableRecord*> s){
  std::vector<SymbolTableRecord*>::iterator it;
  for( it = s.begin(); it != s.end(); ++s){
    if( !strcmp((*it)->name, name) ){
      if( (*it)->type != types->intType() )
        type = types->classType((*it)->type->toString());
      else
        type = types->intType();
      //Set Deref node
      AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
      ret->type = types->derefType();
      return (AST_Node*) ret;
    }
  }
  cerr << line << ": variable " << name << " is not declared!\n";
  type = types->errorType();
  terminalErrors = true;
  return (AST_Node*) this;
}*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_VariableList semantic analysis
 *  If VariableList item Variable can be found in symbol table analyze it
 *  If VariableList item can not be found in symbol table print error
 *  If VariableList restOfList is not NULL recurse down the list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_VariableList::analyze(Type* t){
  if(!symbolTable->install(((AST_Variable*)(item))->name, t)){
    cerr << line << ": duplicate declaration for " << t->toString() << " " <<
      ((AST_Variable*)(item))->name << endl;
  }
  ((AST_Variable*)(item))->analyze();
  if(restOfList != NULL)
    restOfList = (AST_List*)((AST_VariableList*) restOfList)->analyze(t);
  return (AST_Node*) this;
}
//Special VariableList analysis for Class fields
AST_Node* AST_VariableList::analyze(Type* t, SymbolTable* s){
  if(!s->install(((AST_Variable*)(item))->name, t)){
    cerr << line << ": custom symbol table duplicate declaration for " << t->toString() << " " <<
      ((AST_Variable*)(item))->name << endl;
  }
  ((AST_Variable*)(item))->analyze(s);
  if(restOfList != NULL)
    restOfList = (AST_List*)((AST_VariableList*) restOfList)->analyze(t,s);
  return (AST_Node*) this;
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MainFunction semantic analysis
 *  Analyze the MainFunction's StatementList
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_MainFunction::analyze(){
  list->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ExpressionStatement semantic analysis
 *  Analyze the ExpressionStatement's expression
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ExpressionStatement::analyze(){
  express->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Declaration semantic analysis
 *  Analyze the Declaration's AST_Variable
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Declaration::analyze(){
  list->analyze(type);
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Assignment semantic analysis
 *  Analyze the lhs variable (Deref node will be on top so need to strip)
 *  Analyze the rhs variable, if no errors are found return or try to
 *    convert types
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Assignment::analyze(){
  // analyze the lhs variable - a Deref node will be put on top
  AST_Deref* deref = (AST_Deref*) lhs->analyze();
  // strip off the Deref node
  lhs = deref->left;
  deref->left = NULL;
  delete deref;
  // analyze the expression
  rhs = (AST_Expression*) rhs->analyze();
  // check if error was detected in one of the subtrees
  if((lhs->type == types->errorType()) || (rhs->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // add a convert node if the types are not the same
  if(lhs->type != rhs->type){
    if( lhs->type == types->intType() || rhs->type == types->intType() ){
      cerr << line << ": BUG in AST_Assignment::analyze: can't assign " <<
        rhs->type->toString() << " to " << lhs->type->toString() << "\n";
      type = types->errorType();
      terminalErrors = true;
    }
    else if(lhs->type != types->nullType() && rhs->type == types->nullType()){
      type = types->nullType();
    }
    else if( lhs->type == types->nullType() ){
      cerr << line <<
        ": BUG in AST_Assignment::analyze: can't assign to Null instance\n";
      type = types->errorType();
      terminalErrors = true;
    }
    //Need to check for possible widening
    AST_Expression* newNode = new AST_Convert(rhs);

    newNode->type = lhs->type;
    rhs = newNode;
  }
  else{
    type = rhs->type;
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Print semantic analysis
 *  Analyze the variable to printed, a Deref node will be added
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Print::analyze(){
  var = (AST_Expression*)var->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Divide semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be divided
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Divide::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if((left->type == types->errorType()) || (right->type == types->errorType()))
  {
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Divide::analyze: types are different\n";
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Multiply semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be multiplied
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Multiply::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Multiply::analyze: types are different\n";
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Add semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be added
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Add::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    // strip off the Deref node
    left = dl->left;
    dl->left = NULL;
    delete dl;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    // strip off the Deref node
    right = dr->left;
    dr->left = NULL;
    delete dr;
  }

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Add::analyze: types are different\n";
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Subtract semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be subtracted
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Subtract::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Subtract::analyze: types are different\n";
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_LessThan semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be compared with a less than operator
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_LessThan::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_LessThan::analyze: types are different\n";
    terminalErrors = true;
    type = types->errorType();
    return (AST_Node*) this;
  }

  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_GreaterThan semantic analysis
 *  Analyze left and right subtrees and check results for errors
 *
 *  Note: If types aren't the same then a compile-time error occurs,
 *    because only IntTypes can be compared with a greater than operator
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_GreaterThan::analyze(){
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_LessThan::analyze: types are different\n";
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Negate semantic analysis
 *  Analyze left subtree and check result for errors
 *
 *  Note: If type is not IntType then a compile-time error occurs,
 *    because only IntTypes can be negated
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Negate::analyze(){
  left = (AST_Expression*) left->analyze();
  if( left->type == types->intType() ){
    type = left->type;
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Negate::analyze: type is not int\n";
  terminalErrors = true;
  type = types->errorType();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Not semantic analysis
 *  Analyze left subtree and check result for errors
 *
 *  Note: If type is not IntType then a compile-time error occurs,
 *    because only IntTypes can be used with the not operator
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Not::analyze(){
  left = (AST_Expression*) left->analyze();
  if( left->type == types->intType() ){
    type = left->type;
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Not::analyze: type is not int\n";
  terminalErrors = true;
  type = types->errorType();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Equality semantic analysis
 *  Analyze left and right subtrees and check for errors
 *
 *  Pick up any errros and pass them up accordingly
 *
 *  Check if both sides are NULL
 *
 *  If left and right types are different check to see if they can be
 *    converted
 *
 *  If types are the same return the left side's type
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Equality::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  // check for error in either subtree to avoid cascade of errors
  if((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  // check to see if both sides are null
  if( left->type == types->nullType() && right->type == types->nullType() ){
    type = types->intType();
    return (AST_Node*) this;
  }

  // if types not the same then a compile time error occurs if an int is
  // compared with a reference or null type or if it is impossible to convert
  // the type of either operand to the type of the other by a casting
  // conversion
  if (left->type != right->type){
    if( left->type == types->intType() || right->type == types->intType() ){
      cerr << line << ": BUG in AST_Equality::analyze: types are different\n";
      terminalErrors = true;
      type = types->errorType();
      return (AST_Node*) this;
    } // end int check

    // class reference checks
    TypeClass* l = (TypeClass*)left->type;
    TypeClass* r = (TypeClass*)right->type;

    // l is child of r
    TypeClass* scan = l;
    while( scan != NULL ){
      if( !strcmp(scan->getName(),r->getName()) ){
        AST_Expression* newNode = new AST_Convert(left);
        newNode->type = types->classType(r->toString());
        left = newNode;
        type = types->intType();
        return (AST_Node*) this;
      }
      scan = scan->getParent();
    }

    // r is child of l
    scan = r;
    while( scan != NULL ){
      if( !strcmp(scan->getName(),l->getName()) ){
        AST_Expression* newNode = new AST_Convert(right);
        newNode->type = types->classType(l->toString());
        right = newNode;
        type = types->intType();
        return (AST_Node*) this;
      }
      scan = scan->getParent();
    }

    // l and r are not related
    cerr << line << ": BUG in AST_Equality::analyze: types are unrelated\n";
    type = types->errorType();
    return (AST_Node*) this;
  }

  // left and right are same type
  type = types->intType();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Block semantic analysis
 *  If Block not empty (ie, list not null) then analyze the list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Block::analyze(){
  if( list != NULL ){
    return list->analyze();
  }
  return NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IfThenElse semantic analysis
 *  Analyze the condition statement, if the if statement is not null then
 *    analyze the body of the if statement and if the else statement is
 *    not null then analyze the body of the else statement
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_IfThenElse::analyze(){
  condition->analyze();
  if( ifstat != NULL )
    ifstat->analyze();
  if( elstat != NULL )
    elstat->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_While semantic analysis
 *  Increment the while loop counter and analyze the while loop condition
 *  If the while loop body is not null analyze it
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_While::analyze(){
  whileLoopCount += 1;
  condition->analyze();
  if( stat != NULL )
    stat->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Return semantic analysis
 *  Analyze the variable to be returned, if the variable is Null issue an
 *    error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Return::analyze(){
  if( var != NULL ){
    var->analyze();
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Return::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Break semantic analysis
 *  Check that there is a while loop to break out of, if not issue an
 *    error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Break::analyze(){
  if( whileLoopCount > 0 ){
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Break::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Continue semantic analysis
 *  Check that there is a while loop to run continue on, if not issue an
 *    error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Continue::analyze(){
  if( whileLoopCount > 0 ){
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Continue::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_CompilationUnit semantic analysis
 *  If there is a list of Classes, analyze it then analyze the main
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_CompilationUnit::analyze(){
  if( list != NULL ){
    list->analyze();
  }
  main->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Cast semantic analysis
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Cast::analyze(){
  cerr << "AST_Cast::analyze() not yet implemented\n";
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Class semantic analysis
 *  First: Check if declared parent is Null, if it is set parent to
 *    Object class, otherwise sent to declared parent
 *  Second: Go through statement list and add all fields to TypeClass's
 *    symbol table
 *  Third: Finally, analyze all of the fields
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Class::analyze(){
  TypeClass* t;
  if( parent == NULL ){
    t = (TypeClass*)(types->classType(name));
    if( t != NULL )
      t->setParent(types->classType((char*)("Object")));
  }
  else{
    t = (TypeClass*)(types->classType(name));
    if( t != NULL )
      t->setParent(parent);
  }
  //Add fields to symbolTable
  AST_Statement* scan;
  AST_StatementList* rol;
  if( fields != NULL ){
    scan = (AST_Statement*)(fields->getItem());
    rol  = (AST_StatementList*)(fields->getRestOfList());
  }
  while(scan != NULL){
    if( !scan->declarationType ){ //Field
      ((AST_FieldDeclaration*)(scan))->setOwner(name);
    }
    else{ //Method
      ((AST_Method*)scan)->setOwner(name);
    }
    if( rol != NULL ){
      scan = (AST_Statement*)(rol->getItem());
      rol  = (AST_StatementList*)(rol->getRestOfList());
    }
    else
      break;
  }
  ((AST_List*)(fields))->analyze();
  if( parent != NULL ){ // Don't need to change Object
    if( !t->hasDeclaredDestructor ){
      //Build Default Destructor
    }

    if( !t->hasDeclaredConstructor ){
      //Build Default
    }
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldReference semantic analysis
 *  If the left hand side (owner) can be found in the symbol table check
 *    the returned TypeClass's symbol table for the right hand side. If
 *    neither can be found and error is issued
 *  Note: A Deref node is always put on top of a field reference
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_FieldReference::analyze(){
  symbolTable->lookup(owner, type);
  if( types->classType(((TypeClass*)(type))->toString()) == NULL ){
    cerr << "Bad cast: " << ((TypeClass*)(type))->toString() << endl;
    terminalErrors = true;
    type = types->errorType();
  }
  else{
    char* name = ((TypeClass*)(type))->toString();
    if( !types->classType(name)->getItem(variable, type) ){
      terminalErrors = true;
      type = types->errorType();
    }
  }

  AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
  ret->type = types->derefType();
  return (AST_Node*) ret;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldDeclaration semantic analysis
 *  Call analyze on the field declaration list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_FieldDeclaration::analyze(){
  list->analyze(type, types->classType(owner)->getSymbolTable());
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassInstance semantic analysis
 *  If a TypeClass with the name of type can not be found issue an error,
 *    otherwise set type to returned TypeClass
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ClassInstance::analyze(){
  if( types->classType(type->toString()) == NULL ){
    cerr << line << ": BUG in AST_ClassInstance::analyze: class " <<
      type->toString() << " does not exist\n";
    type = types->errorType();
    terminalErrors = true;
  }
  else{
    type = types->classType(type->toString());
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ArgumentsList semantic analysis
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ArgumentsList::analyze(){
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Null semantic analysis
 *  Set type to nullType
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Null::analyze(){
  type = types->nullType();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_EmptyStatement semantic analysis
 *  The name says it all
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_EmptyStatement::analyze(){
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Convert semantic analysis
 *  These nodes are added in analyze but should never actually be analyzed
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Convert::analyze(){
  cerr << line << ": BUG in AST_Convert::analyze: should never be called\n";
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Delete semantic analysis
 *  Need to type check variableName. If not reference or is null, an error
 *    is reported. Otherwise a call to variableName's TypeClass's default
 *    destructor is made.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Delete::analyze(){
  Type* typeFromSymbolTable;
  if(symbolTable->lookup(variableName, typeFromSymbolTable)){
    if( typeFromSymbolTable == types->intType() ){
      cerr << line << ": Can not delete int values" << endl;
      terminalErrors = true;
    }
    else if( typeFromSymbolTable == types->nullType() ){
      cerr << line << ": Can not delete null references" << endl;
      terminalErrors = true;
    }
  }
  else{
    cerr << line << ": variable " << variableName << " is not declared!\n";
    terminalErrors = true;
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method semantic analysis
 *  Analyze the declarator in relation to the class who owns this method
 *    and then proceed to analyze the body of the method
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Method::analyze(){
  TypeClass* owningClass = types->classType(owner);
  declarator->analyze(type, owningClass, flag);
  body->analyze();
  mungedName = declarator->heldType->toVMTString(owner);
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodDeclarator semantic analysis
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_MethodDeclarator::analyze(Type* ret, TypeClass* tc, int f){
  TypeMethod* tm = NULL;
  if( !f ){
    tm = new TypeMethod(methodName, ret, false, false);
    if( params != NULL )
      params->analyze(tm);
    tc->add(methodName,tm);
  }
  else if( f > 0 ){ // Constructor
    tm = new TypeMethod(methodName, ret, true, false);
    if( params != NULL )
      params->analyze(tm);
    tc->add(methodName,tm);
  }
  else{ // Destructor
    tm = new TypeMethod(methodName, ret, false, true);
    // params will be null for destructors
    tc->add(methodName,tm);
  }
  heldType = tm;
  return (AST_Node*) this;
}

AST_Node* AST_MethodDeclarator::analyze(){
  return NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Parameter semantic analysis
 *  Just check for valid type
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Parameter::analyze(){
  if( type == types->errorType() || type == types->nullType() || type == types->noType()){
    type = types->errorType();
    terminalErrors = true;
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ParameterList semantic analysis
 *  Check parameter names to catch duplicate names, if duplicate found
 *    handle with an error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ParameterList::analyze(TypeMethod* tm){
  if( !tm->addParam(((AST_Parameter*)(item))->getName(),((AST_Parameter*)(item))->getType())){
    cerr << line << ": duplicate parameter declaration for " << ((AST_Parameter*)(item))->getName() << endl;
    terminalErrors = true;
  }
  ((AST_Parameter*)(item))->analyze();
  if(restOfList != NULL)
    restOfList = (AST_List*)((AST_ParameterList*) restOfList)->analyze(tm);
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodInvoke semantic analysis
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_MethodInvoke::analyze(){
  if( !identifier ){ // Local method invoke
    //source will be NULL
  }
  else if( identifier > 0 ){ // Variable's method invoke
    ((AST_Variable*)source)->analyze();
    // type =
  }
  else{ // Super method invoke
    //source will be NULL
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ConstructorInvoke semantic analysis
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ConstructorInvoke::analyze(){
  cerr << "AST_ConstructorInvoke::analyze not yet implemented" << endl;
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Deref semantic analysis
 *  These nodes are added in analyze but should never actually be analyzed
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Deref::analyze(){
  cerr << line << ": BUG in AST_Deref::analyze: should never be called\n";
  return (AST_Node*) this;
}