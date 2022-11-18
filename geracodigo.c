#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define ADD_VARIABLES_OPCODES {0x8B, 0x4d, 0x00, 0x01, 0x4d, 0x00} //checked
#define SUB_VARIABLES_OPCODES {0x8b, 0x4d, 0x00, 0x29, 0x4d, 0x00} //checked
#define MUL_VARIABLES_OPCODES {0x8b, 0x4d, 0x00, 0x8b, 0x55, 0x00, 0x0f, 0xaf, 0xd1, 0x89, 0x55, 0x00} //checked

#define ADD_VAR_PARAMETER_OPCODES {0x01, 0x75, 0x00} //checked
#define SUB_VAR_PARAMETER_OPCODES {0x29, 0x75, 0x00} //checked
#define MUL_VAR_X_PARAMETER_OPCODES {0x8b, 0x4d, 0x00, 0x0f, 0xaf, 0xcf, 0x89, 0x4d, 0x00} //var receives
#define MUL_PARAMETER_X_VAR_OPCODES {0x0f, 0xaf, 0x7d, 0x00} //parameter receives
 
void addVars(int lhs, int rhs);
void subVars(int lhs, int rhs);
void mulVars(int lhs, int rhs);

void addVarParameter(int parameter, int var, int parameterReceives);
void subVarParameter(int parameter, int var, int parameterReceives);
void mulVarParameter(int parameter, int var, int parameterReceives);

void addParameters(int lhs, int rhs);
void subParameters(int lhs, int rhs);
void mulParameters(int lhs, int rhs);

void printInstruction(unsigned char first[], int number);

//MARK: operações aritméticas variável-variável

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
    unsigned char instruction[6] = ADD_VARIABLES_OPCODES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instruction[5] = receiver;
    instruction [2] = added;
    printInstruction(instruction, 6);
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
    unsigned char instruction[6] = SUB_VARIABLES_OPCODES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instruction[5] = receiver;
    instruction [2] = added;
    printInstruction(instruction, 6);
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
    unsigned char instruction[12] = MUL_VARIABLES_OPCODES;
    receiver = (char)lhs;
    added = (char)rhs;
    receiver *= -4;
    added *= -4;
    instruction[11] = receiver;
    instruction[5] = receiver;
    instruction[2] = added;
    printInstruction(instruction, 12);
}

//MARK: operações aritméticas parâmetro-variável

/**
 Essa função gera o opcode correspondente a uma operação de soma entre uma variável local e um parâmetro
 
 Foi possível observar que, se a variável local recebe o resultado da operação, o valor do primeiro byte da instrução é 0x01, e caso contrário, 0x03
 
 Também soi possível observar que, representando o parâmetro 1 com %edi e o parâmetro 2 com %esi, quando o p1 está envolvido, o segundo byte da instrução é 0x7d,
 mas quando o p2 está envolvido, o segundo byte é 0x75
 
 Por fim, observou-se que o último byte da instrução é igual ao valor em complemento de 2 de -4v, onde v é o índice da variável local envolvida
 */

void addVarParameter(int parameter, int var, int parameterReceives) {
    unsigned char instruction[3] = ADD_VAR_PARAMETER_OPCODES;
    
    //definindo o primeiro byte da instrução
    if (parameterReceives) {
        instruction[0] = 0x03;
    } else {
        instruction[0] = 0x01;
    }
    
    //definindo o segundo byte da instrução
    if (parameter == 1) {
        instruction[1] = 0x7D;
    } else if (parameter == 2) {
        instruction[2] = 0x75;
    }
    
    //definindo o terceiro byte da instrução
    instruction[2] = (unsigned char)(-4 * var);
    printInstruction(instruction, 3);
}

/**
 Essa função gera o opcode correspodente a uma operação de subtração entre uma variável e um parâmetro
 
 Nós observamos que, quando a variável está recebendo, o primeiro byte da instrução é 0x29. Por outro lado, quando
 o parâmetro está recebendo, o primeiro byte da instrução é 0x2b
 
 Quando o parâmetro é o p1 (representado por %edi), o segundo byte da instrução é 0x7d. Quando o parâmetro é p2 (%esi), o segundo byte é 0x75
 
 Por fim, o último byte é o complemento de dois de -4v, onde v é o índice da variável
 */

void subVarParameter(int parameter, int var, int parameterReceives) {
    unsigned char instruction[3] = SUB_VAR_PARAMETER_OPCODES;
    
    //definindo o primeiro byte da instrução
    if (parameterReceives) {
        instruction[0] = 0x2B;
    } else {
        instruction[0] = 0x29;
    }
    
    //definindo o segundo byte da instrução
    if (parameter == 1) {
        instruction[1] = 0x7D;
    } else {
        instruction[1] = 0x75;
    }
    
    //definindo o último byte da instrução
    instruction[2] = (unsigned char)(-4 * var);
    printInstruction(instruction, 3);
    
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
    unsigned char parReceivesInstructions[4] = MUL_PARAMETER_X_VAR_OPCODES;
    unsigned char varReceivesInstructions[9] = MUL_VAR_X_PARAMETER_OPCODES;
    instructions = (parameterReceives) ? parReceivesInstructions : varReceivesInstructions;
    
    if (parameterReceives) {
        instructions[3] = (unsigned char)(-4 * var);
        instructions[2] = (parameter == 2) ? 0x75 : 0x7D;
    } else {
        instructions[2] = instructions[7] = (unsigned char)(-4 * var);
        instructions[5] = (parameter == 1) ? 0xCF : 0xCE;
    }
    printInstruction(instructions, parameterReceives ? 4 : 9);
    
}

//MARK: Operações aritméticas parâmetro-parâmetro

void addParameters (int lhs, int rhs) {
    unsigned char instructions[2];
    if (lhs == 1) {
        instructions = {0x01, 0xF7};
    } else {
        instructions = {0x01, 0xFE};
    }
}

void subParameters(int lhs, int rhs) {
    unsigned char instructions[2];
    if (lhs == 1) {
        instructions = {0x29, 0xF7};
    } else {
        instructions = {0x29, 0xFE};
    }
}

void mulParameters(int lhs, int rhs) {
    unsigned char instructions[2];
    if (lhs == 1) {
        instructions = {0x0F, 0xAF, 0xFE};
    } else {
        instructions = {0x0F, 0xAF, 0xF7};
    }
}

//MARK: Demais auxiliares

void printInstruction(unsigned char first[], int number) {
    printf("{");
    for (int i = 0; i < number; i++) {
        printf("%02X", first[i]);
        if (i < (number - 1)){printf(", ");}
    }
    printf("}\n");
}
