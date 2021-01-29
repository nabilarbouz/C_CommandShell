#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// maximum number of registers
#define RMAX 4

// maximum length of a string
#define SMAX 80

// maximum number of memory locations
#define MMAX 16

// registers
int r[RMAX];

// memory
int m[MMAX];

// loop variable
int i;

// cmd input string
char cmd_input[SMAX];

// cmd portion of input (1st word)
char cmd[SMAX];

// argument portion of input (2nd word)
char arg[SMAX];
int iarg;                     // interger version of argument

// number words found in input
int wordsFound;

// display mode
char display_mode[4] = "DEC"; // same as {'D','E','C','\0'}

bool done = false;

bool runMode = false;

FILE* fp = NULL;              // file pointer for basic input/output

FILE* runfp = NULL;

int temp;                     // temporary integer

void printRegister(int reg) {
    // prints value of r[reg] in correct display mode
    printf("[%s] ", display_mode);

    if (strcmp(display_mode, "DEC") == 0) {
        printf("r[%d]: ", reg);
        printf("%d\n", r[reg]);
    } else if (strcmp(display_mode, "HEX") == 0) {
        printf("r[0x%x]: ", reg);
        printf("0x%x\n", r[reg]);
    } else {
        printf("INVALID MODE\n");
    }
};

void printRegisterToFile(int reg, FILE* dest) {
    // prints value of r[reg] in decimal mode

    fprintf(dest, "%d\n", r[reg]);
    return;
};

void printMemory(int mem) {
    // prints value of m[mem] in correct display mode
    printf("[%s] ", display_mode);

    if (strcmp(display_mode, "DEC") == 0) {
        printf("m[%d]: ", mem);
        printf("%d\n", m[mem]);
    } else if (strcmp(display_mode, "HEX") == 0) {
        printf("m[0x%x]: ", mem);
        printf("0x%x\n", m[mem]);
    } else {
        printf("INVALID MODE\n");
    }
};

void printMemoryToFile(int mem, FILE* dest) {
    // prints value of m[mem] in decimal mode

    fprintf(dest, "%d\n", m[mem]);
    return;
};


int readIntFromString(char* source){
    // reads value of integer from source string in correct display mode
    int intsFound;
    int result;

    if (strcmp(display_mode, "DEC") == 0) {
        intsFound = sscanf(source, "%d", &result);
        if (intsFound != 1) {
            printf("INVALID NUMBER - used 0 instead\n");
            result = 0;
        }
    } else if (strcmp(display_mode, "HEX") == 0) {
        intsFound = sscanf(source, "%x", &result);
        if (intsFound != 1) {
            printf("INVALID NUMBER - used 0x0 instead\n");
            result = 0;
        }
    } else {
        printf("INVALID MODE - used 0 instead\n");
        result = 0;
    }

    return(result);
}

int readIntFromFile(FILE* dest) {
    // read next int from file dest in decimal mode
    int result;

    fscanf(dest, "%d", &result);
    return(result);
};


void stringUpper(char string[]){
    // upper case the argument string
    int i = 0;

    while ((string[i] != '\0') && (i < SMAX)) {
        if (string[i] >= 'a' && string[i] <= 'z') {
            string[i] = string[i] - 32;
        }
        i++;
    }
}


void clearAllRegisters(){
    int i;
    for (i=0; i<RMAX; i++) {
        r[i] = 0;
    }
}

void clearAllMemory(){
    int i;
    for (i=0; i<MMAX; i++) {
        m[i] = 0;
    }
}


// ============= MAIN PROGRAM ==============

