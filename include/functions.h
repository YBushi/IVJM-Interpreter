//
//  functions.h
//  Computer Programming Project
//
//  Created by Tomáš Buša on 13/06/2023.
//

#ifndef functions_h
#define functions_h
#include "ijvm.h"

/*IVJM*/
/*Stack Initialization*/
void stackINIT(void);

/*Heap Initialization*/
void heapINIT(void);

/*IVJM file initialization*/
bool checkMagicNum(void);
void readConstSize(void);
void readConstants(void);
void readTextSize(void);
void readText(void);

/*HELPER FUNCTIONS*/
int16_t readSignedShort(void);
uint16_t readUnsignedShort(void);

/*STACK OPERATIONS*/
void push(word_t num);
word_t pop(void);
word_t tos(void);

/*IVJM INSTRUCTIONS*/
void opBIPUSH(void);
void opDUP(void);
void opERR(char *str);
void opGOTO(void);
void opHALT(void);
void opIADD(void);
void opIAND(void);
void opIFEQ(void);
void opIFLT(void);
void opIF_ICMPEQ(void);
void opIINC(bool wide);
void opILOAD(bool wide);
void opIN(void);
void opINVOKEVIRTUAL(void);
void opIOR(void);
void opIRETURN(void);
void opISTORE(bool wide);
void opISUB(void);
void opLDC_W(void);
void opNOP(void);
void opOUT(void);
void opPOP(void);
void opSWAP(void);
void opWIDE(void);
void opTAILCALL(void);
void opNEWARRAY(void);
void opIALOAD(void);
void opIASTORE(void);

#endif /* functions_h */
