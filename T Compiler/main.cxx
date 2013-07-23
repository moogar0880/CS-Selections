/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * main routine for compiler for T language
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
using namespace std;
#include "SymbolTable.h"
#include "StringPool.h"
#include <string.h>
#include "Type.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

// prototype for bison-generated parser
int yyparse();

// prototypes for code generation functions in encode.cxx
void encodeInitialize();
void encodeFinish();

// to enable bison parse-time debugging
#if YYDEBUG
extern int yydebug;
#endif

// global string pool
StringPool* stringPool;

// global symbol table
SymbolTable* symbolTable;

// global type module
TypeModule* types;

bool before, after, classes, terminalErrors;

int main(int argc, char* argv[])
{
  before = after = terminalErrors = false;
  if( argc > 1 ){
    if( strcmp(argv[1], "-before") == 0)
      before = true;
    else if( strcmp(argv[1], "-after") == 0)
      after = true;
    else if( strcmp(argv[1], "-classes") == 0)
      classes = true;
  }
  if( argc > 2 ){
    if( strcmp(argv[2], "-before") == 0)
      before = true;
    else if( strcmp(argv[2], "-after") == 0)
      after = true;
    else if( strcmp(argv[2], "-classes") == 0)
      classes = true;
  }
  if( argc > 3 ){
    if( strcmp(argv[3], "-before") == 0)
      before = true;
    else if( strcmp(argv[3], "-after") == 0)
      after = true;
    else if( strcmp(argv[3], "-classes") == 0)
      classes = true;
  }

  // create a string pool
  stringPool = new StringPool();

  // create a type module
  types = new TypeModule();

  // create a symbol table
  symbolTable = new SymbolTable();

  // set yydebug to 1 to enable bison debugging
  // (preprocessor symbol YYDEBUG must also be 1 here and in parse.yy)
#if YYDEBUG
  yydebug = 1;
#endif
  // generate prologue code
  encodeInitialize();

  // syntax directed compilation!
  yyparse();

  // generate epilogue code
  encodeFinish();

  // cleanup symbol table
  delete symbolTable;

  // cleanup the types
  delete types;

  // cleanup the string pool
  delete stringPool;

}
