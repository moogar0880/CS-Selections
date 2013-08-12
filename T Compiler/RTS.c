#include <stdio.h>

void RTS_outputInteger(int i){
  printf("%d\n", i);
}

void RTS_printDivideByZeroError(int i){
	printf("ERROR: Divide By Zero Error on line %d\n", i);
}

void RTS_breakPointPrint(){
	printf("Reached this point\n");
}

void RTS_checkForNullReference(int line, int i){
	if( i == 0 )
		printf("ERROR: Null Reference on line %d\n", line );
}

void RTS_checkCast(int lineNumber, void* vmt, void* objectReference){
	printf("\n");
}

void RTS_outOfMemoryError(int i){
	printf("ERROR: Out of memory on line %d\n", i);
}