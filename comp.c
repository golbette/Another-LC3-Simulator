#include <stdio.h>
#include "bstr.h"
#include "comp.h"

void COMP_Init(Computer *cmp) {
    int r, m;
    BSTR_SetValue(&(cmp->pc),0,16);
    BSTR_SetValue(&(cmp->ir),0,16);
    BSTR_SetValue(&(cmp->cc),0,3);
    for (r = 0; r < 8; r++) {
        BSTR_SetValue(&(cmp->reg[r]),r,16);  /* put some interesting data in registers */
    }
    for (m = 0; m < MAXMEM; m++) {
        BSTR_SetValue(&(cmp->mem[m]),0,16);
    }
}



void COMP_LoadWord(Computer* comp, int addr, BitString word) {
    comp->mem[addr] = word;
}


void COMP_ExecuteNot(Computer *comp) {
    BitString drBS, srBS;
    BSTR_Substring(&drBS,comp->ir,4,3);
    BSTR_Substring(&srBS,comp->ir,7,3);
    comp->reg[ BSTR_GetValue(drBS) ] = comp->reg[ BSTR_GetValue(srBS) ];
    BSTR_Invert( & comp->reg[ BSTR_GetValue(drBS)  ]  );
    
    if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) < 0)
    {
        char ccBits[] = {'1', '0', '0'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) > 0)
    {
        char ccBits[] = {0, 0, 1};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) == 0)
    {
        char ccBits[] = {0, 1, 0};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
}

