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
extern bool terminalErrors;

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
  hasDeclaredDestructor = false;
  hasDeclaredConstructor = false;
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
  if( !strcmp(type->toString(), "method") ){
    if( ((TypeMethod*)type)->isDestructor && hasDeclaredDestructor ){
      cerr << "ERROR: Multiple " << n << " destructors declared! Aborting compilation" << endl;
      terminalErrors = true;
    }
    else if( ((TypeMethod*)type)->isDestructor ){
      hasDeclaredDestructor = true;
      destructor = (TypeMethod*)type;
    }
    else{
      if(((TypeMethod*)type)->isConstructor)
        hasDeclaredConstructor = true;
      if( !classTable->installMethod(((TypeMethod*)type)) )
        cerr << "ERROR: 1 Redeclaration of " << n << endl;
    }
  }
  else{
    if( !classTable->install(n,type) ){
      cerr << "ERROR: 2 Redeclaration of " << n << " " << type->toString() << endl;
    }
  }
}

void TypeClass::setParent(Type* p){
  parent = (TypeClass*)p;
}

TypeClass* TypeClass::getParent(){
  return parent;
}

SymbolTable* TypeClass::getSymbolTable(){
  return classTable;
}

char* TypeClass::toString(){
  return getName();
}

TypeMethod* TypeClass::getDestructor(){
  return destructor;
}

void TypeClass::toVMT(){
  cout << name << "$VMT:\n\t.long ";
  if( parent != NULL )
    cout << parent->getName() << "$VMT";
  else
    cout << "0";
  // Can assume that the destructor will exist by code gen time
  cout << "\n\t.long " << name << "$Destructor\n";
  SymbolTableRecord* scan = classTable->methodHead;
  while( scan != NULL ){
    if( !strcmp(((TypeMethod*)scan->type)->toString(), "method") ){
      cout << "\t.long ";
      ((TypeMethod*)scan->type)->toVMTString(name);
      cout << "\n";
    }
    scan = scan->next;
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Method Type class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeMethod::TypeMethod(char* n, Type* ret, bool cons, bool dest){
  name  = n;
  returnType = ret;
  isConstructor = cons;
  isDestructor  = dest;
  if( returnType == NULL ){
    signature.push_back(new TypeNull());
  }
  else{
    signature.push_back(returnType);
  }
}

TypeMethod::~TypeMethod(){
  delete name;
  delete returnType;
}

char* TypeMethod::getName(){
  return name;
}

SymbolTable* TypeMethod::exportAsSymbolTable(){
  SymbolTable* toRet = new SymbolTable();
  std::vector<SymbolTableRecord*>::iterator it;
  for( it = parameters.end(); it != parameters.begin(); --it){
    if( toRet->head == NULL )
      toRet->head = (*it);
    else{
      (*it)->next = toRet->head;
      toRet->head = (*it);
    }
  }
  return toRet;
}

bool TypeMethod::hasParam(char* n){
  std::vector<SymbolTableRecord*>::iterator it;
  for( it = parameters.begin(); it != parameters.end(); ++it){
    if( strcmp((*it)->name,n) == 0 )
      return true;
  }
  return false;
}

bool TypeMethod::addParam(char* n, Type* type){
  if( !hasParam(n) ){
    parameters.push_back(new SymbolTableRecord(n,type));
    signature.push_back(type);
    return true;
  }
  return false; //Can not have parameters with the same name
}

char* TypeMethod::toString(){
  return (char*)"method";
}

void TypeMethod::toVMTString(char* owner){
  cout << owner << "$" << name;
  std::vector<Type*>::iterator it;
  it = signature.begin();
  // Need to skip return type
  if( it != signature.end() ){
    while( ++it != signature.end() )
      cout << "_" << (*it)->toString();
  }
}

char* TypeMethod::getMunged(char* owner){
  int size = 500;
  char* toRet = new char[size];
  strcpy(toRet, owner);
  strcat(toRet, "$");
  strcat(toRet, name);
  std::vector<Type*>::iterator it;
  it = signature.begin();
  // Need to skip return type
  if( it != signature.end() ){
    while( ++it != signature.end() ){
      strcat(toRet,"_");
      strcat(toRet, (*it)->toString());
    }
  }
  return toRet;
}

std::vector<Type*> TypeMethod::getSig(){
  return signature;
}

// Compare the signatures of two TypeMethods
//bool TypeMethod::operator == (const std::vector<Type*> otherSig) const{
bool TypeMethod::operator== (const TypeMethod* other ){
  std::vector<Type*> otherSig = other->signature;
  if( signature.size() != otherSig.size() )
    return false;
  std::vector<Type*>::iterator mySigIt;
  mySigIt = signature.begin();
  std::vector<Type*>::iterator otherSigIt;
  otherSigIt = otherSig.begin();
  // If any of the parameters types don't line up then the methods are not
  // comparable
  while( mySigIt != signature.end() ){
    if( !strcmp((*mySigIt)->toString(),(*otherSigIt)->toString())){
      cerr << "DEBUG TypeMethod " << (*mySigIt) << ", " << (*otherSigIt) << endl;
      mySigIt++;
      otherSigIt++;
    }
    else{
      return false;
    }
  }
  return true;
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