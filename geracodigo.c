#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geracodigo.h"

#define TRUE 1
#define FALSE 0

//MARK: Opcodes Aritméticos
#define ADD_VARIABLES {0x8B, 0x4d, 0x00, 0x01, 0x4d, 0x00} //checked
#define SUB_VARIABLES {0x8B, 0x4d, 0x00, 0x29, 0x4d, 0x00} //checked
#define MUL_VARIABLES {0x8B, 0x4d, 0x00, 0x8b, 0x55, 0x00, 0x0f, 0xaf, 0xd1, 0x89, 0x55, 0x00} //checked

#define ADD_PARAMETERS {0x01, 0x00}
#define SUB_PARAMETERS {0x29, 0x00}
#define MUL_PARAMETERS {0x0F, 0xAF, 0x00}

#define ADD_CONST_PARAMETER {0x81, 0x00, 0x00, 0x00, 0x00, 0x00}
#define SUB_CONST_PARAMETER {0x81, 0xEF, 0x00, 0x00, 0x00, 0x00}
#define MUL_CONST_PARAMETER {0x69, 0xFF, 0x00, 0x00, 0x00, 0x00}

#define ADD_CONST_VAR {0x81, 0x45, 0xFC, 0x00, 0x00, 0x00, 0x00}
#define SUB_CONST_VAR {0x81, 0x6D, 0xFC, 0x00, 0x00, 0x00, 0x00}
#define MUL_CONST_VAR {0x8B, 0x4D, 0xFC, 0x69, 0xC9, 0x00, 0x00, 0x00, 0x00, 0x89, 0x4D, 0xFC}

#define ADD_VAR_PARAMETER {0x01, 0x75, 0x00} //checked
#define SUB_VAR_PARAMETER {0x29, 0x75, 0x00} //checked
#define MUL_VAR_X_PARAMETER {0x8b, 0x4d, 0x00, 0x0f, 0xaf, 0xcf, 0x89, 0x4d, 0x00} //var receives
#define MUL_PARAMETER_X_VAR {0x0f, 0xaf, 0x7d, 0x00} //parameter receives

//MARK: Opcodes de Retorno
#define RETURN_PARAMETER {0x89, 0x00, 0xC3}
#define RETURN_VAR {0x8B, 0x45, 0x00, 0xC3}
#define RETURN_CONSTANT {0xB8, 0x00, 0x00, 0x00, 0x00, 0xC3}

//MARK: Opcodes de Atribuição
#define ATT_VAR_TO_PARAM {0x8B, 0x7D, 0x00}
#define ATT_PARAM_TO_PARAM {0X89, 0X00}
#define ATT_CONST_TO_PARAM {0xBE, 0x00, 0x00, 0x00, 0x00}
#define ATT_VAR_TO_VAR {0x8B, 0x4D, 0x00, 0x89, 0x4D, 0x00}
#define ATT_PARAM_TO_VAR {0x89, 0x7D, 0x00}
#define ATT_CONST_TO_VAR {0xC7, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00}

//MARK: Opcodes de Desvio
#define GO {0xE9, 0x00, 0x00, 0x00, 0x00}

#define IF_PARAM {0x83, 0xfe, 0x00, 0x74, 0x09, 0x7c, 0x02, 0xeb, 0x0a, 0xe9, 0x00, 0x00, 0x00, 0x00, 0xe9, 0x00, 0x00, 0x00, 0x00}
#define IF_VAR {0x83, 0x7D, 0x00, 0x00, 0x74, 0x09, 0x7c, 0x02, 0xeb, 0x0a, 0xe9, 0x00, 0x00, 0x00, 0x00, 0xe9, 0x00, 0x00, 0x00, 0x00}

#define varIndexAsByte(index) (unsigned char)(-4 * index)

union InsideInt {
    int i;
    unsigned char c[4];
};

struct goInstruction {
    int from;
    int to;
};

struct ifInstruction {
    int from;
    int n1;
    int n2;
    int isVar;
};

static unsigned char* currentInstruction;

//Headers das operações aritméticas
void operate(char varp, int idx0, char varpc, int idx1, char operation);

