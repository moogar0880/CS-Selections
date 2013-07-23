// Jan 2008
// AST methods for code generation

#include <stdlib.h>
#include <cstdio>
#include <iostream>
using namespace std;

#include "AST.h"
#include "Type.h"

// global type module is in main.cxx
extern TypeModule* types;
// global class list in AST.cxx
extern std::vector<AST_Class*> globalClassList;
int labelCount = 0;
int whileCount = 0;
// output the prelude code
void encodeInitialize(){
  cout << "#\tPrologue\n";
  cout << "\t.text\n";
  cout << "\t.align 4\n";
  cout << "\t.globl\tmain\n";
  cout << "main:\n";
  cout << "\tpushl\t%ebp\n";
  cout << "\tmovl\t%esp, %ebp\n";
}

// output the epilogue code
void encodeFinish(){
  cout << "#\tEpilogue\n";
  cout << "main$exit:\n";
  cout << "\tpopl\t%ebp\n";
  cout << "\tret\n";
}

void AST_List::encode(){
  item->encode();
  if (restOfList != NULL) restOfList->encode();
}

void AST_IntegerLiteral::encode(){
  cout << "#\tIntegerLiteral\n";
  cout << "\tpushl\t$" << value << endl;
}

void AST_Variable::encode(){
  cout << "#\tVariable\n";
  cout << "\tpushl\t$mainvar$" << name << endl;
}

void AST_VariableList::encode(){
  cout << "#\tVariableList\n";
  cout << "mainvar$" << ((AST_Variable*)(item))->name << ": .long 0\n";
  if (restOfList != NULL) restOfList->encode();
}

void AST_MainFunction::encode(){
  cout << "#\tMainFunction\n";
  list->encode();
}

void AST_Declaration::encode(){
  cout << "#\tDeclaration\n";
  cout << "\t.data\n";
  list->encode();
  cout << "\t.text\n";
}

void AST_ExpressionStatement::encode(){
  express->encode();
  cout << "#\tExpressionStatement\n";
  cout << "\taddl\t$4, %esp\n";
}

void AST_Assignment::encode(){
  lhs->encode();
  rhs->encode();
  cout << "#\tAssignment\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%edx\n";
  cout << "\tmovl\t%eax, (%edx)\n";
  cout << "\tpushl\t%eax\n";
  //cout << "\taddl\t$4, %esp\n";
}

void AST_Print::encode(){
  var->encode();

  if (var->type == types->intType()){
    cout << "#\tPrint int\n";
    cout << "\tcall\tRTS_outputInteger\n";
    cout << "\taddl\t$4, %esp\n";
  }
  else if (var->type == types->errorType()){
    // do nothing: there was a semantic error
  }
  else{
    cerr << line << ": BUG in AST_Print::encode: unknown type\n";
    exit(-1);
  }
}

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

void AST_Subtract::encode(){
  left->encode();
  right->encode();

  if (type == types->intType())
  {
    cout << "#\tSubtract int\n";
    cout << "\tpopl\t%edx\n";
    cout << "\tpopl\t%eax\n";
    cout << "\tsubl\t%edx, %eax\n";
    cout << "\tpushl\t%eax\n";
  }
  else if (type == types->errorType())
  {
    // do nothing: there was a semantic error
  }
  else
  {
    cerr << line << ": BUG in AST_Subtract::encode: unexpected type\n";
    exit(-1);
  }
}

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

void AST_Block::encode(){
  cout << "#\tBlock\n";
  list->encode();
}

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
  cout << "\tje\tL" << l2 << "\n";
  cout << "L" << l1 << ":\n";
  if( elstat != NULL )
    elstat->encode();
  cout << "L" << l2 << ":\n";
}

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

void AST_Return::encode(){
  cout << "#\tReturn\n";
  if( var == NULL ){
    var->encode();
    cout << "\tpopl\t%eax\n";
  }
  cout << "\tjmp\tmain$exit\n";
}

void AST_Continue::encode(){
  int l1 = labelCount - 2;
  cout << "#\tContinue\n";
  cout << "\tjmp\tL" << l1 << "\n";
}

void AST_Break::encode(){
  int l1 = whileCount - 1;
  cout << "#\tBreak\n";
  cout << "\tjmp\tW" << l1 << "\n";
}

void AST_Null::encode(){
  cout << "#\tNull\n";
  cout << "\tpushl\t$0\n";
}

void AST_CompilationUnit::encode(){
  // generate Object Class code immediately for ease
  std::vector<AST_Class*>::iterator it;
  it = globalClassList.begin();
  (*it)->encode();

  if( list != NULL )
    list->encode();
  main->encode();
}

void AST_Class::encode(){
  if( parent != NULL ){
    char* parentName = ((TypeClass*)parent)->toString();
    cout << "#\t" << name << " Class VMT\n";
    cout << "\t.data\n";
    cout << name << "$VMT:\n";
    cout << "\t.long\t" << parentName << "$VMT\n";
    cout << "\t.text\n";
  }
  else{
    cout << "#\tObject Class VMT\n";
    cout << "\t.data\n";
    cout << name << "$VMT:\n";
    cout << "\t.long\t0\n";
    cout << "\t.text\n";
  }
}

void AST_Cast::encode(){
  char* targetName = ((AST_Variable*)(expr))->name;
  cout << "#\tClass Case\n";
  cout << "\tpushl\t$" << targetName << "$VMT\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkCast\n";
  cout << "\taddl\t$8, %esp\n";
}

void AST_FieldReference::encode(){
  cout << "#\tField Reference\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_checkForNullReference\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tpopl\t%eax\n";
  cout << "\taddl\t$[offset], %eax\n";
  cout << "\tpushl\t%eax\n";
}

void AST_FieldDeclaration::encode(){

}

void AST_ClassInstance::encode(){
  int length = 0; //length of the object in 4-byte words
  char* className = (((TypeClass*)type)->toString());
  cout << "#\t" << className << " Class Instance\n";
  cout << "\tpushl\t$4\n";
  cout << "\tpushl\t$" << length << "\n";
  cout << "\tcall\tcalloc\n";
  cout << "\taddl\t$8, %esp\n";
  cout << "\tcmpl\t$0, %eax\n";
  cout << "\tjne\t" << ++labelCount << "\n";
  cout << "\tpushl\t$" << line << "\n";
  cout << "\tcall\tRTS_outOfMemoryError\n";
  cout << labelCount << ":\n";
  cout << "\tmovl\t$" << className << "$VMT, (%eax)\n";
  cout << "\tpushl\t%eax\n";
}

void AST_ArgumentsList::encode(){

}

void AST_EmptyStatement::encode(){

}

void AST_Convert::encode(){
  left->encode();

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
    cerr << line << ": BUG in AST_Convert::encode: unexpected type\n";
    exit(-1);
  }
}

void AST_Deref::encode(){
  left->encode();

  cout << "#\tDeref\n";
  cout << "\tpopl\t%eax\n";
  cout << "\tmovl\t(%eax), %eax\n";
  cout << "\tpushl\t%eax\n";
}
