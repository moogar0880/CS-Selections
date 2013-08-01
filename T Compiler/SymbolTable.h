#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

class Type;
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
    SymbolTableRecord* next;
    char* name;
    Type* type;
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SymbolTable Class
 *  The SymbolTable Proper
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class SymbolTable{
  protected:
    SymbolTableRecord* head;

  public:
    ~SymbolTable();
    SymbolTable();
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * Returns true if name is found
     *  if found, type returned through the second (reference) parameter
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    bool lookup(char* name, Type*& type);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * Returns true if successful and name and type are stored in a STR
     * Fails if name is already in the ST, in this case nothing is changed
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    bool install(char* name, Type* type);

    void dump();
};
#endif