void operateVars(int lhs, int rhs, char operation);
void addVars(int lhs, int rhs);
void subVars(int lhs, int rhs);
void mulVars(int lhs, int rhs);

void operateParameters(int lhs, int rhs, char operation);
void addParameters(int lhs, int rhs);
void subParameters(int lhs, int rhs);
void mulParameters(int lhs, int rhs);

void operateVarParameter(int parameter, int var, int parameterReceives, char operation);
void addVarParameter(int parameter, int var, int parameterReceives);
void subVarParameter(int parameter, int var, int parameterReceives);
void mulVarParameter(int parameter, int var, int parameterReceives);

void operateConstParameter(int p, int c, char operation);

void operateConstVar(int v, int c, char operation);

//Operação de Retorno
void returnValue(int vpc, char type);

//Operações de Atribuição
void attribute(char varp, int idx0, char varpc, int idx1);
void attVarToParam(int param, int var);
void attParamToParam(int lhs, int rhs);
void attConstToParam(int lhs, int rhs);
void attVarToVar(int lhs, int rhs);
void attParamToVar(int lhs, int rhs);
void attConstToVar(int lhs, int rhs);

//Desvio Incondicional
void goOperation(int from, int to, int instructionPosition, struct goInstruction *goInstructions);
void completeGoOperations(struct goInstruction *allInstructions, int *lineOffset, int nInstructions, unsigned char *codigo);

//Desvio Condicional
void ifOperation(char c, int idx0, int from, int n1, int n2, int instructionPosition, struct ifInstruction *ifInstructions);
void completeIfOperations(struct ifInstruction *allInstructions, int *lineOffset, int nInstructions, unsigned char *codigo);

//Auxiliares
void printInstruction(unsigned char first[], int number);
void appendInstructions(unsigned char **currInstruction, unsigned char newInstruction[], int nInstructions);
int beginningOffset(int ofLine, int* array);
int endingOffset(int ofLine, int* array);
void printGoInstructions(struct goInstruction* goInstructions, int nGoInstructions);

funcp geraCodigo (FILE *f, unsigned char codigo[]) {
    struct goInstruction allGoInstructions[20];
    struct ifInstruction allIfInstructions[20];
    currentInstruction = codigo; //currInstruction vai apontar para o byte seguinte à última instrução adicionada
    unsigned char* lastInstruction = codigo;
    int line = 1;
    int  c;
    int lineOffset[20];
    int nGoInstructions = 0, nIfInstructions = 0;
    lineOffset[0] = 0;
    while ((c = fgetc(f)) != EOF) {
        switch (c) {
            case 'r': { /* retorno */
                char var0;
                int idx0;
                if (fscanf(f, "et %c%d", &var0, &idx0) != 2)
                    exit(1);
                returnValue(idx0, var0);
                break;
            }
            case 'v':
            case 'p': { /* atribuiÃ§Ã£o e op. aritmetica */
                char var0 = c, var1, op;
                int idx0, idx1;
                
                if (fscanf(f, "%d %c= %c%d", &idx0, &op, &var1, &idx1) != 4)
                    exit(1);
                if (op == ':') {
                    attribute(var0, idx0, var1, idx1);
                } else {
                    operate(var0, idx0, var1, idx1, op);
                }
                
                break;
            }
            case 'i': { /* desvio condicional */
                char var0;
                int idx0, n1, n2;
                if (fscanf(f, "f %c%d %d %d", &var0, &idx0, &n1, &n2) != 4)
                    exit(1);
                //MARK: Terminar essa chamada
                ifOperation(var0, idx0, line, n1, n2, nIfInstructions, allIfInstructions);
                nIfInstructions += 1;
                break;
            }
            case 'g': { /* desvio incondicional */
                int n1;
                if (fscanf(f, "o %d", &n1) != 1)
                    exit(1);
                goOperation(line, n1, nGoInstructions, &allGoInstructions);
                nGoInstructions += 1;
                break;
            }
            default:  exit(1);
        }
        if(line != 20) {
            lineOffset[line] = (int)((long)currentInstruction - (long)lastInstruction);
            lineOffset[line] += lineOffset[line - 1];
        }
        lastInstruction = currentInstruction;
        line ++;
        fscanf(f, " ");
    }
    completeGoOperations(allGoInstructions, lineOffset, nGoInstructions, codigo);
    completeIfOperations(allIfInstructions, lineOffset, nIfInstructions, codigo);
    printInstruction(codigo, 80);
    return 0;
}


