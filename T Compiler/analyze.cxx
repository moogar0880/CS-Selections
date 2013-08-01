/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST methods for semantic analysis
 * code gen is in encode.cxx
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;

#include "AST.h"
#include "SymbolTable.h"
#include "Type.h"

// global symbol table is in main.cxx
extern SymbolTable* symbolTable;
extern std::vector<AST_Class*> globalClassList;
// global type module is in main.cxx
extern TypeModule* types;
extern bool terminalErrors;
int whileLoopCount = 0;

AST_Node* AST_List::analyze(){
  if( ownerSet ){
    ((AST_FieldDeclaration*)(item))->setOwner(owner);
  }
  item = item->analyze();
  if (restOfList != NULL) restOfList = (AST_List*) restOfList->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_IntegerLiteral::analyze(){
  // nothing to be done; type already set
  return (AST_Node*) this;
}

AST_Node* AST_Variable::analyze(){
  Type* typeFromSymbolTable;
  char* searchName;
  if( maskSet )
    searchName = maskedName;
  else
    searchName = name;

  if(symbolTable->lookup(searchName, typeFromSymbolTable)){
    type = typeFromSymbolTable;
  }
  else{
    cerr << line << ": variable " << name << " is not declared!\n";
    type = types->errorType();
  }

  // always put Deref node on top of a variable
  AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
  ret->type = type;
  return (AST_Node*) ret;
}

AST_Node* AST_VariableList::analyze(Type* t){
  if( ownerSet ){
    ((AST_Variable*)(item))->setOwner(owner);
    if(!symbolTable->install(((AST_Variable*)(item))->maskedName, t)){
      cerr << line << ": duplicate declaration for " << t->toString() << " " <<
        ((AST_Variable*)(item))->maskedName << endl;
    }
  }
  else{
    if(!symbolTable->install(((AST_Variable*)(item))->name, t)){
      cerr << line << ": duplicate declaration for " << t->toString() << " " <<
        ((AST_Variable*)(item))->name << endl;
    }
  }
  ((AST_Variable*)(item))->analyze();
  if(restOfList != NULL)
    restOfList = (AST_List*)((AST_VariableList*) restOfList)->analyze(t);
  return (AST_Node*) this;
}

AST_Node* AST_MainFunction::analyze(){
  list->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_ExpressionStatement::analyze(){
  express->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_Declaration::analyze(){
  list->analyze(type);
  return (AST_Node*) this;
}

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
  // ie avoid a cascade of error messages
  if((lhs->type == types->errorType()) || (rhs->type == types->errorType())){
    return (AST_Node*) this;
  }
  // add a convert node if the types are not the same
  if (lhs->type != rhs->type){
    AST_Expression* newNode = new AST_Convert(rhs);

    newNode->type = lhs->type;

    rhs = newNode;
  }
  return (AST_Node*) this;
}

AST_Node* AST_Print::analyze(){
  // analyze the variable - a Deref node will be added
  var = (AST_Expression*)var->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_Divide::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();
  // check for error in either subtree to avoid cascade of errors
  if((left->type == types->errorType()) || (right->type == types->errorType()))
  {
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // divided
  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Divide::analyze: types are different\n";
    //exit(-1);
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_Multiply::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();
  // check for error in either subtree to avoid cascade of errors
  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // multiplied
  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Multiply::analyze: types are different\n";
    //exit(-1);
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_Add::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();
  // check for error in either subtree to avoid cascade of errors
  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // added
  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Add::analyze: types are different\n";
    //exit(-1);
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_Subtract::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  // check for error in either subtree to avoid cascade of errors
  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // subtracted
  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Subtract::analyze: types are different\n";
      //exit(-1);
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_LessThan::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  // check for error in either subtree to avoid cascade of errors
  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // compared with the Less Than operator
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

AST_Node* AST_GreaterThan::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  // check for error in either subtree to avoid cascade of errors
  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }
  // if types not the same then a compile-time error occurs, only ints can be
  // compared with the Greater Than operator
  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_LessThan::analyze: types are different\n";
    //exit(-1);
    type = types->errorType();
    return (AST_Node*) this;
  }
  // both types the same at this point
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_Negate::analyze(){
  // analyze subtree
  left = (AST_Expression*) left->analyze();
  // check for proper type
  if( left->type == types->intType() ){
    type = left->type;
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Negate::analyze: type is not int\n";
  //exit(-1);
  type = types->errorType();
  return (AST_Node*) this;
}

AST_Node* AST_Not::analyze(){
  // analyze subtree
  left = (AST_Expression*) left->analyze();
  // check for proper type
  if( left->type == types->intType() ){
    type = left->type;
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Not::analyze: type is not int\n";
  //exit(-1);
  type = types->errorType();
  return (AST_Node*) this;
}

AST_Node* AST_Equality::analyze(){
  // analyze both subtrees
  left = (AST_Expression*) left->analyze();
  right = (AST_Expression*) right->analyze();

  // check for error in either subtree to avoid cascade of errors
  if((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  // if types not the same then a compile time error occurs if an int is
  // compared with a reference or null type or if it is impossible to convert
  // the type of either operand to the type of the other by a casting
  // conversion
  if (left->type != right->type){
    if( left->type == types->intType() || right->type == types->intType() ){
      cerr << line << ": BUG in AST_Equality::analyze: types are different\n";
      //exit(-1);
      type = types->errorType();
      return (AST_Node*) this;
    }
    Type* lTypeFromSymbolTable;
    Type* rTypeFromSymbolTable;
    char* lName;
    char* rName;
    lName = ((AST_Variable*)(left))->name;
    rName = ((AST_Variable*)(right))->name;
    symbolTable->lookup(lName, lTypeFromSymbolTable);
    symbolTable->lookup(rName, rTypeFromSymbolTable);
    AST_Class* l;
    AST_Class* r;
    std::vector<AST_Class*>::iterator it;
    for(it = globalClassList.begin(); it != globalClassList.end(); ++it ){
      if( strcmp((*it)->getName(),lTypeFromSymbolTable->toString()) == 0 )
        l = (*it);
      if( strcmp((*it)->getName(),rTypeFromSymbolTable->toString()) == 0 )
        r = (*it);
    }

    // l is child of r
    if(((AST_Node*)this)->areComparable((AST_Node*)l,(AST_Node*)r)){
      AST_Expression* newNode = new AST_Convert(left);
      newNode->type = types->classType(rTypeFromSymbolTable->toString());
      left = newNode;
      type = left->type;
      return (AST_Node*) this;
    } // r is child of l
    else if(((AST_Node*)this)->areComparable((AST_Node*)r,(AST_Node*)l)){
      AST_Expression* newNode = new AST_Convert(right);
      newNode->type = types->classType(lTypeFromSymbolTable->toString());
      right = newNode;
      type = right->type;
      return (AST_Node*) this;
    } // l and r are not related
    else{
      cerr << line << ": BUG in AST_Equality::analyze: types are unrelated\n";
      type = types->errorType();
      return (AST_Node*) this;
    }
  }
  type = left->type;
  return (AST_Node*) this;
}

AST_Node* AST_Block::analyze(){
  if( list != NULL ){
    return list->analyze();
  }
  return NULL;
}

AST_Node* AST_IfThenElse::analyze(){
  condition->analyze();
  if( ifstat != NULL )
    ifstat->analyze();
  if( elstat != NULL )
    elstat->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_While::analyze(){
  whileLoopCount += 1;
  condition->analyze();
  if( stat != NULL )
    stat->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_Return::analyze(){
  if( var != NULL ){
    var->analyze();
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Return::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

AST_Node* AST_Break::analyze(){
  if( whileLoopCount > 0 ){
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Break::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

AST_Node* AST_Continue::analyze(){
  if( whileLoopCount > 0 ){
    return (AST_Node*) this;
  }
  cerr << line << ": BUG in AST_Continue::analyze: statement out of scope\n";
  terminalErrors = true;
  return NULL;
}

AST_Node* AST_CompilationUnit::analyze(){
  if( list != NULL ){
    list->analyze();
  }
  main->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_Cast::analyze(){
  //cerr << "AST_Cast::analyze() not yet implemented\n";
  return (AST_Node*) this;
}

AST_Node* AST_Class::analyze(){
  if( parent == NULL )
    parent = types->classType((char*)("Object"));
  ((AST_List*)(fields))->setOwner(name);
  ((AST_List*)(fields))->analyze();
  return (AST_Node*) this;
}

AST_Node* AST_FieldReference::analyze(){
  //cerr << "AST_FieldReference::analyze() not yet implemented\n";
  return (AST_Node*) this;
}

AST_Node* AST_FieldDeclaration::analyze(){
  if( ownerSet )
    list->setOwner(owner);
  list->analyze(type);
  return (AST_Node*) this;
}

AST_Node* AST_ClassInstance::analyze(){
  //cerr << "AST_ClassInstance::analyze() not yet implemented\n";
  return (AST_Node*) this;
}

AST_Node* AST_ArgumentsList::analyze(){
  return (AST_Node*) this;
}

AST_Node* AST_Null::analyze(){
  //cerr << "AST_Null::analyze() not yet implemented\n";
  // return NULL;
  return (AST_Node*) this;
}

AST_Node* AST_EmptyStatement::analyze(){
  return (AST_Node*) this;
}

AST_Node* AST_Convert::analyze(){
  // convert node is added in analyze but should never be analyzed itself
  cerr << line << ": BUG in AST_Convert::analyze: should never be called\n";
  return (AST_Node*) this;
}

AST_Node* AST_Deref::analyze(){
  // deref node is added in analyze but should never be analyzed itself
  cerr << line << ": BUG in AST_Deref::analyze: should never be called\n";
  return (AST_Node*) this;
}