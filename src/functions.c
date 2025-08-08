//
//  functions.c
//  Computer Programming Project
//
//  Created by Tomáš Buša on 16/06/2023.
//
#include <stdlib.h> // malloc, free
#include <stdio.h>
#include "functions.h"
#include "ijvm.h"
#include "util.h"

/*IVJM*/
byte_t *text;
uint32_t constSize;
uint32_t textSize;
uint32_t programCounter;
bool halted;
bool signalEncountered;
uint32_t *constValues;
FILE *in;
FILE *out;
FILE *fp;

/*Stack*/
struct stack {
    int32_t stackPointer;
    int32_t localVariable;
    word_t *stack;
};
struct stack operandStack;
uint64_t stackSize;

/*Heap*/
word_t * heap;
uint32_t freeHeap;
uint64_t heapSIZE;

/*Stack Initialization*/
void stackINIT(void) {
    struct operandStack;
    stackSize = 1024;
    programCounter = 0;
    operandStack.stackPointer = 300;
    operandStack.localVariable = 0;
    operandStack.stack = malloc(stackSize * sizeof(word_t));
    halted = false;
}

/*Heap Initialization*/
void heapINIT(void) {
    heapSIZE = 1024;
    heap = malloc(heapSIZE * sizeof(word_t));
}

/*IVJM file initialization*/
bool checkMagicNum(void) {
    const uint32_t magicDecimal = 501931949;
    uint8_t helper[4];
    fread(helper, sizeof(uint8_t), 4, fp);
    uint32_t fileMagicNum = read_uint32_t(helper);
    signalEncountered = false;
    if (fileMagicNum != magicDecimal) {
        return false;
    } else {
        return true;
    }
}

void readConstSize(void) {
    /*Pool origin can be ignored!*/
    uint32_t num;
    uint8_t poolOrg[4];
    fread(poolOrg, sizeof(uint8_t), 4, fp);
    num = read_uint32_t(poolOrg);

    uint8_t poolSize[4];
    fread(poolSize, sizeof(uint8_t), 4, fp);
    num = read_uint32_t(poolSize);
    constSize = num;

}

void readConstants(void) {
    uint32_t numOfConstants = constSize / 4;
    fseek(fp, 12, SEEK_SET);
    constValues = malloc(sizeof(uint32_t) * (numOfConstants + 10));

    for (int u = 0; u < numOfConstants; u++) {
        uint8_t helper[4];
        fread(helper, sizeof(uint8_t), 4, fp);
        uint32_t num = read_uint32_t(helper);
        constValues[u] = num;
    }
}

void readTextSize(void) {
    fseek(fp, constSize + 16, SEEK_SET); //move to the text pool size and read 4 bytes
    uint8_t helper[4];
    fread(helper, sizeof(uint8_t), 4, fp);
    textSize = read_uint32_t(helper);
}

void readText(void) {
    fseek(fp, constSize + 20, SEEK_SET); //move to the text pool and allocate memory for textSize bytes
    text = malloc(textSize + 10);
    fread(text, sizeof(uint8_t), textSize, fp);
}

/*HELPER FUNCTIONS*/
signed short readSignedShort(void) {
    uint8_t helper[2];
    helper[0] = text[programCounter + 1];
    helper[1] = text[programCounter + 2];
    int16_t shortArg = read_uint16_t(helper);
    return shortArg;
}

uint16_t readUnsignedShort(void) {
    uint8_t helper[2];
    helper[0] = text[programCounter + 1];
    helper[1] = text[programCounter + 2];
    uint16_t shortArg = read_uint16_t(helper);
    return shortArg;
}

/*STACK OPERATIONS*/
void push(word_t num) {
    while ((operandStack.stackPointer + 1) >= stackSize) {
        stackSize *= 2;
        operandStack.stack = realloc(operandStack.stack, stackSize * sizeof(word_t));
    }

    operandStack.stackPointer += 1;
    operandStack.stack[operandStack.stackPointer] = num;
}