//MARK: Operação aritmética geral
void operate(char varp, int idx0, char varpc, int idx1, char operation) {
    if (varp == 'v') {
        if(varpc == 'p') {
            operateVarParameter(idx1, idx0, FALSE, operation);
        } else if (varpc == 'v') {
            operateVars(idx0, idx1, operation);
        } else {
            operateConstVar(idx0, idx1, operation);
        }
    } else {
        if(varpc == 'v') {
            operateVarParameter(idx0, idx1, TRUE, operation);
        } else if (varpc == 'p') {
            operateParameters(idx0, idx1, operation);
        } else {
            operateConstParameter(idx0, idx1, operation);
        }
    }
}

//MARK: operações aritméticas variável-variável

void operateVars(int lhs, int rhs, char operation) {
    switch (operation) {
        case '*':
            mulVars(lhs, rhs);
            break;
        case '+':
            addVars(lhs, rhs);
            break;
        case '-':
            subVars(lhs, rhs);
    }
}

/**
 esta função gera o valor em opcode que correponde
 a adicionar as variáveis lhs (left hand side) e rhs(right hand side),
 e jogar o resultado em lhs.
 
 isso ocorre considerando que, seja a variável local vi, ela está alocada
 em -4i(%rsp)
 
 o código de máquina gerado corresponde às seguintes instruções em Assembly:
 movl -4rhs(%rbp), %ecx
 add %ecx, -4lhs(%rbp)
 */
void addVars(int lhs, int rhs) {
    char receiver, added;
    unsigned char instructions[6] = ADD_VARIABLES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instructions[5] = receiver;
    instructions [2] = added;
    printInstruction(instructions, 6);
    appendInstructions(&currentInstruction, instructions, 6);
}

/**
 esta função gera o valor em opcode que correponde
 a subtrair a variável rhs (right hand side) da variável lhs(left hand side),
 e jogar o resultado em lhs.
 
 isso ocorre considerando que, seja a variável local vi, ela está alocada
 em -4i(%rsp)
 
 o código de máquina gerado corresponde às seguintes instruções em Assembly:
 movl -4rhs(%rbp), %ecx
 sub %ecx, -4lhs(%rbp)
 */
void subVars(int lhs, int rhs) {
    char receiver, added;
    unsigned char instructions[6] = SUB_VARIABLES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instructions[5] = receiver;
    instructions [2] = added;
    appendInstructions(&currentInstruction, instructions, 6);
}

/**
 A função abaixo gera o opcode correspondente ao seguinte código em assembly:
 movl -4rhs(%rbp), %ecx
 movl -4lhs(%rbp), %edx
 imul %ecx, %edx
 movl %edx, -4lhs(%rbp)
 
 esse código multiplica os valores das variáveis de índice lhs e rhs, e joga o resultado em lhs
 */
void mulVars(int lhs, int rhs) {
    char receiver, added;
    unsigned char instructions[12] = MUL_VARIABLES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instructions[11] = receiver;
    instructions[5] = receiver;
    instructions[2] = added;
    appendInstructions(&currentInstruction, instructions, 12);
}

//MARK: operações aritméticas parâmetro-variável

void operateVarParameter(int parameter, int var, int parameterReceives, char operation) {
    switch (operation) {
        case '+':
            addVarParameter(parameter, var, parameterReceives);
            break;
        case '-':
            subVarParameter(parameter, var, parameterReceives);
            break;
        case '*':
            mulVarParameter(parameter, var, parameterReceives);
    }
}

