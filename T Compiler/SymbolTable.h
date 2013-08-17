#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H
#include <vector>
class Type;
class TypeMethod;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Symbol Table for T language
 *
 * It is just an inefficient linked list for now.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SymbolTableRecord (STR) Class
 *  Container for one symbol table entry
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class SymbolTableRecord{
  public:
    ~SymbolTableRecord();
    SymbolTableRecord(char*, Type*);
    void dump();
    int getStringLength();
    SymbolTableRecord* next;
    char* name;
    Type* type;
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SymbolTable Class
 *  The SymbolTable Proper
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class SymbolTable{
public:
    ~SymbolTable();
    SymbolTable();
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * Returns true if name is found
     *  if found, type returned through the second (reference) parameter
     *  In the case of lookupMethod the return type is passed by reference
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    bool lookup(char* name, Type*& type);
    bool lookupMethod(TypeMethod* m, Type*& type);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * Returns true if successful and name and type are stored in a STR
     * Fails if name is already in the ST, in this case nothing is changed
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    bool install(char* name, Type* type);
    bool installMethod(TypeMethod* type);

    void dump();
    int  getStringLength();
    int  size;
    SymbolTableRecord* getList();
    SymbolTableRecord* head;
    SymbolTableRecord* methodHead;
};
#endif