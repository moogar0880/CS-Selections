/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST methods for semantic analysis
 * code gen is found in encode.cxx
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;

#include "AST.h"
#include "SymbolTable.h"
#include "Type.h"
#include "ScopeManager.h"

// global symbol table is in main.cxx
extern SymbolTable* symbolTable;
// global type module is in main.cxx
extern TypeModule* types;
// global scope manager is in main.cxx
extern ScopeManager* scopeManager;
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
  if(scopeManager->currentScope()->lookup(name, typeFromSymbolTable)){
    type = typeFromSymbolTable;
    if( type != types->intType() ){
      type = types->classType(type->toString());
    }
  }
  else if(scopeManager->in_method()){
    if( scopeManager->getMethod()->hasParam(name,typeFromSymbolTable) ){
      isParam = true;
      SymbolTableRecord* scan = scopeManager->getMethod()->exportAsSymbolTable()->head;
      int i = 1;
      while(scan != NULL){
        if(!strcmp(scan->name,name)){
          index = i;
          break;
        }
        scan = scan->next;
        i += 1;
      }
      type = typeFromSymbolTable;
      if( type != types->intType() ){
        type = types->classType(type->toString());
      }
    }
    else{
      if(scopeManager->up()->lookup(name, typeFromSymbolTable)){
        type = typeFromSymbolTable;
        if( type != types->intType() ){
          type = types->classType(type->toString());
        }
      }
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

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_VariableList semantic analysis
 *  If VariableList item Variable can be found in symbol table analyze it
 *  If VariableList item can not be found in symbol table print error
 *  If VariableList restOfList is not NULL recurse down the list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_VariableList::analyze(Type* t){
  if(!scopeManager->currentScope()->install(((AST_Variable*)(item))->name, t)){
    cerr << line << ": duplicate declaration for " << t->toString() << " " <<
      ((AST_Variable*)(item))->name << endl;
  }
  ((AST_Variable*)(item))->analyze();
  if(restOfList != NULL)
    restOfList = (AST_List*)((AST_VariableList*) restOfList)->analyze(t);
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MainFunction semantic analysis
 *  Analyze the MainFunction's StatementList
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_MainFunction::analyze(){
  scopeManager->setMain();
  list->analyze();
  scopeManager->clearMain();
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
  if( rhs->type == types->derefType() ){
    AST_Deref* dr = (AST_Deref*)rhs;
    rhs->type = dr->left->type;
  }
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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

  if((left->type == types->errorType()) || (right->type == types->errorType())){
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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

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
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

  if ((left->type == types->errorType()) || (right->type == types->errorType())){
    type = types->errorType();
    return (AST_Node*) this;
  }

  if(left->type != types->intType() || right->type != types->intType()){
    cerr << line << ": BUG in AST_Add::analyze: types are different - " << left->type->toString() << " & " << right->type->toString() <<  "\n";
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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

  if(right->type == types->derefType()){
    AST_Deref* dr = (AST_Deref*)right;
    right->type = dr->left->type;
  }

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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

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

  if(left->type == types->derefType()){
    AST_Deref* dl = (AST_Deref*)left;
    left->type = dl->left->type;
  }

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
  left  = (AST_Expression*) left->analyze();
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

  Type* leftType = left->type;
  if( leftType == types->derefType() )
    leftType = ((AST_Deref*)left)->left->type;
  Type* rightType = right->type;
  if( rightType == types->derefType() )
    rightType = ((AST_Deref*)right)->left->type;
  // if types not the same then a compile time error occurs if an int is
  // compared with a reference or null type or if it is impossible to convert
  // the type of either operand to the type of the other by a casting
  // conversion
    if (leftType != rightType){
      if( left->type == types->intType() || right->type == types->intType() ){
        cerr << line << ": BUG in AST_Equality::analyze: types are different\n";
        terminalErrors = true;
        type = types->errorType();
        return (AST_Node*) this;
      } // end int check

      // If one side is Null, the other is not
      if( leftType == types->nullType() || rightType == types->nullType() ){
        type = types->intType();
        return (AST_Node*) this;
      } // end Null check

      // class reference checks
      TypeClass *l, *r;
      l = (TypeClass*)leftType;
      r = (TypeClass*)rightType;

      // l is child of r
      TypeClass* scan = l;
      while( scan != NULL ){
        if( !strcmp(scan->toString(),r->toString()) ){
          AST_Expression* newNode = new AST_Cast(right,left);
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
        if( !strcmp(scan->toString(),l->toString()) ){
          AST_Expression* newNode = new AST_Cast(left,right);
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

  // at this point left and right are same type
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
  }
  if( scopeManager->inMain ){
    label = (char*)"main$exit";
    return (AST_Node*) this;
  }
  else if( scopeManager->in_method() ){
    char* l = new char[500];
    strcpy(l,scopeManager->getClass()->getName());
    strcat(l,"$");
    strcat(l,scopeManager->getMethod()->signatureString());
    strcat(l,"$exit");
    label = l;
    return (AST_Node*) this;
  }
  else{
    cerr << line << ": BUG in AST_Return::analyze: statement out of scope\n";
    terminalErrors = true;
    return NULL;
  }
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
  if( !types->createNewClassType((char*)"Object") ){
    int l = ((AST_Node*)this)->getLineNumber();
    cerr << l << ": Declaration of custom Object class is not allowed" << endl;
    terminalErrors = true;
  }
  //Hardcode Object Constructor
  AST_StatementList* objConst;
  objConst = new AST_StatementList(
    new AST_Method(NULL,
      new AST_MethodDeclarator(
        new AST_Variable((char*)"Object"),
          new AST_ParameterList(NULL,NULL)),
      new AST_StatementList(new AST_EmptyStatement(),NULL),1),NULL);
  //Hardcode Object Destructor
  AST_StatementList* objDest;
  objDest  = new AST_StatementList(
    new AST_Method(NULL,
      new AST_MethodDeclarator(
        new AST_Variable((char*)"Object"),
          new AST_ParameterList(NULL,NULL)),
      new AST_StatementList(new AST_EmptyStatement(),NULL),-1),NULL);
  //Hardcode Object Equals Method
  objConst->concat(objDest);
  AST_StatementList* objEq;
  objEq    = new AST_StatementList(
    //Method return type
    new AST_Method(types->intType(),
      new AST_MethodDeclarator(
        //Method name
        new AST_Variable((char*)"equals"),
          new AST_ParameterList(
            //Method Parameter type
            new AST_Parameter(types->classType((char*)"Object"),
                              //Method parameter name
                              new AST_Variable((char*)"obj")),
                              //Terminate ParamList
                              NULL)),
      //Method Body
      new AST_StatementList(
        new AST_IfThenElse(
          new AST_Equality(new AST_This((char*)"this"),new AST_Variable((char*)"obj")),
          new AST_Return(new AST_IntegerLiteral(1)),
          new AST_Return(new AST_IntegerLiteral(0))),
      NULL),0),NULL);

  objConst->concat(objEq);
  if( list != NULL ){
    list->concat(new AST_ClassList(new AST_Class(new AST_Variable((char*)"Object"),objConst, NULL),NULL));
  }
  else{
    list = new AST_ClassList(new AST_Class(new AST_Variable((char*)"Object"),objConst, NULL),NULL);
  }
  list->analyze();
  main->analyze();
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Cast semantic analysis
 * Not much is done during analysis. The types are checked at runtime
 *  using a RTS function named checkCast
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Cast::analyze(){
  castType = types->classType(((AST_Variable*)expr)->name);
  if( castType == NULL ){
    cerr << line << ": Error: Invalid cast type " <<
            ((AST_Variable*)expr)->name << " given" << endl;
    terminalErrors = true;
    type = types->errorType();
  }
  cast->analyze();
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
  scopeManager->setClass(name);
  TypeClass* t;
  // No declared Super class and not Object
  if( parent == NULL && strcmp(name, (char*)"Object")){
    t = types->classType(name);
    if( t != NULL )
      types->classType(name)->setParent(types->classType((char*)("Object")));
  }// No delcared Super class and is Object
  else if( parent == NULL && !strcmp(name, (char*)"Object")){
    t = types->classType(name);
  }// Has declared Super class, need to pull Super classType out of TypeModule
  else{
    types->classType(name)->setParent(types->classType(parent->toString()));
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
    if( !types->classType(name)->hasDeclaredDestructor ){
      //Build Default Destructor
    }

    if( !types->classType(name)->hasDeclaredConstructor ){
      //Build Default Constructor
    }
  }
  if( strcmp(name, (char*)"Object") && strcmp(types->classType(name)->getParent()->getName(), (char*)"Object") )
    types->classType(name)->inheritFields();
  scopeManager->clearClass();
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
  //cerr << "FieldRef type: " << type->toString() << ", owner: " << owner << ", variable: " << variable << endl;
  owner->analyze();

  if(owner->type == types->errorType()){
    type = types->errorType();
    return (AST_Node*) this;
  }
  if( owner->type == types->intType() ){
    cerr << "Error: int types do not contain values to be referenced" << endl;
    terminalErrors = true;
    type = types->errorType();
    return (AST_Node*) this;
  }
  if( owner->type == types->nullType() ){
    cerr << "Error: Null reference detected" << endl;
    terminalErrors = true;
    type = types->errorType();
    return (AST_Node*) this;
  }

  Type* t;
  if(!types->classType(owner->type->toString())->getSymbolTable()->lookup(variable,t)){
    cerr << "Error: instances of Class " << owner->type->toString() <<
            " do not contain a variable named " << variable << endl;
    terminalErrors = true;
    type = types->errorType();
    return (AST_Node*) this;
  }
    type = t;

  AST_Expression* ret = (AST_Expression*) new AST_Deref(this);
  //ret->type = types->derefType();
  ret->type = type;
  return (AST_Node*) ret;
  //return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldDeclaration semantic analysis
 *  Call analyze on the field declaration list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_FieldDeclaration::analyze(){
  list->analyze(type);
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
 * AST_Delete semantic analysis
 *  Need to type check variableName. If not reference or is null, an error
 *    is reported. Otherwise a call to variableName's TypeClass's default
 *    destructor is made.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_Delete::analyze(){
  Type* typeFromSymbolTable;
  variable = (AST_Expression*)variable->analyze();
  if( variable->type == types->errorType() ){
    type = types->errorType();
    return (AST_Node*) this;
  }
  if(scopeManager->currentScope()->lookup(variableName, typeFromSymbolTable)){
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
    cerr << line << ": 762 variable " << variableName << " is not declared!\n";
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
  //cerr << declarator->methodName << "\n";
  body->analyze();
  mungedName = declarator->heldType->getMunged(owner);
  scopeManager->clearMethod();
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
    scopeManager->setMethod(tm);
    if( params != NULL )
      params->analyze(tm);
    tm->owner = scopeManager->getClass()->getName();
    tc->add(methodName,(TypeMethod*)tm);
  }
  else if( f > 0 ){ // Constructor
    tm = new TypeMethod(methodName, ret, true, false);
    scopeManager->setMethod(tm);
    if( params != NULL )
      params->analyze(tm);
    tc->add(methodName,(TypeMethod*)tm);
  }
  else{ // Destructor
    tm = new TypeMethod(methodName, ret, false, true);
    scopeManager->setMethod(tm);
    // params will be null for destructors
    tc->add(methodName,(TypeMethod*)tm);
  }
  heldType = tm;
  return (AST_Node*) this;
}

// This is only here to make the compiler happy, it will never get called
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
    cerr << "Error: Invalid parameter given\n";
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
 *  CASE 0: A local method is being called from within another method.
 *    This means that we must be within a Class, so the scopeManager
 *    pulls in the currentClass and checks it's MethodTable for the given
 *    method.
 *  Case 1: A variable calls one of it's methods. Because method
 *    invocations like this can occur in either methods or the main block
 *    we rely on the scopeManager to pull in the correct SymbolTable to
 *    search for the method
 *  Case 2: A call to one of 'SUPERS' methods is made. Because the main
 *    block has no parent type, this call can only come from within a
 *    Class's method. The current class is pulled in, we re-direct to
 *    the current class's parent, and search it's SymbolTable for the
 *    provided method
 *
 *  NOTE: The type of a MethodInvoke is the return type of the method
 *    being called
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_MethodInvoke::analyze(){
  SymbolTableRecord* caller = scopeManager->currentScope()->head;
  while(caller != NULL){
    if(!strcmp(((AST_Variable*)source)->name, caller->name)){
      scopeManager->setCaller(caller);
      break;
    }
    caller = caller->next;
  }

  if( identifier == 0 ){ // Local method invoke source will be NULL
    callingType = types->classType(scopeManager->getClass()->getName());
    TypeMethod* search = new TypeMethod(methodName,types->nullType(), false, false);
    char* searchSignature;
    searchSignature = new char[500];
    strcpy(searchSignature,methodName);
    AST_Expression* arg;
    if( params != NULL ){
      arg = (AST_Expression*)params->getItem();
      AST_ArgumentsList* l = params;
      while(arg != NULL){
        ((AST_Variable*)arg)->analyze();
        search->addParam(((AST_Variable*)arg)->name, ((AST_Variable*)arg)->type);
        if(l != NULL){
          arg = (AST_Expression*)l->getItem();
            if( arg->type != types->intType() ){
              if(!scopeManager->currentScope()->lookup(((AST_Variable*)arg)->name, arg->type)){
                cerr << line << ": variable " << ((AST_Variable*)source)->name << " does not exist" << endl;
                type = types->errorType();
                terminalErrors = true;
              }
            }
            strcat(searchSignature,"_");
            strcat(searchSignature,((AST_Variable*)arg)->type->toString());
            l = (AST_ArgumentsList*)l->getRestOfList();
        }
        else{
          arg = NULL;
        }
      }
    }
    sig = searchSignature;
    if(!scopeManager->getClass()->getSymbolTable()->lookupMethod(search,type)){
      cerr << line << ": method " << scopeManager->getClass()->getName() << "." << methodName << " does not exist" << endl;
      type = types->errorType();
      terminalErrors = true;
    }
  }
  else if( identifier > 0 ){ // Variable's method invoke
    if(!scopeManager->currentScope()->lookup(((AST_Variable*)source)->name, type)){
      cerr << line << ": variable " << ((AST_Variable*)source)->name << " does not exist" << endl;
      type = types->errorType();
      terminalErrors = true;
    }

    if( type == types->intType() || type == types->nullType() ){
      cerr << line << ": variable " << ((AST_Variable*)source)->name <<
            " can not have methods" << endl;
      type = types->errorType();
      terminalErrors = true;
    }
    else{
      // May need to scan up into Parent's methods if methods haven't been
      // inherited yet
      TypeClass* scan = types->classType(type->toString());
      callingType = scan;
      // Build signature to search for
      char* searchSignature;
      searchSignature = new char[500];
      strcpy(searchSignature,methodName);
      AST_Expression* arg;
      if( params != NULL ){
        arg  = (AST_Expression*)params->getItem();
        AST_ArgumentsList* l = params;
        while(arg != NULL){
          if(l != NULL){
            arg = (AST_Expression*)l->getItem();
            if( arg->type != types->intType() ){
              if(!scopeManager->currentScope()->lookup(((AST_Variable*)arg)->name, arg->type)){
                cerr << line << ": variable " << ((AST_Variable*)source)->name << " does not exist" << endl;
                type = types->errorType();
                terminalErrors = true;
              }
            }
            strcat(searchSignature,"_");
            strcat(searchSignature,((AST_Variable*)arg)->type->toString());
            l = (AST_ArgumentsList*)l->getRestOfList();
          }
          else{
            arg = NULL;
          }
        }
      } // End signature build
      sig = searchSignature;
      while( scan != NULL ){
        SymbolTableRecord* str = scan->getSymbolTable()->methodHead;
        while( str != NULL ){
          if( !strcmp( ((TypeMethod*)str->type)->signatureString(), searchSignature ) ){
            type = ((TypeMethod*)str->type)->getReturnType();
          }
          str = str->next;
        }
        scan = scan->getParent();
      }
    }
    source = (AST_Expression*)source->analyze();
    if( source->type == types->errorType() ){
      type = types->errorType();
      return (AST_Node*) this;
    }
  }
  else{ // Super method invoke source will be NULL
    TypeMethod* search = new TypeMethod(methodName,types->nullType(), false, false);
    callingType = types->classType(scopeManager->getClass()->getParent()->getName());
    char* searchSignature;
    searchSignature = new char[500];
    strcpy(searchSignature,methodName);
    AST_Expression* arg;
    if( params != NULL ){
      arg = (AST_Expression*)params->getItem();
      AST_ArgumentsList* l = params;
      while(arg != NULL){
        ((AST_Variable*)arg)->analyze();
        search->addParam(((AST_Variable*)arg)->name, ((AST_Variable*)arg)->type);
        if(l != NULL){
          arg = (AST_Expression*)l->getItem();
            if( arg->type != types->intType() ){
              if(!scopeManager->currentScope()->lookup(((AST_Variable*)arg)->name, arg->type)){
                cerr << line << ": variable " << ((AST_Variable*)source)->name << " does not exist" << endl;
                type = types->errorType();
                terminalErrors = true;
              }
            }
            strcat(searchSignature,"_");
            strcat(searchSignature,((AST_Variable*)arg)->type->toString());
            l = (AST_ArgumentsList*)l->getRestOfList();
        }
        else{
          arg = NULL;
        }
      }
    }
    sig = searchSignature;
    if(!scopeManager->getClass()->getParent()->getSymbolTable()->lookupMethod(search,type)){
      cerr << line << ": method " << scopeManager->getClass()->getParent()->getName() << "." << methodName << " does not exist" << endl;
      type = types->errorType();
      terminalErrors = true;
    }
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ConstructorInvoke semantic analysis
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_ConstructorInvoke::analyze(){
  TypeMethod* search = new TypeMethod(scopeManager->getClass()->getName(),types->nullType(), true, false);
  AST_Expression* arg = (AST_Expression*)params->getItem();
  AST_ArgumentsList* l = params;
  while(arg != NULL){
    ((AST_Variable*)arg)->analyze();
    search->addParam(((AST_Variable*)arg)->name, ((AST_Variable*)arg)->type);
    if(l != NULL){
      arg = (AST_Expression*)l->getItem();
      l = (AST_ArgumentsList*)l->getRestOfList();
    }
    else{
      arg = NULL;
    }
  }
  Type* junk;
  if( !identifier ){ // A call to a THIS constructor
    if(!scopeManager->getClass()->getSymbolTable()->lookupMethod(search,junk)){
      cerr << line << ": Constructor " << scopeManager->getClass()->getName() << " does not exist" << endl;
      terminalErrors = true;
    }
  }
  else{ // A call to a SUPER constructor
    if( scopeManager->getClass()->getParent() != NULL ){
      if(!scopeManager->getClass()->getParent()->getSymbolTable()->lookupMethod(search,junk)){
        cerr << line << ": Constructor " << scopeManager->getClass()->getParent()->getName() << " does not exist" << endl;
        terminalErrors = true;
      }
    }
    else{
      cerr << line << ": SUPER keyword can not be used within Object class"<< endl;
      terminalErrors = true;
    }
  }
  return (AST_Node*) this;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_This semantic analysis
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
AST_Node* AST_This::analyze(){
  if( !scopeManager->in_method() ){
    cerr << line << ": This keyword can not be used outside of method body" << endl;
    terminalErrors = true;
    type = types->errorType();
  }
  /*else{
    type = scopeManager->getCaller()->type;
  }*/
    type = scopeManager->getClass();
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