/**
 Essa função gera o opcode correspondente a uma operação de soma entre uma variável local e um parâmetro
 
 Foi possível observar que, se a variável local recebe o resultado da operação, o valor do primeiro byte da instrução é 0x01, e caso contrário, 0x03
 
 Também soi possível observar que, representando o parâmetro 1 com %edi e o parâmetro 2 com %esi, quando o p1 está envolvido, o segundo byte da instrução é 0x7d,
 mas quando o p2 está envolvido, o segundo byte é 0x75
 
 Por fim, observou-se que o último byte da instrução é igual ao valor em complemento de 2 de -4v, onde v é o índice da variável local envolvida
 */

void addVarParameter(int parameter, int var, int parameterReceives) {
    unsigned char instructions[3] = ADD_VAR_PARAMETER;
    
    //definindo o primeiro byte da instrução
    if (parameterReceives) {
        instructions[0] = 0x03;
    } else {
        instructions[0] = 0x01;
    }
    
    //definindo o segundo byte da instrução
    if (parameter == 1) {
        instructions[1] = 0x7D;
    } else if (parameter == 2) {
        instructions[2] = 0x75;
    }
    
    //definindo o terceiro byte da instrução
    instructions[2] = (unsigned char)(-4 * var);
    appendInstructions(&currentInstruction, instructions, 3);
}

/**
 Essa função gera o opcode correspodente a uma operação de subtração entre uma variável e um parâmetro
 
 Nós observamos que, quando a variável está recebendo, o primeiro byte da instrução é 0x29. Por outro lado, quando
 o parâmetro está recebendo, o primeiro byte da instrução é 0x2b
 
 Quando o parâmetro é o p1 (representado por %edi), o segundo byte da instrução é 0x7d. Quando o parâmetro é p2 (%esi), o segundo byte é 0x75
 
 Por fim, o último byte é o complemento de dois de -4v, onde v é o índice da variável
 */

void subVarParameter(int parameter, int var, int parameterReceives) {
    unsigned char instructions[3] = SUB_VAR_PARAMETER;
    
    //definindo o primeiro byte da instrução
    if (parameterReceives) {
        instructions[0] = 0x2B;
    } else {
        instructions[0] = 0x29;
    }
    
    //definindo o segundo byte da instrução
    if (parameter == 1) {
        instructions[1] = 0x7D;
    } else {
        instructions[1] = 0x75;
    }
    
    //definindo o último byte da instrução
    instructions[2] = (unsigned char)(-4 * var);
    appendInstructions(&currentInstruction, instructions, 3);
}

/**
 Para esta função o código em assembly para quando a variável recebe e quando o parâmetro recebe são diferentes.
 
 Para quando a variável recebe, o código é o seguinte:
 movl -4v(%rbp), %ecx
 imull %param, %ecx
 movl %ecx, -4v(%rbp)
 
 E para quando o parâmetro recebe:
 movl -4v(%rbp), %ecx
 imull %ecx, %param
 
 Onde %param pode ser o p1 (%edi) ou (%esi), e v é o índice da variável.
 
 Quando a variável recebe, o opcode "geral" é definido por 'MULVARXPARAMETEROPCODES'. Nesse opcode,
 o terceiro e oitavo bytes são substituídos pelo complemento de dois de -4v. O sexto byte é 0xCF quando o
 parâmetro é p1 (%edi) e 0xCE quando o parâmetro é p2 (%esi)
 
 Quando o parâmetro recebe, o opcode "geral" é definido por 'MULPARAMETERXVAROPCODES'. Nesse opcode,
 o terceiro byte é 0x7D quando o parâmetro é p1 (%edi), e 0x75 quando o parâmetro é p2 (%esi)
 
 Por fim, o último byte é o complemento de 2 de -4v, onde v é o índice da variável
 
 */

void mulVarParameter(int parameter, int var, int parameterReceives) {
    unsigned char *instructions;
    unsigned char parReceivesInstructions[4] = MUL_PARAMETER_X_VAR;
    unsigned char varReceivesInstructions[9] = MUL_VAR_X_PARAMETER;
    instructions = (parameterReceives) ? parReceivesInstructions : varReceivesInstructions;
    
    if (parameterReceives) {
        instructions[3] = (unsigned char)(-4 * var);
        instructions[2] = (parameter == 2) ? 0x75 : 0x7D;
    } else {
        instructions[2] = instructions[8] = (unsigned char)(-4 * var);
        instructions[5] = (parameter == 1) ? 0xCF : 0xCE;
    }
    appendInstructions(&currentInstruction, instructions, parameterReceives ? 4 : 9);
    
}

