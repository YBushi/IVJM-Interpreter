#include <stdio.h>  // for getc, printf
#include <stdlib.h> // malloc, free
#include "ijvm.h"
#include "util.h" // read this file for debug prints, endianness helper functions
#include "functions.h"


// see ijvm.h for descriptions of the below functions

/*Global variables*/
struct stack {
    int32_t stackPointer;
    int32_t localVariable;
    word_t * stack;
};

/*IVJM*/
struct stack operandStack;
byte_t * text;
uint32_t programCounter;
uint32_t textSize;
bool halted;
bool signalEncountered;
uint32_t *constValues;
FILE *in;
FILE *out;
FILE *fp;

/*Stack*/
struct stack operandStack;

/*Heap*/
word_t *heap;

void set_input(FILE *fp) {
    in = fp;
}

void set_output(FILE *fp) {
    out = fp;
}

int init_ijvm(char *binary_path) {
    in = stdin;
    out = stdout;

    /*Open file and check if it opened correctly*/
    fp = fopen(binary_path, "rb");

    if (fp == NULL) {
        printf("Couldn't open file!.\n");
        return 1;
    };

    /*Initialize the stack, stackpointer, programCounter, heap*/
    stackINIT();
    heapINIT();

    /*Check if we are working with binary file*/
    if (!checkMagicNum()) {
        char errString[] = "This is not a binary file!";
        opERR(errString);
        return -1;
    }

    /*Read constant size and constants into an array*/
    readConstSize();
    readConstants();

    /*Read text size and read the data text into an array*/
    readTextSize();
    readText();
    return 0;
}

void destroy_ijvm(void) {
    /*Free all the memory allocated by malloc*/
    free(operandStack.stack);
    free(constValues);
    free(text);
    free(heap);
    fclose(fp);
}

byte_t *get_text(void) {
    return text;
}

uint32_t get_text_size(void) {
    return textSize;
}

word_t get_constant(int i) {
    return constValues[i];
}

uint32_t get_program_counter(void) {
    return programCounter;
}

bool finished(void) {
    return halted;
}

word_t get_local_variable(int i) {
    return operandStack.stack[operandStack.localVariable + i];
}

void step(void) {
    byte_t currInstruction = text[programCounter];
    switch (currInstruction) {
        case OP_BIPUSH: {
            opBIPUSH();
            break;
        }

        case OP_DUP: {
            opDUP();
            break;
        }

        case OP_GOTO: {
            opGOTO();
            break;
        }

        case OP_HALT: {
            opHALT();
            break;
        }

        case OP_IADD: {
            opIADD();
            break;

        }

        case OP_IAND: {

            opIAND();
            break;
        }

        case OP_IFEQ: {
            opIFEQ();
            break;
        }

        case OP_IFLT: {
            opIFLT();
            break;
        }

        case OP_IF_ICMPEQ: {
            opIF_ICMPEQ();
            break;
        }

        case OP_IINC: {
            opIINC(false);
            break;
        }

        case OP_ILOAD: {
            opILOAD(false);
            break;
        }

        case OP_IN: {
            opIN();
            break;
        }

        case OP_INVOKEVIRTUAL: {
            opINVOKEVIRTUAL();
            break;
        }

        case OP_IOR: {
            opIOR();
            break;
        }

        case OP_IRETURN: {
            opIRETURN();
            break;
        }

        case OP_ISTORE: {
            opISTORE(false);
            break;
        }

        case OP_ISUB: {
            opISUB();
            break;
        }

        case OP_LDC_W: {
            opLDC_W();
            break;
        }

        case OP_NOP: {
            opNOP();
            break;
        }

        case OP_OUT: {
            opOUT();
            break;
        }

        case OP_POP: {
            opPOP();
            break;
        }

        case OP_SWAP: {
            opSWAP();
            break;
        }

        case OP_WIDE: {
            opWIDE();
            break;
        }

        case OP_TAILCALL: {
            opTAILCALL();
            break;
        }

        case OP_NEWARRAY: {
            opNEWARRAY();
            break;
        }

        case OP_IALOAD: {
            opIALOAD();
            break;
        }

        case OP_IASTORE: {
            opIASTORE();
            break;
        }
    }

    if ((get_program_counter() >= textSize) || (signalEncountered)) {
        halted = true;
    }
}

void run(void) {
    while (!finished()) {
        step();
    }
}

byte_t get_instruction(void) {
    return text[get_program_counter()];
}

// Below: methods needed by bonus assignments, see ijvm.h

int get_call_stack_size(void) {
    int32_t size = operandStack.stackPointer;
    return size;
}

// Checks if reference is a freed heap array. Note that this assumes that
//
//bool is_heap_freed(word_t reference)
//{
// TODO: implement me
// return 0;
//}

