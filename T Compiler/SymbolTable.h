#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

#include "Type.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Symbol Table for T language
 *
 * It is just an inefficient linked list for now.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// Container for one symbol table entry
class SymbolTableRecord{
  public:
    ~SymbolTableRecord();
    SymbolTableRecord(char*, Type*);
    void dump();
    SymbolTableRecord* next;
    char* name;
    Type* type;
};

// The SymbolTable proper
class SymbolTable{
  protected:
    SymbolTableRecord* head;

  public:
    ~SymbolTable();
    SymbolTable();
    // returns true if name is found
    // if found, type returned through the second (reference) parameter
    bool lookup(char* name, Type*& type);

    // returns true if successful
    // in this case name and type are stored in a new record in table
    // it only fails if name is already in the list
    // in that case symbol table is not changed
    bool install(char* name, Type* type);

    void dump();
};
#endif