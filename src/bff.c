/* BRAINFUCK INTERPRETER
 * Lanciato da linea di comando:
 * ./brainfuck nomefile.bf <debug {0|1}>
*/
#include <stdio.h>
#include <stdlib.h>
#include "simple-stack.h"

#define RAMSIZE 30000
#define NEWLINE 0

void parse(FILE * fileSrc);
void exec(int debug, int newline);

/* jmp -> Jump Table.
 * Aka: una hash table; laddove, in src[idx], c'è una parentesi quadra,
 * jmp[idx] conterrà l'indirizzo (o meglio, lo spiazzamento da &src) a cui
 * saltare. */
int * jmp;

/* src -> source code.
 * In fase di lettura del file sorgente, tutti i caratteri non necessari
 * non verranno inseriti in src. */
unsigned char * src;

/* ram -> memoria centrale del programma. */
unsigned char ram[RAMSIZE] = {0};

enum instructions {
     INC_DP = '>',
     DEC_DP = '<',
     INC_BY = '+',
     DEC_BY = '-',
     OUTPUT = '.',
     INPUT  = ',',
     BEQZ   = '[',
     BNEZ   = ']'
};
    
int main(int argv, char** argc) {
    if (argv == 1 || argv > 3) {
        printf("brainfuck - a brainfuck interpreter\n\n");
        printf("Syntax:\n");
        printf("\tbrainfuck [filename] <debug>\n");
        printf("\n");
        printf("Usage:\n");
        printf("\twithout flags, brainfuck executes the .bf file as it is.\n");
        printf("\t<debug> can be set either to 0 or 1; the latter enables it.\n");
        printf("\tbrainfuck with no arguments prints this text.\n");
        exit(-1);
    }

    int fileIndex = (argv == 3) ? 2 : 1; 
    int debugMode = (argv == 3) ? *argc[1] - 48 : 0;
    
    FILE * fileSrc = fopen(argc[fileIndex], "rb");
    if (fileSrc == NULL) {
        printf("File not found. Exiting...");
        exit(-1); 
    }
    
    parse(fileSrc);
    
    exec(debugMode, NEWLINE);
}

void parse(FILE * fileSrc) {
    // measure file size
    fseek(fileSrc, 0, SEEK_END);
    int size = ftell(fileSrc);    
    rewind(fileSrc);

    // allocate space for source code and memory
    src = calloc(size, sizeof(unsigned char));
    jmp = calloc(size, sizeof(int));
    Stack stack = stackInit();

    // parse
    int  i = 0;
    char c;
    while ((c = fgetc(fileSrc)) != EOF) {
        if (c == '>' || c == '<' || c == '.' || c == ',' ||
            c == '-' || c == '+' || c == '[' || c == ']') {

            /* Copy char in src */
            src[i++] = c;

            /* Populate jmp table */
            if (c == '[')
                stackPush(stack, i-1);
            if (c == ']') {
                int idx1 = stackPop(stack); // also checks if len(stack) < 0 
                int idx2 = i-1;
                jmp[idx1] = idx2;
                jmp[idx2] = idx1;
            }
            
        }
    }
    src[i] = 0;

    if (stackCount(stack) != 0) {
        printf("Unmatched bracket. Exiting...");
        exit(-1);
    }

    free(stack);
    src = realloc(src, i);
    jmp = realloc(jmp, i * sizeof(int));
    fclose(fileSrc);
}

void exec(int debug, int newline) {
    unsigned char* dp = ram;
    unsigned char* ip = src;
    
    while(*ip != 0) {
        
        if (dp - ram >= RAMSIZE) {
            printf("Out of bound error\n");
            exit(-1);
        }
        
        if (debug) 
            printf("%c\t%d\t%d\t%d", *ip, ip-src, dp-ram, *dp);
        
        switch(*ip) {
            case INC_DP:
                dp++;
                break;
            case DEC_DP:
                dp--;
                break;
            case INC_BY:
                *dp = 1 + *dp;
                break;
            case DEC_BY:
                *dp = -1 + *dp;
                break;
            case OUTPUT:
                if (debug) printf("\t");
                printf("%c", *dp);
                if (newline) printf("\n");
                break;
            case INPUT:
                scanf(" %c", dp);
                break;
            case BEQZ:
                if (*dp == 0)
                    ip = src + jmp[ip - src];
                break;
            case BNEZ:
                if (*dp != 0)
                    ip = src + jmp[ip - src];
                break;
        }

        if (debug) printf("\n");
        ip++;
    }
}
