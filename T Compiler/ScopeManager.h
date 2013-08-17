#ifndef _SCOPEMANAGER_H
#define _SCOPEMANAGER_H
#include "Type.h"
#include "SymbolTable.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ScopeManager for T Compiler
 *
 * The main purpose of the ScopeManager is to keep track of the context
 *  that any variables or method calls you may come across will be in.
 *  Because there is a possiblity for method calls to be made from
 *  several different contexts, this simple utility provides the
 *  appropriate SymbolTables, TypeMethods, or TypeClasses to the analyzer
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class ScopeManager{
protected:
    bool inMethod;
    TypeMethod* curMethod;

    bool inClass;
    char* curClass;

    bool inMain;
    // An accessor to keep track of which ST the ScopeManager has
    // presented to the compiler
    int  inUseCurrently;

public:
	ScopeManager();
	~ScopeManager();

    void setMethod(TypeMethod* m);
    TypeMethod* getMethod();
    void clearMethod();

    void setClass(char* n);
    TypeClass* getClass();
    void clearClass();

    void setMain();
    void clearMain();

    SymbolTable* currentScope();
    SymbolTable* up();
};
#endif