//MARK: Operações aritméticas parâmetro-parâmetro

void operateParameters(int lhs, int rhs, char operation) {
    switch (operation) {
        case '+':
            addParameters(lhs, rhs);
            break;
        case '-':
            subParameters(lhs, rhs);
            break;
        case '*':
            mulParameters(lhs, rhs);
    }
}

void addParameters (int lhs, int rhs) {
    unsigned char instructions[2] = ADD_PARAMETERS;
    if (lhs == 1) {
        instructions[1] = (rhs == 1) ? 0xFF : 0xF7;
    } else {
        instructions[1] = (rhs == 1) ? 0xFE : 0xF6;
    }
    appendInstructions(&currentInstruction, instructions, 2);
}

void subParameters(int lhs, int rhs) {
    unsigned char instructions[2] = SUB_PARAMETERS;
    if (lhs == 1) {
        instructions[1] = (rhs == 1) ? 0xFF : 0xF7;
    } else {
        instructions[1] = (rhs == 1) ? 0xFE : 0xF6;
    }
    appendInstructions(&currentInstruction, instructions, 2);
}

void mulParameters(int lhs, int rhs) {
    unsigned char instructions[3] = MUL_PARAMETERS;
    if (lhs == 1) {
        instructions[2] = (rhs == 1) ? 0xFF : 0xFE;
    } else {
        instructions[2] = (rhs == 1) ? 0xF7 : 0xF6;
    }
    appendInstructions(&currentInstruction, instructions, 3);
}

//MARK: Operações aritméticas parâmetro-constante

void operateConstParameter(int p, int c, char operation) {
    int i = 0;
    union InsideInt inside;
    unsigned char instructions[6] = ADD_CONST_PARAMETER;
    inside.i = c;
    switch (operation) {
        case '+':
            instructions[1] = (p == 1) ? 0xC7 : 0xC6;
            break;
        case '-':
            instructions[1] = (p == 1) ? 0xEF : 0xEE;
            break;
        case '*':
            instructions[0] = 0x69;
            instructions[1] = (p == 1) ? 0xFF :0xF6;
    }
    for(i = 0; i < 4; i++) {
        instructions[i + 2] = inside.c[i];
    }
    appendInstructions(&currentInstruction, instructions, 6);
}


//MARK: Operações Variável-Constante

void operateConstVar(int v, int c, char operation) {
    int i, beginningOffset;
    union InsideInt inside;
    unsigned char *instructions;
    unsigned char addSubOperations[7] = ADD_CONST_VAR;
    unsigned char mulOperations[12] = MUL_CONST_VAR;
    inside.i = c;
    
    if ( (operation == '+') || (operation == '-') ) {
        instructions = addSubOperations;
        instructions[1] = (operation == '+') ? 0x45 : 0x6D;
        instructions[2] = (unsigned char)(-4 * v);
        beginningOffset = 3;
    } else if (operation == '*') {
        instructions = mulOperations;
        instructions[11] = (unsigned char)(-4 * v);
        instructions[2] = (unsigned char)(-4 * v);
        beginningOffset = 5;
    }
    
    for (i = 0; i < 4; i++) {
        instructions[i + beginningOffset] = inside.c[i];
    }
    appendInstructions(&currentInstruction, instructions, (operation == '*') ? 12 : 7);
}

//MARK: Operações de Atribuição

void attribute(char varp, int idx0, char varpc, int idx1) {
    if (varp == 'v') {
        if (varpc == 'v') {
            attVarToVar(idx0, idx1);
        } else if (varpc == 'p') {
            attParamToVar(idx0, idx1);
        } else {
            attConstToVar(idx0, idx1);
        }
    } else {
        if (varpc == 'v') {
            attVarToParam(idx0, idx1);
        } else if (varpc == 'p') {
            attParamToParam(idx0, idx1);
        } else {
            attConstToParam(idx0, idx1);
        }
    }
}

