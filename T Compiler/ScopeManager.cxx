/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ScopeManager for the T language
 * Maintain knowledge of the current scope of the program
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <iostream>
using namespace std;
#include "ScopeManager.h"
#include "Type.h"
// global type module is in main.cxx
extern TypeModule* types;
// global symbol table is in main.cxx
extern SymbolTable* symbolTable;

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ScopeManager class
 *  Maintains getters, setters, and clearers for updating scoping status
 *  on the fly
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
ScopeManager::ScopeManager(){
    inMethod = inClass = inMain = false;
}

ScopeManager::~ScopeManager(){
    delete curMethod;
    delete curClass;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Set the scope to be currently within the AST_Method represented by
 *  the TypeMethod* curMethod
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::setMethod(TypeMethod* m){
    inMethod  = true;
    curMethod = m;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Return the current method's TypeMethod* representation
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeMethod* ScopeManager::getMethod(){
    return curMethod;
}

bool ScopeManager::in_method(){
    return inMethod;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Clear method data. Only to be used when exiting the Method
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::clearMethod(){
    inMethod  = false;
    curMethod = NULL;
    clearCaller();
}

void ScopeManager::setCaller(SymbolTableRecord* str){
    caller = str;
}

SymbolTableRecord* ScopeManager::getCaller(){
    return caller;
}

void ScopeManager::clearCaller(){
    caller = NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Set the scope to show as being currently within a class named curClass
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::setClass(char* n){
    inClass  = true;
    curClass = n;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Using the name of the class return the TypeClass representation of
 *  the class currently in scope
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
TypeClass* ScopeManager::getClass(){
    if(curClass == NULL)
        return NULL;
    return types->classType(curClass);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Clear all class data. Only to be called when exiting the Class
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::clearClass(){
    inClass  = false;
    curClass = NULL;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Set the current scope to show we are in the main block
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::setMain(){
    inMain = true;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Clear the main block data, only to be called when exiting the main
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void ScopeManager::clearMain(){
    inMain = false;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Return the SymbolTable of the 'inner-most' scope
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
SymbolTable* ScopeManager::currentScope(){
    if( inMethod )
        return curMethod->exportAsSymbolTable();
    if( inClass )
        return types->classType(curClass)->getSymbolTable();
    return symbolTable;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Returns the scope one level higher than the current scope. This method
 *  can only be used for accessing class fields from within methods, as
 *  there is currently no support for truly global variables that can
 *  be shared between the main block and classes.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
SymbolTable* ScopeManager::up(){
    if( inMethod )
        return types->classType(curClass)->getSymbolTable();
    return NULL;
}

void ScopeManager::printScope(){
    if( inMain ){
        cerr << "\tCurrent Scope: MainFunction\n";
    }
    else{
        cerr << "\tCurrent Scope: " << curClass;
        if( inMethod )
            cerr << "\'s " << curMethod->getName() << " method";
        cerr << "\n";
    }
}