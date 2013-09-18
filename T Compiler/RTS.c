#include <stdlib.h>
#include <stdio.h>

void RTS_outputInteger(int i){
  printf("%d\n", i);
}

void RTS_printDivideByZeroError(int i){
	printf("ERROR: Divide By Zero Error on line %d\n", i);
	exit(1);
}

void RTS_checkForNullReference(int line, int i){
	if( i == 0 ){
		printf("ERROR: Null Reference on line %d\n", line );
		exit(1);
	}
}

void RTS_checkCast(int lineNumber, void* vmt, void* objectReference){
	//printf("RTS_checkCast\n");
}

void RTS_outOfMemoryError(int i){
	printf("ERROR: Out of memory on line %d\n", i);
	exit(1);
}

void RTS_reverseArgumentsOnStack(){
	// printf( "RTS_reverseArgumentsOnStack called \n" );
}