void attVarToParam(int param, int var) {
    unsigned char instructions[3] = ATT_VAR_TO_PARAM;
    instructions[2] = (unsigned char)(-4 * var);
    instructions[1] = (param == 1) ? 0x7D : 0x75;
    appendInstructions(&currentInstruction, instructions, 3);
}

void attParamToParam(int lhs, int rhs) {
    unsigned char instructions[2] = ATT_PARAM_TO_PARAM;
    if (lhs == 1) {
        instructions[1] = (rhs == 1) ? 0xFF : 0xF7;
    } else {
        instructions[1] = (rhs == 1) ? 0xFE : 0xF6;
    }
    appendInstructions(&currentInstruction, instructions, 2);
}

void attConstToParam(int param, int c) {
    int i;
    union InsideInt inside;
    unsigned char instructions[5] = ATT_CONST_TO_PARAM;
    inside.i = c;
    instructions[0] = (param == 1) ? 0xBF : 0xBE;
    for (i = 0; i < 4; i++) {
        instructions[i + 1] = inside.c[i];
    }
    appendInstructions(&currentInstruction, instructions, 5);
}

void attVarToVar(int lhs, int rhs) {
    unsigned char instructions[6] = ATT_VAR_TO_VAR;
    instructions[2] = (unsigned char)(-4 * rhs);
    instructions[5] = (unsigned char)(-4 * lhs);
    appendInstructions(&currentInstruction, instructions, 6);
}

void attParamToVar(int var, int param) {
    unsigned char instructions[3] = ATT_PARAM_TO_VAR;
    instructions[1] = (param == 1) ? 0x7D : 0x75;
    instructions[2] = (unsigned char)(-4 * var);
    appendInstructions(&currentInstruction, instructions, 3);
}

void attConstToVar(int var, int c) {
    int i;
    union InsideInt inside;
    unsigned char instructions[7] = ATT_CONST_TO_VAR;
    inside.i = c;
    instructions[2] = (unsigned char)(-4 * var);
    for(i = 0; i < 7; i++) {
        instructions[3 + i] = inside.c[i];
    }
    appendInstructions(&currentInstruction, instructions, 7);
}

//MARK: Operacão de Retorno

void returnValue(int vpc, char type) {
    int instructionSize;
    unsigned char *instructions;
    unsigned char returnParameter[3] = RETURN_PARAMETER;
    unsigned char returnVar[4] = RETURN_VAR;
    unsigned char returnConst[6] = RETURN_CONSTANT;
    switch (type) {
            int i;
            union InsideInt inside;
        case 'v':
            instructions = returnVar;
            instructions[2] = (unsigned char)(-4 * vpc);
            instructionSize = 4;
            break;
        case 'p':
            instructions = returnParameter;
            instructions[1] = (vpc == 1) ? 0xF8 : 0xF0;
            instructionSize = 3;
            break;
        case '$':
            inside.i = vpc;
            instructions = returnConst;
            instructionSize = 6;
            for(i = 0; i < 4; i++) {
                instructions[i + 1] = inside.c[i];
            }
            break;
    }
    appendInstructions(&currentInstruction, instructions, instructionSize);
}

//MARK: Desvio Incondicional
void goOperation(int from, int to, int instructionPosition, struct goInstruction *goInstructions) {
    unsigned char instructions[5] = GO;
    struct goInstruction currGo;
    currGo.from = from; currGo.to = to;
    goInstructions[instructionPosition] = currGo;
    appendInstructions(&currentInstruction, instructions, 5);
}

void completeGoOperation(struct goInstruction instruction, int *lineOffset, unsigned char *codigo) {
    union InsideInt inside;
    int from = instruction.from;
    int to = instruction.to;
    unsigned char *instructionPointer = codigo + lineOffset[from-1] + 1;
    inside.i = lineOffset[to-1] - lineOffset[from-1] - 5;
    for(int i = 0; i < 4; i++) {
        instructionPointer[i] = inside.c[i];
    }
}

