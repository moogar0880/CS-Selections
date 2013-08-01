/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Type representation for T language
 *
 * NOTE: Don't use Type methods directly. Rather use indirectly via
 *       TypeModule. See Type.h for more information.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;
#include <stdio.h>
#include "SymbolTable.h"
#include "Type.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Abstract Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
Type::Type(){}

Type::~Type(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * None Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeNone::TypeNone(){}

TypeNone::~TypeNone(){}

char* TypeNone::toString(){
  return (char *) "<no type>";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Error Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeError::TypeError(){}

TypeError::~TypeError(){}

char* TypeError::toString(){
  return (char *) "<error type>";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Int Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeInt::TypeInt(){}

TypeInt::~TypeInt(){}

char* TypeInt::toString(){
  return (char *) "int";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Null Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeNull::TypeNull(){}

TypeNull::~TypeNull(){}

char* TypeNull::toString(){
  return (char *) "null";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeClass::TypeClass(char* n){
  name = n;
  parent = NULL;
  symbolTable = new SymbolTable();
}

TypeClass::~TypeClass(){
  delete name;
  delete parent;
  delete symbolTable;
}

char* TypeClass::getName(){
  return name;
}

bool TypeClass::getItem(char* n, Type* type){
  return symbolTable->lookup(n,type);
}

void TypeClass::add(char* n, Type* type){
  symbolTable->install(n,type);
}

void TypeClass::setParent(Type* p){
  parent = p;
}

char* TypeClass::toString(){
  return getName();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Method Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeMethod::TypeMethod(char* n, Type* o){
  name = n;
  owner = o;
  symbolTable = new SymbolTable();
}

TypeMethod::~TypeMethod(){
  delete name;
  delete owner;
  delete symbolTable;
}

char* TypeMethod::getName(){
  return name;
}

bool TypeMethod::getItem(char* n, Type* type){
  return symbolTable->lookup(n,type);
}

void TypeMethod::add(char* n, Type* type){
  symbolTable->install(n,type);
}

char* TypeMethod::toString(){
  return getName();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * TypeModule Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeModule::TypeModule(){
  intTypeInternal = (Type*) new TypeInt();
  errorTypeInternal = (Type*) new TypeError();
  noTypeInternal = (Type*) new TypeNone();
  nullTypeInternal = (Type*) new TypeNull();
}

//Search TypeModule and determine if a class with name n exists
bool TypeModule::containsClassType(char* n){
  std::vector<TypeClass*>::iterator it;
  for(it = classTypes.begin(); it != classTypes.end(); ++it ){
    if( strcmp((*it)->getName(),n) == 0 )
      return true; //if class type already exists
  }
  return false;
}

// Create new ClassType with name n
bool TypeModule::createNewClassType(char * n){
  if( containsClassType(n) ){ //compile time error
    return false;
  }
  else
    classTypes.push_back((TypeClass*) new TypeClass(n));
  return true;
}

TypeModule::~TypeModule(){
  delete intTypeInternal;
  delete errorTypeInternal;
  delete noTypeInternal;
  classTypes.erase(classTypes.begin(), classTypes.end());
}

Type* TypeModule::intType(){
  return intTypeInternal;
}

Type* TypeModule::errorType(){
  return errorTypeInternal;
}

Type* TypeModule::noType(){
  return noTypeInternal;
}

Type* TypeModule::nullType(){
  return nullTypeInternal;
}

Type* TypeModule::classType(char* name){
  std::vector<TypeClass*>::iterator it;
  // cerr << "\niterating over ClassTypes\n-------------------------\n";
  for(it = classTypes.begin(); it != classTypes.end(); ++it ){
    // cerr << "iterator = " << (*it)->getName() << ", name = " << name << endl;
    if( strcmp((*it)->getName(),name) == 0 ){
      // cerr << "-------------------------\n";
      return (Type*)(*it);
    }
  }
  // cerr << "-------------------------\n\n";
  return NULL;
}