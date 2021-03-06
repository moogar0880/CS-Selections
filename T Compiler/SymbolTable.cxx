/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Symbol Table for T language
 * Symbol tables are implemented as a linked list. "Stupid is beautiful."
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;
#include "Type.h"
#include "SymbolTable.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SymbolTableRecord (STR) Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
SymbolTableRecord::SymbolTableRecord(char* n, Type* t){
  next = NULL;
  name = n;
  type = t;
}

SymbolTableRecord::~SymbolTableRecord(){
  if(next != NULL) delete next;
  // can't free name or type as their values may still be needed
}

void SymbolTableRecord::dump(){
  if(name == NULL || type == NULL){
    cerr << "corrupt symbol table record with NULL name or type!\n";
  }
  else{
    cerr << "symbol table record (" << name << ", " << type->toString() <<
      ")\n";
  }
  if(next != NULL) next->dump();
}

int SymbolTableRecord::getStringLength(){
  return strlen(name);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SymbolTable Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
SymbolTable::SymbolTable(){
  head = NULL;
  methodHead = NULL;
}

SymbolTable::~SymbolTable(){
  delete head;
}

bool SymbolTable::lookup(char* name, Type*& type){
  SymbolTableRecord* cur = head;
  while (cur != NULL){
    if (!strcmp(name, cur->name)){
      type = cur->type;
      return true;
    }
    cur = cur->next;
  }
  return false;
}

bool SymbolTable::lookupMethod(TypeMethod* m, Type*& type){
  SymbolTableRecord* cur = methodHead;
  while( cur != NULL ){
    if( !strcmp(((TypeMethod*)cur->type)->getName(), m->getName()) ){
      type = ((TypeMethod*)cur->type)->returnType;
      return true;
    }
    cur = cur->next;
  }
  return false;
}

bool SymbolTable::install(char* name, Type* type){
  SymbolTableRecord* cur = head;
  SymbolTableRecord* last = NULL;
  while (cur != NULL){
    if (!strcmp(name, cur->name)){
      return false;
    }
    last = cur;
    cur = cur->next;
  }
  if (last == NULL){
    head = new SymbolTableRecord(name, type);
  }
  else{
    last->next = new SymbolTableRecord(name, type);
  }
  size += 1;
  return true;
}

bool SymbolTable::installMethod(TypeMethod* m){
  SymbolTableRecord* cur  = methodHead;
  SymbolTableRecord* last = NULL;
  while( cur != NULL ){
    if( ((TypeMethod*)cur->type) == m ){
      return false; // Method already exists
    }
    last = cur;
    cur = cur->next;
  }
  if( last == NULL ){
    methodHead = new SymbolTableRecord(m->getName(), m);
  }
  else{
    last->next = new SymbolTableRecord(m->getName(), m);
  }
  return true;
}

void SymbolTable::dump(){
  if( head != NULL )
    head->dump();
  else
    cerr<<"SymbolTable EMPTY\n";
}

int SymbolTable::getStringLength(){
  SymbolTableRecord* cur = head;
  int newLineCounter = 0, stringLength = 0;
  while( cur != NULL ){
    stringLength += cur->getStringLength();
    newLineCounter++;
    cur = cur->next;
  }
  return stringLength + newLineCounter;
}

SymbolTableRecord* SymbolTable::getList(){
  return head;
}