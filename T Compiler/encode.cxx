/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST methods for code generation
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <stdlib.h>
#include <cstdio>
#include <iostream>
using namespace std;

#include "AST.h"
#include "Type.h"

// global type module is in main.cxx
extern TypeModule* types;
extern SymbolTable* symbolTable;
int labelCount = 0;
int whileCount = 0;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * encodeInitialize
 *  Output the prelude assembly code
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void encodeInitialize(){
  cout << "#\tPrologue\n";
  cout << "\t.text\n";
  cout << "\t.align 4\n";
  cout << "\t.globl\tmain\n";
  cout << "main:\n";
  cout << "\tpushl\t%ebp\n";
  cout << "\tmovl\t%esp, %ebp\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * encodeFinish
 *  Output the epilogue assembly code
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void encodeFinish(){
  cout << "#\tEpilogue\n";
  cout << "main$exit:\n";
  cout << "\tpopl\t%ebp\n";
  cout << "\tret\n";
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
  cout << "#\tVariable\n";
  cout << "\tpushl\t$mainvar$" << name << endl;
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
  if( list != NULL )
    list->encode();
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
  //cerr << "Equality " << left->type->toString() << " =?= " << right->type->toString() << endl;
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
  if( var == NULL ){
    var->encode();
    cout << "\tpopl\t%eax\n";
  }
  cout << "\tjmp\tmain$exit\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Continue encode
 *  Jump to the beginning of the inner-most while loop
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Continue::encode(){
  int l1 = labelCount - 2;
  cout << "#\tContinue\n";
  cout << "\tjmp\tL" << l1 << "\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Break encode
 *  Jump out of the inner-most while loop
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Break::encode(){
  int l1 = whileCount - 1;
  cout << "#\tBreak\n";
  cout << "\tjmp\tW" << l1 << "\n";
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
  // generate Object Class code immediately for ease
  /*cout << "#\tObject Class VMT\n";
  cout << "\t.data\n";
  cout << "Object$VMT:\n";
  cout << "\t.long\t0\n";
  cout << "\t.text\n";
  cout << "~~~~~~~~~~~~~~~~~~~~~~~" << endl;*/
  std::vector<TypeClass*> cs = types->getClassList();
  std::vector<TypeClass*>::iterator it;
  for(it = cs.begin(); it != cs.end(); ++it ){
    cout << (*it)->toVMT();
  }
  //cout << "~~~~~~~~~~~~~~~~~~~~~~~" << endl;

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
    cout << name << "$Destructor:\n";
    cout << "\tret\n";
  }
  if( !tc->hasDeclaredConstructor ){
    cout << name << "$" << name << ":\n";
    cout << "\tret\n";
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
  cout << "#\tClass Case\n";
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
  cout << "\tpushl\t$mainvar$" << owner << endl;
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkForNullReference\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%eax\n";
  Type* t;
  symbolTable->lookup(owner,t);
  SymbolTableRecord* scan = types->classType(t->toString())->getSymbolTable()->head;
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
void AST_FieldDeclaration::encode(){
  //Empty...for now
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_ClassInstance encode
 *  Encode length of the class and calloc enough space for it. This call
 *    to calloc requires a RTS check to ensure we have not run out of
 *    memory.
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ClassInstance::encode(){
  int length = 4; //length of the object in 4-byte words
  char* className = (((TypeClass*)type)->toString());
  SymbolTableRecord* scan = types->classType(className)->getSymbolTable()->head;
  while( scan != NULL ){
    length += 4;
    scan = scan->next;
  }
  cout << "#\t" << className << " Class Instance\n";
  cout << "\tpushl\t$4\n";
  cout << "\tpushl\t$" << length << "\n";
  cout << "\tcall\tcalloc\n";
  cout << "\taddl\t$8, %esp\n";
  cout << "\tcmpl\t$0, %eax\n";
  cout << "\tjne\t" << "\tCI" << ++labelCount << "\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_outOfMemoryError\n";
  cout << "CI" << labelCount << ":\n";
  cout << "\tmovl\t$" << className << "$VMT, (%eax)\n";
  cout << "\tpushl\t%eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_EmptyStatement encode
 *  Takes care of itself
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_EmptyStatement::encode(){

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Convert encode
 *  Need to remove since we only have ints and reference types
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Convert::encode(){
  left->encode();
  //cerr << line << ": Declared type of Convert = " << type->toString() << endl;
  if (type == types->intType()){
    cout << "#\tConvert to int\n";
    // load value into fp register
    cout << "\tflds\t(%esp)\n";
    // set rounding mode to truncate
    // store fp control word in first word
    cout << "\tfnstcw\t(%esp)\n";
    // put control word in %ax
    cout << "\tmovw\t(%esp), %ax\n";
    // set RC bits to "truncate"
    cout << "\tmovb\t$12, %ah\n";
    // put modified control word in 2nd slot
    cout << "\tmovw\t%ax, 2(%esp)\n";
    // load modified control word
    cout << "\tfldcw\t2(%esp)\n";
    // put old control word in %ax
    cout << "\tmovw\t(%esp), %ax\n";
    // convert value to signed int
    cout << "\tfistpl\t(%esp)\n";
    // re-establish old fp control word */
    // alloc one word on top of stack
    cout << "\tsubl\t$2, %esp\n";
    // put original cntrl word on stack
    cout << "\tmovw\t%ax, (%esp)\n";
    // re-load original control word
    cout << "\tfldcw\t(%esp)\n";
    // free word on top of stack
    cout << "\taddl\t$2, %esp\n";
  }
  else{
    //cerr << line << ": BUG in AST_Convert::encode: unexpected type\n";
    //exit(-1);
  }
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
void AST_ArgumentsList::encode(){

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Method::encode(){
  cout << "#\t Method " << mungedName << "\n";
  cout << "\t.align\t4\n";
  cout << "\t.globl\t" << mungedName << "\n";
  cout << mungedName << ":\n";
  cout << "\tpushl\t%ebp\t\t\t# save old frame pointer\n";
  cout << "\tmovl\t%esp, %ebp\t\t# establish new frame pointer\n";
  body->encode();
  cout << "\tmovl\t$0, %eax\n";
  cout << mungedName << "$exit:\n";
  cout << "\tpopl\t%ebp\t\t\t# restore caller's frame pointer\n";
  cout << "\tret\t\t\t\t\t\t# restore caller's program counter\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_Parameter::encode(){
  cout << "#\t Parameter " << type->toString() << " " << name << "\n";
  cout << "\tleal\t((" << index << "-1)*4)+12(%ebp), %eax\n";
  cout << "pushl %eax\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_MethodInvoke::encode(){
  cout << "#\tMethodInvoke " << methodName << "\n";
  if( identifier > 0 ){ // Variable method call
    source->encode();
  }
  params->encode();
  int n = params->getLength();

  cout << "pushl $" << n + 1 << "\n";
  cout << "call\tRTS_reverseArgumentsOnStack\n";
  cout << "popl\t%ecx\t\t# discard n+1 argument\n";
  cout << "pushl $" << line << "\n";
  cout << "call\tRTS_checkForNullReference\n";
  cout << "popl\t%ecx\t\t# discard line number\n";
  cout << "popl\t%eax\t\t# get copy of \"this\" in eax\n";
  cout << "pushl\t%eax\t\t# put copy of \"this\" back on stack\n";
  cout << "movl\t(%eax), %eax\t\t# put VMT pointer into eax\n";
  cout << "addl\t$" << methodOffset << ", %eax\n";
  cout << "movl\t(%eax), %eax\t\t# put method address into eax\n";
  cout << "call\t*%eax\n";
  cout << "addl\t$(" << n << "+1)*4], %esp\t# deallocate arguments from stack\n";
  cout << "pushl\t%eax\t\t\t# leave method return value on top of stack\n";
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ParameterList::encode(){
  int i = 1;
  AST_List* scan = restOfList;
  ((AST_Parameter*)(item))->index = i++;
  ((AST_Parameter*)(item))->encode();
  while(scan->getItem() != NULL){
    ((AST_Parameter*)(scan->getItem()))->index = i++;
    ((AST_Parameter*)(scan->getItem()))->encode();
  }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_MethodDeclarator::encode(){
  params->encode();
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * AST_Method encode
 *  TODO
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void AST_ConstructorInvoke::encode(){

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
}
