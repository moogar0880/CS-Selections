#ifndef _TYPE_H
#define _TYPE_H
#include <vector>
#include <string.h>
// April 2013
// Type representation for T language
//
// NOTE: don't construct Type objects directly! Use a TypeModule.
//
// A TypeModule is simply a mechanism for managing the memory used
// to represent types. The strategy is to only allocate memory for
// a distinct type once and re-use that memory for subsequent calls
// to construct that type. A TypeModule keeps track of all the
// allocated types so that memory can be reclaimed when the types
// are no longer needed.
//
// So, to obtain a type, access the appropriate member function for
// the global type module object, which is created in main.cxx.

class Type{ // abstract base class
  public:
    virtual ~Type();
    virtual char* toString() = 0;

  protected:
    Type();
};

class TypeNone: public Type{
  public:
    TypeNone();
    ~TypeNone();

    char* toString();
};

class TypeError: public Type{
  public:
    TypeError();
    ~TypeError();

    char* toString();
};

class TypeInt: public Type{
  public:
    TypeInt();
    ~TypeInt();

    char* toString();
};

class TypeNull: public Type{
  public:
    TypeNull();
    ~TypeNull();

    char* toString();
};

class TypeClass: public Type{
  protected:
    char* name;
  public:
    TypeClass(char* n);
    ~TypeClass();

    char* toString();
    char* getName();
};

class TypeModule{
  protected:
     Type* intTypeInternal;
     Type* errorTypeInternal;
     Type* noTypeInternal;
     Type* nullTypeInternal;
     std::vector<TypeClass*> classTypes; //dynamic Class Type storage

  public:
    TypeModule();
    ~TypeModule();
    Type* intType();
    Type* errorType();
    Type* noType();
    Type* nullType();
    Type* classType(char* name);
    bool createNewClassType(char* n); //returns false if class already defined
    bool containsClassType(char* n); //returns false on error
};
#endif