word_t pop(void) {
    word_t num = operandStack.stack[operandStack.stackPointer];
    operandStack.stackPointer -= 1;
    return num;
}

word_t tos(void) {
    return operandStack.stack[operandStack.stackPointer];
}

/*IVJM INSTRUCTIONS*/
void opBIPUSH(void) {
    int8_t num = text[programCounter + 1];
    push(num);
    programCounter += 2;
}

void opDUP(void) {
    word_t topOfStack = tos();
    push(topOfStack);
    programCounter += 1;
}

void opERR(char * str) {
    printf("ERROR: %s\n", str);
    halted = true;
}

void opGOTO(void) {
    int16_t increment = readSignedShort();
    programCounter += increment;
}

void opHALT(void) {
    halted = true;
}

void opIADD(void) {
    word_t num1 = pop();
    word_t num2 = pop();
    word_t sum = num1 + num2;
    push(sum);
    programCounter += 1;
}

void opIAND(void) {
    word_t num1 = pop();
    word_t num2 = pop();
    word_t sum = num2 & num1;
    push(sum);
    programCounter += 1;
}

void opIFEQ(void) {
    int16_t increment = readSignedShort();
    word_t num = pop();
    if (num == 0) {
        programCounter += increment;
    } else {
        programCounter += 3;
    }
}

void opIFLT(void) {
    int16_t increment = readSignedShort();
    word_t num = pop();
    if (num < 0) {
        programCounter += increment;
    } else {
        programCounter += 3;
    }
}

void opIF_ICMPEQ(void) {
    int16_t increment = readSignedShort();
    word_t num1 = pop();
    word_t num2 = pop();
    if (num1 == num2) {
        programCounter += increment;
    } else {
        programCounter += 3;
    }
}

void opIINC(bool wide) {
    int8_t value;
    if (wide) {
        uint16_t index = readUnsignedShort();
        value = text[programCounter + 3];
        operandStack.stack[operandStack.localVariable + index] += value;
        programCounter += 4;
    } else {
        uint8_t index;
        index = text[programCounter + 1];
        value = text[programCounter + 2];
        operandStack.stack[operandStack.localVariable + index] += value;
        programCounter += 3;
    }
}

void opILOAD(bool wide) {
    word_t num;
    if (wide) {
        uint16_t index = readUnsignedShort();
        num = operandStack.stack[operandStack.localVariable + index];
        programCounter += 3;
    } else {
        uint8_t index = text[programCounter + 1];
        num = operandStack.stack[operandStack.localVariable + index];
        programCounter += 2;
    }

    push(num);
}

void opIN(void) {
    programCounter += 1;
    word_t input = fgetc(in);
    printf("INPUT: %d\n", input);
    if (input == EOF) {
        push(0);
    } else {
        push(input);
    }
}

void opINVOKEVIRTUAL(void) {
    uint16_t index = readUnsignedShort();
    uint32_t pos = get_constant(index);
    word_t prevPC = get_program_counter();
    word_t prevLV = operandStack.localVariable;
    programCounter = pos;

    /*read short of num of args*/
    programCounter -= 1;
    uint16_t argCount = readUnsignedShort();
    operandStack.localVariable = operandStack.stackPointer - argCount + 1;
    
    /*read size of local variables*/
    programCounter += 2;
    uint16_t localVarSize = readUnsignedShort();
    programCounter -= 1;

    operandStack.stackPointer += localVarSize;
    programCounter += 4;
    push(prevPC);
    operandStack.stack[operandStack.localVariable] = operandStack.stackPointer;
    push(prevLV);
}

void opIOR(void) {
    word_t num1 = pop();
    word_t num2 = pop();
    word_t sum = num2 | num1;
    push(sum);
    programCounter += 1;
}

void opIRETURN(void) {
    int pos = operandStack.stack[operandStack.localVariable];
    programCounter = operandStack.stack[pos];
    operandStack.stack[operandStack.localVariable] = tos();
    operandStack.stackPointer = operandStack.localVariable;
    operandStack.localVariable = operandStack.stack[pos + 1];
    programCounter += 3;
}

