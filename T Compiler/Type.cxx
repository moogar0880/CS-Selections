/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Type representation for T language
 *
 * NOTE: Don't use Type methods directly. Rather use indirectly via
 *       TypeModule. See Type.h for more information.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;
#include <stdio.h>
#include "Type.h"

//~~~~~~~~~~~~~~~~~~~~~~~~Type~~~~~~~~~~~~~~~~~~~~~~~~
Type::Type(){}

Type::~Type(){}

//~~~~~~~~~~~~~~~~~~~~~~~~No Type~~~~~~~~~~~~~~~~~~~~~~~~
TypeNone::TypeNone(){}

TypeNone::~TypeNone(){}

char* TypeNone::toString(){
  return (char *) "<no type>";
}

//~~~~~~~~~~~~~~~~~~~~~~~~Error Type~~~~~~~~~~~~~~~~~~~~~~~~
TypeError::TypeError(){}

TypeError::~TypeError(){}

char* TypeError::toString(){
  return (char *) "<error type>";
}

//~~~~~~~~~~~~~~~~~~~~~~~~Int Type~~~~~~~~~~~~~~~~~~~~~~~~
TypeInt::TypeInt(){}

TypeInt::~TypeInt(){}

char* TypeInt::toString(){
  return (char *) "int";
}

//~~~~~~~~~~~~~~~~~~~~~~~~Null Type~~~~~~~~~~~~~~~~~~~~~~~~
TypeNull::TypeNull(){}

TypeNull::~TypeNull(){}

char* TypeNull::toString(){
  return (char *) "null";
}

//~~~~~~~~~~~~~~~~~~~~~~~~ClassType~~~~~~~~~~~~~~~~~~~~~~~~
TypeClass::TypeClass(char* n){
  name = n;
}

TypeClass::~TypeClass(){
  delete name;
}

char* TypeClass::getName(){
  return name;
}

char* TypeClass::toString(){
  return getName();
}

//~~~~~~~~~~~~~~~~~~~~~~~~TypeModule~~~~~~~~~~~~~~~~~~~~~~~~
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