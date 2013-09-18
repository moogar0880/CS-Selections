/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST methods for code generation
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <cstdio>
#include <iostream>
#include <vector>
using namespace std;

#include "AST.h"
#include "Type.h"

// global type module is in main.cxx
extern TypeModule* types;
extern SymbolTable* symbolTable;
int labelCount = 0;
int whileCount = 0;
int curWhileExit = 0;
std::vector<int> breakStack;
std::vector<int> continueStack;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * encodeInitialize
 *  Output the prelude assembly code
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void encodeInitialize(){
  /*cout << "#\tPrologue\n";
  cout << "\t.text\n";
  cout << "\t.align 4\n";
  cout << "\t.globl\tmain\n";
  cout << "main:\n";
  cout << "\tpushl\t%ebp\n";
  cout << "\tmovl\t%esp, %ebp\n";*/
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * encodeFinish
 *  Output the epilogue assembly code
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void encodeFinish(){
  /*cout << "#\tEpilogue\n";
  cout << "main$exit:\n";
  cout << "\tpopl\t%ebp\n";
  cout << "\tret\n";*/
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_List encode
 *  Encode this lists item (if there is one), and continue encoding the
 *    rest of the list
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_List::encode(){
  if( item != NULL )
    item->encode();
  if (restOfList != NULL) restOfList->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IntegerLiteral encode
 *  Write the integer value to the stack
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_IntegerLiteral::encode(){
  cout << "#\tIntegerLiteral\n";
  cout << "\tpushl\t$" << value << endl;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Variable encode
 *  Declare a main variable with the given name
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Variable::encode(){
  if( !isParam ){
    cout << "#\tVariable\n";
    cout << "\tpushl\t$mainvar$" << name << endl;
  }
  else{
    cout << "#\tParameter " << type->toString() << " " << name << "\n";
    cout << "\tleal\t" << ((index-1)*4)+12 << "(%ebp), %eax\n";
    cout << "\tpushl %eax\n";
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_VariableList encode
 *  Encode each AST_Variable in a VariableList as if it were it's own
 *    AST_Variable
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_VariableList::encode(){
  cout << "#\tVariableList\n";
  cout << "mainvar$" << ((AST_Variable*)(item))->name << ": .long 0\n";
  if (restOfList != NULL) restOfList->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MainFunction encode
 *  If the MainFunction is not empty, encode the body of the main
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_MainFunction::encode(){
  cout << "#\tMainFunction\n";
  cout << "\t.text\n";
  cout << "\t.align 4\n";
  cout << "\t.globl\tmain\n";
  cout << "main:\n";
  cout << "\tpushl\t%ebp\n";
  cout << "\tmovl\t%esp, %ebp\n";
  if( list != NULL )
    list->encode();
  cout << "#\tMainFunctionExit\n";
  cout << "main$exit:\n";
  cout << "\tpopl\t%ebp\n";
  cout << "\tret\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Declaration encode
 *  Encode the Declaration's variable list surrounded by appropriate
 *    assembly code
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Declaration::encode(){
  cout << "#\tDeclaration\n";
  cout << "\t.data\n";
  list->encode();
  cout << "\t.text\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ExpressionStatement encode
 *  Encode the Expression and update the stack pointer
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ExpressionStatement::encode(){
  express->encode();
  cout << "#\tExpressionStatement\n";
  cout << "\taddl\t$4, %esp\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Assignment
 *  Encode the left hand side of the assignment, followed by the right
 *    hand side. Then proceed to move the contents of the right hand sides
 *    register into the left hand side's register.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Assignment::encode(){
  cout << "#\tAssignment\n";
  lhs->encode();
  rhs->encode();
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%edx\n";
  cout << "\tmovl\t%eax, (%edx)\n";
  cout << "\tpushl\t%eax\n";
  cout << "#\tEnd of Assignment\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Print encode
 *  Encode and print an intType value
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Print::encode(){
  var->encode();
  if( var->type == types->intType() ){
    cout << "#\tPrint int\n";
    cout << "\tcall\tRTS_outputInteger\n";
    cout << "\taddl\t$4, %esp\n";
  }
  else if( var->type == types->derefType() ){
    if( ((AST_Deref*)var)->left->type == types->intType() ){
      cout << "#\tPrint int\n";
      cout << "\tcall\tRTS_outputInteger\n";
      cout << "\taddl\t$4, %esp\n";
    }
  }
  else if( var->type == types->errorType() ){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Print::encode: unknown type " << var->type->toString() << endl;
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Divide encode
 *  Encode the left and right sides of the operation. Check to ensure that
 *    the divisor is not zero. If the divisor is zero, call the RTS
 *    divide by zero error to print an appropriate error message and
 *    continue execution.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Divide::encode(){
  left->encode();
  right->encode();
  int l1 = labelCount++;
  int l2 = labelCount++;
  if (type == types->intType()){
    cout << "#\tDivide\n";
    cout << "\tpopl\t%ecx\n";
    cout << "\tcmpl\t$0, %ecx\n";
    cout << "\tje\tL" << l1 << "\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tcltd\n";
    cout << "\tidivl\t%ecx, %eax\n";
    cout << "\tpushl\t%eax\n";
    cout << "\tjmp\tL" << l2 << "\n";
    cout << "L" << l1 << ":\n";
    cout << "\tpushl\t$" << line << "\n";
    cout << "\tcall\tRTS_printDivideByZeroError\n";
    cout << "L" << l2 << ":\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Divide::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Multiply encode
 *  Encode the left and right hand sides of the operation and multiply
 *    the values of the values, pushing the result to the stack
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Multiply::encode(){
  left->encode();
  right->encode();

  if (type == types->intType()){
    cout << "#\tMultiply int\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tpopl\t%edx\n";
    cout << "\timull\t%edx, %eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Multiply::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Add encode
 *  Encode the left and right hand sides of the operation and add
 *    the value of the values, pushing the result to the stack
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Add::encode(){
  left->encode();
  right->encode();

  if (type == types->intType()){
    cout << "#\tAdd int\n";
    cout << "\tpopl\t%edx\n";
    cout << "\tpopl\t%eax\n";
    cout << "\taddl\t%edx, %eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Add::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Subtract encode
 *  Encode the left and right hand sides of the operation and subtract
 *    the values of the values, pushing the result to the stack
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Subtract::encode(){
  left->encode();
  right->encode();

  if (type == types->intType()){
    cout << "#\tSubtract int\n";
    cout << "\tpopl\t%edx\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tsubl\t%edx, %eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Subtract::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Equality encode
 *  Encode the left and right hand sides of the operation and check the
 *    the equality of the two values being compared
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Equality::encode(){
  left->encode();
  right->encode();
  cout << "#\tEquality\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%edx\n";
  cout << "\tcmpl\t%eax,%edx\n";
  cout << "\tsete\t%al\n";
  cout << "\tmovzbl\t%al, %eax\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_LessThan encode
 *  Encode the left and right hand sides of the operation and determine
 *    if the value represented by the left hand side is less than that
 *    of the right.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_LessThan::encode(){
  right->encode();
  left->encode();
  cout << "#\tLess Than\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%edx\n";
  cout << "\tcmpl\t%eax, %edx\n";
  cout << "\tsetg\t%al\n";
  cout << "\tmovzbl\t%al, %eax\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_GreaterThan encode
 *  Encode the left and right hand sides of the operation and determine
 *    if the value represented by the left hand side is greater than that
 *    of the right.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_GreaterThan::encode(){
  right->encode();
  left->encode();
  cout << "#\tGreater Than\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%edx\n";
  cout << "\tcmpl\t%eax, %edx\n";
  cout << "\tsetl\t%al\n";
  cout << "\tmovzbl\t%al, %eax\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Negate encode
 *  Encode the left of the expression and negate the value stored within
 *    the int. If the variable is not an int type, print error
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Negate::encode(){
  left->encode();

  if (type == types->intType()){
    cout << "#\tNegate int\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tnegl\t%eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Subtract::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Not encode
 *  Encode the left and ensure that the type of left is an int, otherwise
 *    gracefully halt execution.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Not::encode(){
  left->encode();

  if (type == types->intType()){
    cout << "#\tLogical Not int\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tcmpl\t$0, %eax\n";
    cout << "\tsete\t%al\n";
    cout << "\tmovzbl\t%al, %eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Subtract::encode: unexpected type\n";
    exit(-1);
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Block encode
 *  Encode the StatementList within the block
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Block::encode(){
  cout << "#\tBlock\n";
  list->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_IfThenElse encode
 *  Begin by encoding the condition expression. If this expression is
 *    evaluated to true, the if statement block is executed, otherwise the
 *    else statement block is executed.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_IfThenElse::encode(){
  int l1 = labelCount++;
  int l2 = labelCount++;

  cout << "#\tIf then else\n";
  condition->encode();
  cout << "\tpopl\t%eax\n";
  cout << "\tcmpl\t$0, %eax\n";
  cout << "\tje\tL" << l1 << "\n";
  if( ifstat != NULL )
    ifstat->encode();
  cout << "\tjmp\tL" << l2 << "\n";
  cout << "L" << l1 << ":\n";
  if( elstat != NULL )
    elstat->encode();
  cout << "L" << l2 << ":\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_While encode
 *  Encode the condition expression and determine if it is true. If it is
 *    true continue executing the while's block code, after which the
 *    condition is re-evaluated.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_While::encode(){
  int l1 = whileCount++;
  int l2 = whileCount++;
  continueStack.push_back(l1);
  breakStack.push_back(l2);
  cout << "#\tWhile\n";
  cout << "W" << l1 << ":\n";
  condition->encode();
  cout << "\tpopl\t%eax\n";
  cout << "\tcmpl\t$0, %eax\n";
  cout << "\tje\tW" << l2 << "\n";
  stat->encode();
  cout << "\tjmp\tW" << l1 << "\n";
  cout << "W" << l2 << ":\n";
  cout << "#\tEnd While\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Return encode
 *  If there is a variable/value to return encode it, otherwise jump to
 *    the exit because you are returning from the main function
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Return::encode(){
  cout << "#\tReturn\n";
  if( var != NULL ){
    var->encode();

  }
  cout << "\tpopl\t%eax\n";
  cout << "\tjmp\t\t" << label << "\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Continue encode
 *  Jump to the beginning of the inner-most while loop
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Continue::encode(){
  int e = continueStack.back();
  continueStack.pop_back();
  cout << "#\tContinue\n";
  cout << "\tjmp\tW" << e << "\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Break encode
 *  Jump out of the inner-most while loop
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Break::encode(){
  int e = breakStack.back();
  breakStack.pop_back();
  cout << "#\tBreak\n";
  cout << "\tjmp\tW" << e << "\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Null encode
 *  Push zero onto the stack
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Null::encode(){
  cout << "#\tNull\n";
  cout << "\tpushl\t$0\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_CompilationUnit encode
 *  Output Object's VMT and encode the class list (if there is one) and
 *    the main block
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_CompilationUnit::encode(){
  std::vector<TypeClass*> cs = types->getClassList();
  std::vector<TypeClass*>::iterator it;
  for(it = cs.begin(); it != cs.end(); ++it ){
    (*it)->toVMT();
  }

  if( list != NULL )
    list->encode();
  main->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Class encode
 *  Set the parent VMT to Object$VMT if for some reason there isn't one
 *    and encode all methods appropriately
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Class::encode(){
  AST_Statement* stat;
  AST_StatementList* statList;
  stat = (AST_Statement*)fields->getItem();
  statList = (AST_StatementList*)fields->getRestOfList();
  while( stat != NULL ){
    if( stat->declarationType ){// AST_Method Found
      ((AST_Method*)stat)->encode();
    }
    if( statList != NULL ){
      stat = (AST_Statement*)statList->getItem();
      statList = (AST_StatementList*)statList->getRestOfList();
    }
    else{
      stat = NULL;
    }
  }
  TypeClass* tc = types->classType(name);
  if( !tc->hasDeclaredDestructor ){
    cout << "#\tDefault Destructor " << name << "$Destructor\n";
    cout << "\t.align\t4\n";
    cout << "\t.globl\t" << name << "$Destructor\n";
    cout << name << "$Destructor:\n";
    // Need to call super destructor OR encode it
    // cout << "\tpopl\t%ebp\n";
    // cout << "\tret\n";
    cout << "\tpushl\t%ebp\t\t\t# save old frame pointer\n";
    cout << "\tmovl\t%esp, %ebp\t\t# establish new frame pointer\n";
    cout << "\tcall\t" << tc->getParent()->toString() << "$" << "Destructor\n";
    cout << name << "$Destructor$exit:\n";
    cout << "\tpopl\t%ebp\t\t\t# restore caller's frame pointer\n";
    cout << "\tret\t\t\t\t\t\t# restore caller's program counter\n";
  }
  if( !tc->hasDeclaredConstructor ){
    cout << "#\tDefault Constructor " << name << "$" << name << "\n";
    cout << "\t.align\t4\n";
    cout << "\t.globl\t" << name << "$" << name << "\n";
    cout << name << "$" << name << ":\n";
    // Need to call super destructor OR encode it
    // cout << "\tpopl\t%ebp\n";
    // cout << "\tret\n";
    cout << "\tpushl\t%ebp\t\t\t# save old frame pointer\n";
    cout << "\tmovl\t%esp, %ebp\t\t# establish new frame pointer\n";
    cout << "\tcall\t" << tc->getParent()->toString() << "$" << tc->getParent()->toString() << "\n";
    cout << name << "$" << name << "$exit:\n";
    cout << "\tpopl\t%ebp\t\t\t# restore caller's frame pointer\n";
    cout << "\tret\t\t\t\t\t\t# restore caller's program counter\n";
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Cast encode
 *  Push the VMT of the class we're casting to and use the RTS to check
 *    the run time type of the variable and ensure that a valid cast is
 *    being made
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Cast::encode(){
  char* targetName = ((AST_Variable*)(expr))->name;
  cast->encode();
  cout << "#\tClass Cast\n";
  cout << "\tpushl\t$" << targetName << "$VMT\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkCast\n";
  cout << "\taddl\t$8, %esp\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldReference encode
 *  Calculate the offset of the field within the class type and pull the
 *    data out of it
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_FieldReference::encode(){
  cout << "#\tField Reference\n";
  owner->encode();
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkForNullReference\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%eax\n";

  SymbolTableRecord* scan = types->classType(owner->type->toString())->getSymbolTable()->head;
  int offset = 4;
  while(scan != NULL){
    if( !strcmp(scan->name, variable) )
      break;
    offset += 4;
    scan = scan->next;
  }
  cout << "\taddl\t$" << offset << ", %eax\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_FieldDeclaration encode
 *
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_FieldDeclaration::encode(){}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassInstance encode
 *  Encode length of the class and calloc enough space for it. This call
 *    to calloc requires a RTS check to ensure we have not run out of
 *    memory.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ClassInstance::encode(){
  int length = 1; //length of the object in 4-byte words
  char* className = (((TypeClass*)type)->toString());
  SymbolTableRecord* scan = types->classType(className)->getSymbolTable()->head;
  while( scan != NULL ){
    length += 1;
    scan = scan->next;
  }
  int nArgs = 0;
  cout << "#\t" << className << " Class Instance\n";
  if( arguments != NULL ){
    arguments->encode();
    nArgs = arguments->getLength();
  }
  cout << "\tpushl\t$" << nArgs << "\n";
  cout << "\tcall\tRTS_reverseArgumentsOnStack\n";
  cout << "\tpopl\t%ecx\t\t\t# discard n+1 argument\n";
  cout << "\tpushl\t$4\t\t\t# unit size to be used by calloc\n";
  cout << "\tpushl\t$" << length << "\t\t\t# number of units to be allocated\n";
  cout << "\tcall\tcalloc\t\t# address of new object returned in eax\n";
  cout << "\taddl\t$8, %esp\t# deallocate arguments to calloc\n";
  cout << "\tcmpl\t$0, %eax\n";
  cout << "\tjne\t" << "\tCI" << ++labelCount << "\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_outOfMemoryError\n";
  cout << "CI" << labelCount << ":\n";
  cout << "\tmovl\t$" << className << "$VMT, (%eax)\n";
  cout << "\tpushl\t%eax\t\t# pass the \"this\" pointer\n";
  cout << "\tcall\t" << className << "$" << className <<  "\n";
  cout << "\tpopl\t%eax\t\t# get address of new object into eax\n";
  cout << "\taddl\t$" << nArgs * 4 << ",\t%esp\t# deallocate arguments to constructor\n";
  cout << "\tpushl\t%eax\n";
  cout << "#\tEnd of Class Instance Creation\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_EmptyStatement encode
 *  Takes care of itself
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_EmptyStatement::encode(){

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Delete encode
 *  Encode the variable to delete and check to ensure that it is not NULL.
 *    If it is not NULL, pull the destructor out of the VMT and execute
 *    it.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Delete::encode(){
  cout << "#\tDelete " << variableName << "\n";
  variable->encode();

  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkForNullReference\n";
  cout << "\tpopl\t%ecx\t\t\t# discard line number\n";
  cout << "\tpopl\t%eax\t\t\t# get copy of object address in eax\n";
  cout << "\tpushl\t%eax\n";
  cout << "\tmovl\t(%eax), %eax\t# get VMT out of object\n";
  cout << "\taddl\t$4, %eax\t\t# destructor is always in slot 1\n";
  cout << "\tmovl\t(%eax), %eax\t# get destructor address from VMT\n";
  cout << "\tcall\t*%eax\n";
  cout << "\tcall\tfree\t\t\t# address of object is still on stack\n";
  cout << "\taddl\t$4, %esp\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ArgumentsList encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*void AST_ArgumentsList::encode(){

}*/

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  Output the munged name, the method declarator, and the method body
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Method::encode(){
  if(flag == 0)
    cout << "#\tMethod " << mungedName << "\n";
  if(flag == 1)
    cout << "#\tConstructor " << mungedName << "\n";
  if(flag == -1)
    cout << "#\tDestructor " << mungedName << "\n";
  cout << "\t.align\t4\n";
  cout << "\t.globl\t" << mungedName << "\n";
  cout << mungedName << ":\n";
  cout << "\tpushl\t%ebp\t\t\t# save old frame pointer\n";
  cout << "\tmovl\t%esp, %ebp\t\t# establish new frame pointer\n";
  declarator->encode();
  body->encode();
  cout << "\tmovl\t$0, %eax\n";
  cout << mungedName << "$exit:\n";
  cout << "\tpopl\t%ebp\t\t\t# restore caller's frame pointer\n";
  cout << "\tret\t\t\t\t\t\t# restore caller's program counter\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Parameter encode
 *  Encode the offset of the parameter
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Parameter::encode(){
  cout << "#\tParameter " << type->toString() << " " << name << "\n";
  cout << "\tleal\t" << ((index-1)*4)+12 << "(%ebp), %eax\n";
  cout << "\tpushl %eax\n";
}

int AST_MethodInvoke::count(SymbolTableRecord* scan, int c, char* s){
  if( scan != NULL ){
    if( !strcmp(scan->name,s) )
      return c+4;
    return count(scan->next,c+4,s);
  }
  return c;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodInvoke encode
 *  Encode the class instance calling the method, the parameters, and
 *  then the body of the method invokation
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_MethodInvoke::encode(){
  cout << "#\tMethodInvoke " << methodName << ": " << identifier <<"\n";
  int n = 0;
  if( params != NULL )
    n = params->getLength();
  // int methodCount = count(types->classType(callingType->getName())->getSymbolTable()->methodHead,8,sig);
  int methodCount = 8;
  for(std::vector<TypeMethod*>::iterator it = types->classType(callingType->getName())->vmt.begin(); it != types->classType(callingType->getName())->vmt.end(); ++it)
  {
    if( !strcmp((*it)->signatureString(), sig) ){
      break;
    }
    methodCount += 4;
  }
  // cout << "\tpushl\t$" << n + 1 << "\n";
  if( identifier > 0 ){ // Variable method call
    source->encode();
  }

  if( params != NULL )
    params->encode();
  if( identifier > 0 ){
    // cout << "\tcall\tRTS_reverseArgumentsOnStack\n";
    // cout << "\tpopl\t%ecx\t\t\t# discard n+1 argument\n";
    cout << "\tpushl\t$" << line << "\n";
    cout << "\tcall\tRTS_checkForNullReference\n";
    cout << "\tpopl\t%ecx\t\t\t# discard line number\n";
    cout << "\tpopl\t%eax\t\t\t# get copy of \"this\" in eax\n";
    cout << "\tpushl\t%eax\t\t\t# put copy of \"this\" back on stack\n";
    cout << "\tmovl\t(%eax), %eax\t# put VMT pointer into eax\n";
    cout << "\taddl\t$" << methodCount << ", %eax\n";
  }
  else if( identifier == 0 ){ // this method
    cout << "\tmovl\t8(%ebp), %eax\t# 8(%ebp) is the \"this\" pointer\n";
    cout << "\tpushl\t%eax\n";
    cout << "\tmovl\t(%eax), %eax\t# put VMT pointer into eax\n";
    cout << "\taddl\t$" << methodCount << ", %eax\n";
  }
  else if( identifier < 0 ){ //Super method
    cout << "\tmovl\t8(%ebp), %eax\t# 8(%ebp) is the \"this\" pointer\n";
    cout << "\tpushl\t%eax\n";
    cout << "\tmovl\t(%eax), %eax\t# put VMT pointer into eax\n";
    cout << "\taddl\t$" << methodCount << ", %eax\n";
  }
  cout << "\tmovl\t(%eax), %eax\t# put method address into eax\n";
  cout << "\tcall\t*%eax\n";
  cout << "\taddl\t$(" << n << "+1)*4, %esp\t# deallocate arguments from stack\n";
  cout << "\tpushl\t%eax\t\t\t# leave method return value on top of stack\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ParameterList encode
 *  Encode all parameters in the ParameterList
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ParameterList::encode(){
  int i = 1;
  AST_List* scan = restOfList;
  ((AST_Parameter*)(item))->index = i++;
  ((AST_Parameter*)(item))->encode();
  if( scan != NULL ){
    while(scan != NULL){
      ((AST_Parameter*)(scan->getItem()))->index = i++;
      ((AST_Parameter*)(scan->getItem()))->encode();
      scan = scan->getRestOfList();
    }
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_MethodDeclarator encode
 *  Encode the list of method parameters, if there are any
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_MethodDeclarator::encode(){
  if( params != NULL )
    params->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ConstructorInvoke::encode(){

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_This encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_This::encode(){
  cout << "\tmovl\t8(%ebp), %eax\t#8(%ebp) is the \"this\" pointer\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Deref encode
 *  Dereference the variable stored in left
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Deref::encode(){
  left->encode();

  cout << "#\tDeref\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tmovl\t(%eax), %eax\n";
  cout << "\tpushl\t%eax\n";
  cout << "#\tEnd Deref\n";
}
