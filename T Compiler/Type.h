#ifndef _TYPE_H
#define _TYPE_H
#include <vector>
#include <string.h>
class SymbolTable;
class SymbolTableRecord;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Type representation for T language
 *
 * NOTE: don't construct Type objects directly! Use a TypeModule.
 *
 * A TypeModule is simply a mechanism for managing the memory used
 * to represent types. The strategy is to only allocate memory for
 * a distinct type once and re-use that memory for subsequent calls
 * to construct that type. A TypeModule keeps track of all the
 * allocated types so that memory can be reclaimed when the types
 * are no longer needed.
 *
 * So, to obtain a type, access the appropriate member function for
 * the global type module object, which is created in main.cxx.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class Type{ // abstract base class
  public:
    virtual ~Type();
    virtual char* toString() = 0;

  protected:
    Type();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for items with no types
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeNone: public Type{
  public:
    TypeNone();
    ~TypeNone();

    char* toString();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for deref types
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeDeref: public Type{
  public:
    TypeDeref();
    ~TypeDeref();

    char* toString();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for a type which will or has resulted in an error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeError: public Type{
  public:
    TypeError();
    ~TypeError();

    char* toString();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for interger types
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeInt: public Type{
  public:
    TypeInt();
    ~TypeInt();

    char* toString();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for the null type
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeNull: public Type{
  public:
    TypeNull();
    ~TypeNull();

    char* toString();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for any method type
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
 class TypeMethod: public Type{
  protected:
    char* name;
    //dynamic parameter storage, needs to be a list because order matters
    std::vector<SymbolTableRecord*> parameters;
    std::vector<Type*> signature;
    int paramCount;

  public:
    TypeMethod(char* n, Type* ret, bool cons, bool dest);
    ~TypeMethod();
    bool isConstructor;
    bool isDestructor;
    Type* returnType;
    char* owner;

    bool  hasParam(char* n);
    bool  hasParam(char* n, Type*& type);
    bool  addParam(char* n, Type* type);
    char* toString();
    char* getName();
    void  toVMTString();
    void  toVMTString(char* owner);
    char* getMunged(char* owner);
    char* signatureString();
    std::vector<Type*> getSig();
    SymbolTable* exportAsSymbolTable();
    Type* getReturnType();
    bool  operator==(const TypeMethod* other);
 };

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Class representation for any class type
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeClass: public Type{
  protected:
    char* name;
    TypeClass* parent;
    SymbolTable* classTable;
    TypeMethod* destructor;

  public:
    TypeClass(char* n);
    ~TypeClass();

    std::vector<TypeMethod*> vmt;
    bool  getItem(char* n, Type*& type);
    void  add(char* n, Type* type);
    void  setParent(Type* p);
    TypeClass* getParent();
    char* toString();
    char* getName();
    void  encode();
    SymbolTable* getSymbolTable();
    void toVMT();
    TypeMethod* getDestructor();
    bool hasDeclaredDestructor;
    bool hasDeclaredConstructor;
    void inheritFields();
};

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Container class for all possible type classes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
class TypeModule{
  protected:
     Type* intTypeInternal;
     Type* errorTypeInternal;
     Type* noTypeInternal;
     Type* nullTypeInternal;
     Type* derefTypeInternal;
     std::vector<TypeClass*> classTypes; //dynamic Class Type storage

  public:
    TypeModule();
    ~TypeModule();
    Type* intType();
    Type* errorType();
    Type* noType();
    Type* nullType();
    Type* derefType();
    TypeClass* classType(char* name);
    Type* createNewClassType(char* n); //returns false if class already defined
    bool containsClassType(char* n);   //returns false on error
    std::vector<TypeClass*> getClassList(); //returns class vector
};
#endif