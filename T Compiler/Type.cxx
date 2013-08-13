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
 * Deref Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeDeref::TypeDeref(){}

TypeDeref::~TypeDeref(){}

char* TypeDeref::toString(){
  return (char *) "deref";
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
  classTable = new SymbolTable();
}

TypeClass::~TypeClass(){
  delete name;
  delete parent;
  delete classTable;
}

char* TypeClass::getName(){
  return name;
}

bool TypeClass::getItem(char* n, Type*& type){
  return classTable->lookup(n,type);
}

void TypeClass::add(char* n, Type* type){
  classTable->install(n,type);
}

void TypeClass::setParent(Type* p){
  parent = p;
}

TypeClass* TypeClass::getParent(){
  return (TypeClass*)parent;
}

SymbolTable* TypeClass::getSymbolTable(){
  return classTable;
}

char* TypeClass::toString(){
  return getName();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Method Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeMethod::TypeMethod(char* n, Type* o, bool cons, bool dest){
  name  = n;
  owner = o;
  symbolTable = new SymbolTable();
  isConstructor = cons;
  isDestructor  = dest;
}

TypeMethod::~TypeMethod(){
  delete name;
  delete owner;
  delete symbolTable;
}

char* TypeMethod::getName(){
  return name;
}

bool TypeMethod::hasParam(char* n){
  std::vector<SymbolTableRecord*>::iterator it;
  for( it = parameters.begin(); it != parameters.end(); ++it){
    if( strcmp((*it)->name,n) == 0 )
      return false;
  }
  return true;
}

bool TypeMethod::addParam(char* n, Type* type){
  if( !hasParam(n) ){
    parameters.push_back(new SymbolTableRecord(n,type));
    return true;
  }
  return false; //Can not have parameters with the same name
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
  derefTypeInternal = (Type*) new TypeDeref();
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
Type* TypeModule::createNewClassType(char * n){
  if( containsClassType(n) ){ //compile time error
    return NULL;
  }
  else{
    TypeClass* t = new TypeClass(n);
    classTypes.push_back(t);
    return t;
  }
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

Type* TypeModule::derefType(){
  return derefTypeInternal;
}

TypeClass* TypeModule::classType(char* name){
  std::vector<TypeClass*>::iterator it;
  for(it = classTypes.begin(); it != classTypes.end(); ++it ){
    if( strcmp((*it)->getName(),name) == 0 ){
      return *it;
    }
  }
  return NULL;
}

std::vector<TypeClass*> TypeModule::getClassList(){
  return classTypes;
}