int main () {
    // initialize registers to 0
    clearAllRegisters();

    //initialize memories to 0
    clearAllMemory();

    printf("\nWelcome to the command shell...now with memory!\n");

    while (!done) {
        // GET INPUT
        // show current status and value of r[0]

        printRegister(0);

        //prompt for input and read input string
        if(runMode == false) {
            printf("> ");
            fgets(cmd_input, SMAX, stdin);
            wordsFound = sscanf(cmd_input, "%s %s %*s", cmd, arg);
        } else {
            // this implementation will ignore any command that is on the same line as an EOF flag
            if(fgets(cmd_input, SMAX, runfp) && !feof(runfp)) {
                wordsFound = sscanf(cmd_input, "%s %s %*s", cmd, arg);
                printf("RUN> %s %s\n", cmd, arg);
            } else {
                fclose(runfp);
                printf("RUN> EOF\n");
                printf("====================\n");
                printf("RUN - end reading of file\n");
                printf("====================\n");
                wordsFound = 0;
                runMode = false;
            }
        }
        //break the input string into cmd arg and junk


        // uppercase cmd to standard format
        stringUpper(cmd);

        // handle error inputs
        if (wordsFound <= 0) {
            // no command found, substitute NOP
            strcpy(cmd, "NOP");
        } else if (wordsFound <= 1) {
            // no argument found, substute empty string
            strcpy(arg, "");
        }

        // PROCESS INPUT
        if ((strcmp(cmd, "EXIT") == 0) || (strcmp(cmd, "QUIT") == 0)) {
            // exit from shell
            printf("EXIT encountered.\n");
            done = true;

        } else if (strcmp(cmd, "NOP") == 0) {
            // do nothing
            printf("NOP\n");

        } else if (strcmp(cmd, "LOADI") == 0) {
            // load (immediate) arg into r[0]
            // convert arg to integer - make sure it is valid

            iarg = readIntFromString(arg);

            // push the other registers up
            for (i=RMAX-1; i>0; i--) {
                r[i] = r[i-1];
            }

            // load arg into r[0]
            r[0] = iarg;

        } else if (strcmp(cmd, "LOADM") == 0) {
            // load (memory) m[arg] into r[0]
            // convert arg to integer - make sure it is valid

            iarg = readIntFromString(arg);

            if ((0 <= iarg) && (iarg < MMAX)) {
                // push the other registers up
                for (i=RMAX-1; i>0; i--) {
                    r[i] = r[i-1];
                }

                // load m[arg] into r[0]
                r[0] = m[iarg];

            } else {
                printf("Error: invalid memory location\n");
            }

        } else if (strcmp(cmd, "STORM") == 0) {
            // store r[0] into memory m[arg]
            // convert arg to integer - make sure it is valid

            iarg = readIntFromString(arg);

            if ((0 <= iarg) && (iarg < MMAX)) {

                // store r[0] into m[arg]
                m[iarg] = r[0];

                // scroll the other registers down
                for (i=0; i<=RMAX-2; i++){
                    r[i] = r[i+1];
                }

                r[RMAX-1] = 0;

            } else {
                printf("Error: invalid memory location\n");
            }

        } else if (strcmp(cmd, "PRTRS") == 0) {
            // print out the value of all the registers

            printf("--------------------\n");

            for (i=RMAX-1; i>=0; i--) {
                printRegister(i);
            }

            printf("--------------------\n");

        } else if (strcmp(cmd, "PRTMS") == 0) {
            // print out the value of all the memories

            printf("--------------------\n");

            for (i=0; i<MMAX; i++) {
                printMemory(i);
            }

            printf("--------------------\n");

        } else if (strcmp(cmd, "ECHO") == 0) {
            // echo arg
            printf("%s\n", arg);

        } else if (strcmp(cmd, "MODE") == 0) {
            // update display mode
            // standardize argument
            stringUpper(arg);

            if (strcmp(arg, "DEC") == 0) {
                strcpy(display_mode, "DEC");
            } else if (strcmp(arg, "HEX") == 0) {
                strcpy(display_mode, "HEX");
            } else {
                printf("INVALID DISPLAY MODE: %s\n", arg);
            }

        } else if (strcmp(cmd, "FSTOR") == 0) {
            // store register/memory to file
            // file name in second argument - arg

            fp = fopen(arg, "w");

            if (fp != NULL) {
                for (i=0; i<RMAX; i++) {
                    printRegisterToFile(i, fp);
                }

                for (i=0; i<MMAX; i++) {
                    printMemoryToFile(i, fp);
                }
                fclose(fp);
                printf("Data successfully stored to file: %s\n",arg);

            } else {
                printf("INVALID FILE: %s\n", arg);
            }

        } else if (strcmp(cmd, "FLOAD") == 0) {
            // load register/memory from file
            // file name in second argument - arg

            fp = fopen(arg, "r");

            if (fp != NULL) {
                for (i=0; i<RMAX; i++) {
                    temp = readIntFromFile(fp); // read next integer from file
                    if (feof(fp) == 0) {
                        r[i] = temp;  // assign only if not eof
                    } else {
                        printf("End of File encountered at register %d\n",i);
                    }
                }

                for (i=0; i<MMAX; i++) {
                    temp = readIntFromFile(fp); // read next integer from file
                    if (feof(fp) == 0) {
                        m[i] = temp;  // assign only if not eof
                    } else {
                        printf("End of File encountered at memory %d\n", i);
                    }
                }
                fclose(fp);
                printf("Data successfully read from file: %s\n",arg);

            } else {
                printf("INVALID FILE: %s\n", arg);
            }

        } else if (strcmp(cmd, "RUN") == 0) {
            runfp = fopen(arg, "r");
            if(runfp != NULL) {
                runMode = true;
                printf("=====================\n");
                printf("RUN - starting to read from file: %s\n", arg);
                printf("=====================\n");
            } else {
                printf("INVALID FILE: %s\n", arg);
            }
        } else if (strcmp(cmd, "CLRRS") == 0) {
            // clear all registers
            clearAllRegisters();

        } else if (strcmp(cmd, "CLRMS") == 0) {
            // clear all memories
            clearAllMemory();

        } else if (strcmp(cmd, "ADD") == 0) {
            // adds r[0]+r[1], puts answer into r[0]
            // moves 0->r[3]
            // moves r[3]->r[2]
            // moves r[2]->r[1]

            r[0] = r[0] + r[1];

            for (i=1; i<=RMAX-2; i++){
                r[i] = r[i+1];
            }

            r[RMAX-1] = 0;

        } else if (strcmp(cmd, "SUB") == 0) {
            // subtracts r[1]-r[0], puts answer into r[0]
            // moves 0->r[3]
            // moves r[3]->r[2]
            // moves r[2]->r[1]

            r[0] = r[1] - r[0];

            for (i=1; i<=RMAX-2; i++){
                r[i] = r[i+1];
            }

            r[RMAX-1] = 0;

        } else if (strcmp(cmd, "MUL") == 0) {
            // multiplies r[1]*r[0], puts answer into r[0]
            // moves 0->r[3]
            // moves r[3]->r[2]
            // moves r[2]->r[1]

            r[0] = r[1] * r[0];

            for (i=1; i<=RMAX-2; i++){
                r[i] = r[i+1];
            }

            r[RMAX-1] = 0;

        } else if (strcmp(cmd, "DIV") == 0) {
            // integer divides  r[1]/r[0], puts answer into r[0]
            // moves 0->r[3]
            // moves r[3]->r[2]
            // moves r[2]->r[1]

            if (r[0] != 0) {
                r[0] = r[1] / r[0];

                for (i=1; i<=RMAX-2; i++){
                    r[i] = r[i+1];
                }

                r[RMAX-1] = 0;
            } else {
                printf("Error: do not divide by 0\n");
            }

        } else if (strcmp(cmd, "REM") == 0) {
            // integer remiander r[1]%r[0], puts answer into r[0]
            // moves 0->r[3]
            // moves r[3]->r[2]
            // moves r[2]->r[1]

            if (r[0] != 0) {
                r[0] = r[1] % r[0];

                for (i=1; i<=RMAX-2; i++){
                    r[i] = r[i+1];
                }

                r[RMAX-1] = 0;
            } else {
                printf("Error: do not divide by 0\n");
            }

        } else if (strcmp(cmd, "INC") == 0) {
            // increment r[0]
            r[0]++;

        } else if (strcmp(cmd, "DEC") == 0) {
            // decrement r[0]
            r[0]--;

        } else if (strcmp(cmd, "NEG") == 0) {
            // negate r[0]
            r[0] = -1*r[0];

        } else if (strcmp(cmd, "HELP") == 0) {
            // list all commands
            printf("Commands:\n");
            printf("EXIT (or QUIT): terminate program\n");
            printf("NOP: no effect\n");
            printf("ECHO arg: print content of first word of arg\n");
            printf("MODE arg: change input/output display of numbers - either DEC or HEX\n");
            printf("HELP: list valid commands\n");
            printf("\n");
            printf("LOADI arg: load integer 'arg' into register 0, scroll registers up\n");
            printf("LOADM arg: load contents of memory location m[arg] into register 0, scroll registers up\n");
            printf("STORM arg: move contents of register 0 into memory location m[arg], scroll registers down\n");
            printf("CLRRS: clear all registers\n");
            printf("CLRMS: clear all memories\n");
            printf("\n");
            printf("ADD: add contents of register r0 to r1 store into r0, scroll registers down\n");
            printf("SUB: subtract contents of register r0 from r1 store into r0, scroll registers down\n");
            printf("MUL: multiply contents of register r0 by r1 store into r0, scroll registers down\n");
            printf("DIV: integer divide contents of register r1 by r0 store into r0, scroll registers down\n");
            printf("REM: integer remainder of register r1 dividd by r0 store into r0, scroll registers down\n");
            printf("INC: increment r0\n");
            printf("DEC: decrement r0\n");
            printf("NEG: multiply r0 by -1\n");
            printf("\n");
            printf("PRTRS: print content of all registers\n");
            printf("PRTMS: print content of all memories\n");
            printf("FSTOR arg: store all registers and memory to file 'arg'\n");
            printf("FLOAD arg: load all registers and memory from file 'arg'\n");
            printf("RUN arg: run a program using the shell's commands taken from file 'arg'\n");


        } else {
            // Error on Input
            printf(">> Unknown Command |%s|%s|\n", cmd, arg);
        }

    }

    printf("\nEnd Program.\n\n");
    return(0);

}