void opISTORE(bool wide) {
    word_t poppedVar = pop();
    if (wide) {
        uint16_t index = readUnsignedShort();
        operandStack.stack[operandStack.localVariable + index] = poppedVar;
        programCounter += 3;
    } else {
        uint8_t index;
        index = text[programCounter + 1];
        operandStack.stack[operandStack.localVariable + index] = poppedVar;
        programCounter += 2;
    }
}

void opISUB(void) {
    word_t num1 = pop();
    word_t num2 = pop();
    word_t sum = num2 - num1;
    push(sum);
    programCounter += 1;
}

void opLDC_W(void) {
    uint16_t index = readUnsignedShort();
    word_t constant = get_constant(index);
    push(constant);
    programCounter += 3;
}

void opNOP(void) {
    programCounter += 1;
}

void opOUT(void) {
    programCounter += 1;
    word_t popped = pop();
    fprintf(out, "%c", popped);
}

void opPOP(void) {
    pop();
    programCounter += 1;
}

void opSWAP(void) {
    word_t num1 = pop();
    word_t num2 = pop();
    push(num1);
    push(num2);
    programCounter += 1;
}

void opWIDE(void) {
    programCounter += 1;
    byte_t nextInstruction = text[programCounter];
    if (nextInstruction == OP_ILOAD) {
        opILOAD(true);
    } else if (nextInstruction == OP_ISTORE) {
        opISTORE(true);
    } else {
        opIINC(true);
    }
}

void opTAILCALL(void) {
    /*Get number of arguments of the called function*/
    uint16_t index = readUnsignedShort();
    uint8_t posOfCall = get_constant(index);
    uint8_t pos = posOfCall;

    programCounter = pos;
    programCounter -= 1;
    uint16_t argCount = readUnsignedShort();
    word_t prevPC = operandStack.stack[operandStack.stack[operandStack.localVariable]];
    word_t prevLV = operandStack.stack[operandStack.stack[operandStack.localVariable] + 1];

    for (int i = 0; i < argCount; i++) {
        operandStack.stack[operandStack.localVariable + i] = operandStack.stack[operandStack.stackPointer - argCount + 1 + i];
    }

    programCounter = posOfCall;
    //read short of num of args
    programCounter -= 1;

    //read size of local variables
    programCounter += 2;
    uint16_t localVarSize = readUnsignedShort();

    programCounter -= 1;

    operandStack.stackPointer += localVarSize;
    programCounter += 4;

    operandStack.stackPointer = operandStack.localVariable + argCount + localVarSize;
    push(prevPC);
    operandStack.stack[operandStack.localVariable] = operandStack.stackPointer;
    push(prevLV);
}

void opNEWARRAY(void) {
    uint32_t count = pop();
    while ((freeHeap + count) > heapSIZE) {
        heapSIZE *= 2;
        heap = realloc(heap, heapSIZE * sizeof(word_t));
    }

    push(freeHeap);
    heap[freeHeap + count] = -999999;
    freeHeap += count + 1;
    programCounter += 1;
}

void opIALOAD(void) {
    uint32_t arrayref = pop();
    uint32_t index = pop();
    uint32_t value;
    char errStr[] = "Heap array out of bound!";

    /*Bounds checking*/
    if (index < 0) {
        opERR(errStr);
    }

    for (int i = arrayref; i <= arrayref + index; i++) {
        if (heap[i] == -999999) {
            opERR(errStr);
        }
    }

    value = heap[arrayref + index];
    push(value);
    programCounter += 1;
}

void opIASTORE(void) {
    uint32_t arrayref = pop();
    uint32_t index = pop();
    int32_t value = pop();
    char errStr[] = "Heap array out of bound!";

    /*Bounds checking*/
    if (index < 0) {
        opERR(errStr);
    }

    for (int i = arrayref; i <= arrayref + index; i++) {
        if (heap[i] == -999999) {
            opERR(errStr);
        }
    }

    heap[arrayref + index] = value;
    programCounter += 1;
}
