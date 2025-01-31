
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define START_ADDRESS 0x00400000
// structs for labels, R, I, and J types
typedef struct {
    char label[20];
    int address;
} Label;

typedef struct {
    char func[10];
    int rs;
    int rt;
    int rd;
    int shamt;
} RType;

typedef struct {
    char opcode[10];
    int rs;
    int rt;
    int immediate;
} IType;

typedef struct {
    char opcode[10];
    int address;
} JType;

Label labels[100];
int labelCount = 0;
int address = START_ADDRESS;

void addLabel(char *label, int address) {
    // Copy the label and address to the labels array
    strcpy(labels[labelCount].label, label);
    labels[labelCount].address = address;
    labelCount++;
}

int getLabelAddress(char *label) {
    for (int i = 0; i < labelCount; i++) {
        // If the label is found, return the address
        if (strcmp(labels[i].label, label) == 0) {
            return labels[i].address;
        }
    }
    return -1;
}

char *dec2binNBits(int n, int nbit) {
    // Allocate memory 
    char *bin = (char *)malloc(nbit + 1); 
    bin[nbit] = '\0';
 
    unsigned int num;
    if (n < 0) {
        num = (1U << nbit) + n; //2^nbit + n(2's complement)
    } else {
        num = n;
    }
    // Initialize all bits to '0' for 
    for (int i = 0; i < nbit; i++) {
        bin[i] = '0';
    }
    // Convert the number to binary
    int i = nbit - 1;
    while (num > 0 && i >= 0) {
        bin[i] = '0' + (num % 2);// dividing constantly to 2 and getting remainder until reaching 0 
        num /= 2;
        i--;
    }
    return bin;
}
char *opcode2bin(char *opcode) {
    if (strcmp(opcode, "ADDI") == 0) return "001000";
    if (strcmp(opcode, "ANDI") == 0) return "001100";
    if (strcmp(opcode, "BEQ") == 0) return "000100";
    if (strcmp(opcode, "BNE") == 0) return "000101";
    if (strcmp(opcode, "J") == 0) return "000010";
    
}

char *func2bin(char *func) {
    if (strcmp(func, "ADD") == 0) return "100000";
    if (strcmp(func, "SUB") == 0) return "100010";
    if (strcmp(func, "AND") == 0) return "100100";
    if (strcmp(func, "OR") == 0) return "100101";
    if (strcmp(func, "SLL") == 0) return "000000";
    }

// Write R-Type instruction
void writeRType(FILE *output, RType r) {
    fprintf(output, "0x%08X %s %s %s %s %s %s\n", address, dec2binNBits(0, 6), dec2binNBits(r.rs, 5), dec2binNBits(r.rt, 5), dec2binNBits(r.rd, 5), dec2binNBits(r.shamt, 5), func2bin(r.func));
    address += 4;
}
// Write I-Type instruction
void writeIType(FILE *output, IType i) {
    fprintf(output, "0x%08X %s %s %s %s\n", address, opcode2bin(i.opcode), dec2binNBits(i.rs, 5), dec2binNBits(i.rt, 5), dec2binNBits(i.immediate, 16));
    address += 4;
}
// Write J-Type instruction
void writeJType(FILE *output, JType j) {
    int address_offset = j.address / 4;  // Divide by 4 for instruction address
    fprintf(output, "0x%08X %s %s\n", address, opcode2bin(j.opcode), dec2binNBits(address_offset, 26));
    address += 4;
}

int main(void) {
    char instruction[80];

    char file[100];
    char originalFile[100];

    printf("Enter the input file name: ");
    scanf("%s", file);
    strcpy(originalFile, file); // Store the original file name to use later for input

    // Split the filename parts before and after the dot
    char name[100] = {0}; // Base 
    char *token = strtok(file, "."); 
    if (token) {
        strcpy(name, token); // Copy the base name
        token = strtok(NULL, ".");
    }
    char outputName[100];// output filename
    if (token) {
        //create the output file name
        snprintf(outputName, sizeof(outputName), "%s.obj", name);
    } 

    FILE *input = fopen(originalFile, "r");
    FILE *output = fopen(outputName, "w");

    if (input == NULL || output == NULL) {
        printf("Cannot open file\n");
    }

    // First pass to collect labels
    while (fgets(instruction, 80, input)) {
        char opcode[10];
        sscanf(instruction, "%s", opcode);

        if (strchr(opcode, ':')) {//if there is a colon in the opcode, it is a label
            opcode[strlen(opcode) - 1] = '\0';  //store each label without the colon
            addLabel(opcode, address);
        } else {
            address += 4;
        }
    }

    // Close and reopen the file
    fclose(input);
    input = fopen(originalFile, "r");
    address = START_ADDRESS;// Reset the address for the second pass

    // Second pass to convert instructions
    while (fgets(instruction, 80, input)) { // Read each line of the file
        char opcode[10];
        sscanf(instruction, "%s", opcode);

        // Check the opcodes and convert the instruction to binary
        // R-Type instructions ADD, SUB, AND, OR, SLL 
        if (strcmp(opcode, "ADD") == 0 || strcmp(opcode, "SUB") == 0 || strcmp(opcode, "AND") == 0 || strcmp(opcode, "OR") == 0) {
            RType r;
            strcpy(r.func, opcode);
            sscanf(instruction, "%*s $%d, $%d, $%d", &r.rd, &r.rs, &r.rt);
            r.shamt = 0;
            writeRType(output, r);
            }
            //R-Type instruction SLL
            else if (strcmp(opcode, "SLL") == 0) {
            RType r;
            strcpy(r.func, opcode);
            sscanf(instruction, "%*s $%d, $%d, %d", &r.rd, &r.rt, &r.shamt);
            r.rs = 0;
            writeRType(output, r);
            } 
            // I-Type instructions ADDI, ANDII
            else if (strcmp(opcode, "ADDI") == 0 || strcmp(opcode, "ANDI") == 0) {
            IType i;
            strcpy(i.opcode, opcode);
            sscanf(instruction, "%*s $%d, $%d, %d", &i.rt, &i.rs, &i.immediate);
            writeIType(output, i);
             } 
             // I-Type instructions BEQ, BNE
            else if (strcmp(opcode, "BEQ") == 0 || strcmp(opcode, "BNE") == 0) {
            IType i;
            strcpy(i.opcode, opcode);
            char label[20];
            sscanf(instruction, "%*s $%d, $%d, %s", &i.rs, &i.rt, label);
            int targetAddress = getLabelAddress(label);
            i.immediate = (targetAddress - (address + 4)) / 4;// Calculate the offset
            writeIType(output, i);
             }
             // J-Type instruction J
            else if (strcmp(opcode, "J") == 0) {
            JType j;
            strcpy(j.opcode, opcode);
            char label[20];
            sscanf(instruction, "%*s %s", label);
            j.address = getLabelAddress(label);
            writeJType(output, j);
           } 
            else if (strchr(opcode, ':')) {

            continue;
        }
    }

    fclose(input);
    fclose(output);
    return 0;
    
}