void completeGoOperations(struct goInstruction *allInstructions, int *lineOffset, int nInstructions, unsigned char *codigo) {
    int i;
    for(i = 0; i < nInstructions; i++) {
        completeGoOperation(allInstructions[i], lineOffset, codigo);
    }
}

//MARK: Desvio Condicional

void completeIfOperation(struct ifInstruction instruction, int *lineOffset, unsigned char* codigo) {
    union InsideInt inside1, inside2;
    int from = instruction.from;
    int n1 = instruction.n1;
    int n2 = instruction.n2;
    int isVar = instruction.isVar;
    unsigned char *n1Pointer = codigo + lineOffset[from-1] + 10 + isVar;
    unsigned char *n2Pointer = codigo + lineOffset[from-1] + 15 + isVar;
    inside1.i = lineOffset[n1-1] - lineOffset[from-1] - 14 - isVar;
    inside2.i = lineOffset[n2-1] - lineOffset[from-1] - 19 - isVar;
    for (int j = 0; j < 4; j++) {
        n1Pointer[j] = inside1.c[j];
        n2Pointer[j] = inside2.c[j];
    }
}

void completeIfOperations(struct ifInstruction *allInstructions, int *lineOffset, int nInstructions, unsigned char *codigo) {
    int i;
    for(i = 0; i < nInstructions; i++) {
        completeIfOperation(allInstructions[i], lineOffset, codigo);
    }
}

void ifOperationParameter(int idx0, int from, int n1, int n2, int instructionPosition, struct ifInstruction *ifInstructions){
    unsigned char instructions[19] = IF_PARAM;
    instructions[1] = (idx0 == 1) ? 0xFF : 0xFE;
    struct ifInstruction currIf;
    currIf.from = from;
    currIf.n1 = n1;
    currIf.n2 = n2;
    currIf.isVar = FALSE;
    ifInstructions[instructionPosition] = currIf;
    appendInstructions(&currentInstruction, instructions, 19);
}

void ifOperationVar(int idx0, int from, int n1, int n2, int instructionPosition, struct ifInstruction *ifInstructions) {
    unsigned char instructions[20] = IF_VAR;
    instructions[2] = (unsigned char)(-4 * idx0);
    struct ifInstruction currIf;
    currIf.from = from;
    currIf.n1 = n1;
    currIf.n2 = n2;
    currIf.isVar = TRUE;
    ifInstructions[instructionPosition] = currIf;
    appendInstructions(&currentInstruction, instructions, 20);
}

void ifOperation(char c, int idx0, int from, int n1, int n2, int instructionPosition, struct ifInstruction *ifInstructions) {
    if (c == 'p') {
        ifOperationParameter(idx0, from, n1, n2, instructionPosition, ifInstructions);
    } else {
        ifOperationVar(idx0, from, n1, n2, instructionPosition, ifInstructions);
    }
}

//MARK: Demais auxiliares

void appendInstructions(unsigned char **currInstruction, unsigned char newInstruction[], int nInstructions) {
    int i;
    for (i = 0; i < nInstructions; i++) {
        *(*currInstruction + i) = newInstruction[i];
    }
    *currInstruction += nInstructions;
}

void printInstruction(unsigned char first[], int number){
    printf("{");
    for (int i = 0; i < number; i++) {
        printf("%02X", first[i]);
        if (i < (number - 1)){printf(" ");}
    }
    printf("}\n");
}

void printGoInstructions(struct goInstruction* goInstructions, int nGoInstructions) {
    for(int i = 0; i < nGoInstructions; i++) {
        printf("Go Instruction %d\n", i + 1);
        printf("From %d To %d\n\n", goInstructions[i].from, goInstructions[i].to);
    }
}

int beginningOffset(int ofLine, int* array) {
    if (ofLine == 1) {
        return 0;
    } else {
        return array[0] + beginningOffset(ofLine - 1, array + 1);
    }
}

int endingOffset(int ofLine, int* array) {
    return beginningOffset(ofLine, array) + array[ofLine - 1];
}