void COMP_ExecuteAdd(Computer *comp) {
    //printf("Executing ADD.");
    BitString drBS, srBS1, srBS2, imm;
    int decVal1, decVal2, result;
    BSTR_Substring(&drBS,comp->ir,4,3);
    BSTR_Substring(&srBS1,comp->ir,7,3);
    BSTR_Substring(&imm, comp->ir,10,1);
    
    //check if immediate mode
    if (BSTR_GetValue(imm) > 0)
    {
        //here, srBS2 is just the value we want already
        BSTR_Substring(&srBS2,comp->ir,11,5);
        decVal1 = BSTR_GetValueTwosComp(srBS2);
        
    }
    else
    {
        BSTR_Substring(&srBS2,comp->ir,13,3);
        //here, get 2's c value from the register at SR2
        decVal1 = BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(srBS2)]);
    }

    //get the 2's c value from the register at SR1 every time
    decVal2 = BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(srBS1)]);
    
    result = decVal1+decVal2;
    //printf("%d", result);
    
    //emulate overflow
    if (result > 32767)
    {
        result = result - 32768*2;
    }
    
    //emulate underflow
    if (result < -32768)
    {
        result = result + 32767*2;
    }
    
    BSTR_SetValueTwosComp(&comp->reg[BSTR_GetValue(drBS)], result, 16);
    
    
    if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) < 0)
    {
        char ccBits[] = {'1', '0', '0'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) > 0)
    {
        char ccBits[] = {'0', '0', '1'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) == 0)
    {
        char ccBits[] = {'0', '1', '0'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
}

void COMP_ExecuteLD(Computer *comp) {
    BitString drBS, off;
    int decOff, decAddr;
    BSTR_Substring(&drBS,comp->ir,4,3);
    BSTR_Substring(&off,comp->ir,7,9);
    
    decOff = BSTR_GetValueTwosComp(off);
    decAddr = BSTR_GetValue(comp->pc) + decOff;
    
    //underflow and overflow on address??
    if (decAddr < 0)
    {
        decAddr = 50 + decAddr;
    }
    
    if (decAddr > 50)
    {
        decAddr = 50 - decAddr;
    }
    
    BSTR_Copy(&comp->reg[BSTR_GetValue(drBS)], comp->mem[decAddr]);
    
    if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) < 0)
    {
        char ccBits[] = {'1', '0', '0'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) > 0)
    {
        char ccBits[] = {'0', '0', '1'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    else if (BSTR_GetValueTwosComp(comp->reg[BSTR_GetValue(drBS)]) == 0)
    {
        char ccBits[] = {'0', '1', '0'};
        BSTR_SetBits(&(comp->cc), ccBits);
    }
    
    
}

void COMP_ExecuteBR(Computer *comp) {
    BitString cond, off, subCond, subCC;
    int decOff, decAddr;
    BSTR_Substring(&cond,comp->ir,4,3);
    BSTR_Substring(&off,comp->ir,7,9);
    
    decOff = BSTR_GetValueTwosComp(off);
    decAddr = BSTR_GetValue(comp->pc) + decOff;
    
    //underflow and overflow on address??
    if (decAddr < 0)
    {
        decAddr = 50 + decAddr;
    }
    
    if (decAddr > 50)
    {
        decAddr = 50 - decAddr;
    }
    
    //N
    BSTR_Substring(&subCond, cond, 0, 1);
    BSTR_Substring(&subCC, comp->cc, 0, 1);
    if (BSTR_GetValue(subCond) == 1 && BSTR_GetValue(subCC) == 1)
    {
       BSTR_SetValue(&comp->pc, decAddr, 16); 
    }
    
    //Z
    BSTR_Substring(&subCond, cond, 1, 1);
    BSTR_Substring(&subCC, comp->cc, 1, 1);
    if (BSTR_GetValue(subCond) == 1 && BSTR_GetValue(subCC) == 1)
    {
       BSTR_SetValue(&comp->pc, decAddr, 16); 
    }
    
    //P
    BSTR_Substring(&subCond, cond, 2, 1);
    BSTR_Substring(&subCC, comp->cc, 2, 1);
    if (BSTR_GetValue(subCond) == 1 && BSTR_GetValue(subCC) == 1)
    {
       BSTR_SetValue(&comp->pc, decAddr, 16); 
    }
    
    
    
    
}

void COMP_Execute(Computer* comp) {
    BitString opCode;
    int opCodeInt;
    
    /* use the PC to load current instruction from memory into IR */
    
    while (1 > 0)
    {
        comp->ir = comp->mem[BSTR_GetValue(comp->pc)];
        
        BSTR_AddOne(&comp->pc);
        
        BSTR_Substring(&opCode,comp->ir,0,4);  /* isolate op code */
        opCodeInt = BSTR_GetValue(opCode); /* get its value */
        
        /*what kind of instruction is this? */
        if (opCodeInt == 9) {   // NOT
            COMP_ExecuteNot(comp);
        }
        
        if (opCodeInt == 1) { //ADD
            COMP_ExecuteAdd(comp);
        }
        
        if (opCodeInt == 2) { //LD
            COMP_ExecuteLD(comp);
        }
        
        if (opCodeInt == 0) { //BR
            COMP_ExecuteBR(comp);
        }
        
        if (opCodeInt == 15) //TRAP
        {
            BitString routineCode; 
            BSTR_Substring(&routineCode, comp->ir,8,8);
            
            //HALT
            if (BSTR_GetValue(routineCode) == 37)
            {
                return;
            }
            
            //OUT
            if (BSTR_GetValue(routineCode) == 33)
            {
                int chrval;
                chrval = BSTR_GetValue(comp->reg[0]);
                printf("%c\n", chrval);
            }
            
        }
    }
    
}


void COMP_Display(Computer cmp) {
    int r, m;
    printf("\n");
    
    printf("PC ");
    BSTR_Display(cmp.pc,1);
    printf("   ");
    
    
    printf("IR ");
    BSTR_Display(cmp.ir,1);
    printf("   ");
    
    
    printf("CC ");
    BSTR_Display(cmp.cc,1);
    printf("\n");
    
    
    for (r = 0; r < 8; r++) {
        printf("R%d ",r);
        BSTR_Display(cmp.reg[r], 1);
        if (r % 3 == 2)
            printf("\n");
        else
            printf("   ");
    }
    printf("\n");
    for (m = 0; m < MAXMEM; m++) {
        printf("%3d ",m);
        BSTR_Display(cmp.mem[m], 1);
        
        if (m % 3 == 2)
            printf("\n");
        else
            printf("    ");
    }
    printf("\n");
}

