#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "data.h"


/* ************************************************************************************ */
/* Declarations                                                                         */
/* ************************************************************************************ */

/**
* Recommended design includes the following functions implemented.
* However, you are free to change them as you wish inside the vm.c file.
* */
void initVM(VirtualMachine*);

int readInstructions(FILE*, Instruction*);

void dumpInstructions(FILE*, Instruction*, int numOfIns);

int getBasePointer(int *stack, int currentBP, int L);

void dumpStack(FILE*, int* stack, int sp, int bp);

int executeInstruction(VirtualMachine* vm, Instruction ins, FILE* vmIn, FILE* vmOut);

/* ************************************************************************************ */
/* Global Data and misc structs & enums                                                 */
/* ************************************************************************************ */

/**
* allows conversion from opcode to opcode string
* */
const char *opcodes[] =
{
	"illegal", // opcode 0 is illegal
	"lit", "rtn", "lod", "sto", "cal", // 1, 2, 3 ..
	"inc", "jmp", "jpc", "sio", "sio",
	"sio", "neg", "add", "sub", "mul",
	"div", "odd", "mod", "eql", "neq",
	"lss", "leq", "gtr", "geq"
};

enum { CONT, HALT };

/* ************************************************************************************ */
/* Definitions                                                                          */
/* ************************************************************************************ */

/**
* Initialize Virtual Machine
* */
void initVM(VirtualMachine* vm)
{
	if (vm)
	{
		// TODO
		vm->BP = 1;
		vm->SP = 0;
		vm->PC = 0;
		vm->IR = 0;

		/**
		* register file
		* */
		for (int i = 0; i < REGISTER_FILE_REG_COUNT; i++)
			vm->RF[i] = 0;

		/**
		* stack
		* */
		memset(vm->stack,0,MAX_STACK_HEIGHT*sizeof(int));
	}
}

/**
* Fill the (ins)tructions array by reading instructions from (in)put file
* Return the number of instructions read
* */
int readInstructions(FILE* in, Instruction* ins)
{
	// Instruction index
	int i = 0;

	while (fscanf(in, "%d %d %d %d", &ins[i].op, &ins[i].r, &ins[i].l, &ins[i].m) != EOF)
	{
		i++;
	}

	// Return the number of instructions read
	return i;
}

/**
* Dump instructions to the output file
* */
void dumpInstructions(FILE* out, Instruction* ins, int numOfIns)
{
	// Header
	fprintf(out,
		"***Code Memory***\n%3s %3s %3s %3s %3s \n",
		"#", "OP", "R", "L", "M"
	);

	// Instructions
	int i;
	for (i = 0; i < numOfIns; i++)
	{
		fprintf(
			out,
			"%3d %3s %3d %3d %3d \n", // formatting
			i, opcodes[ins[i].op], ins[i].r, ins[i].l, ins[i].m
		);
	}
}

/**
* Returns the base pointer for the lexiographic level L
* */
int getBasePointer(int *stack, int currentBP, int L)
{
	int B1;
	B1 = currentBP;
	while (L > 0) {
		B1 = stack[B1 + 1];
		L--;
	}
	return B1;
}

// Function that dumps the whole stack into output file
// Do not forget to use '|' character between stack frames
void dumpStack(FILE* out, int* stack, int sp, int bp)
{
	if (bp == 0)
		return;

	// bottom-most level, where a single zero value lies
	if (bp == 1)
	{
		fprintf(out, "%3d ", 0);
	}

	// former levels - if exists
	if (bp != 1)
	{
		dumpStack(out, stack, bp - 1, stack[bp + 2]);
	}

	// top level: current activation record
	if (bp <= sp)
	{
		// indicate a new activation record
		fprintf(out, "| ");

		// print the activation record
		int i;
		for (i = bp; i <= sp; i++)
		{
			fprintf(out, "%3d ", stack[i]);
		}
	}
}

/**
* Executes the (ins)truction on the (v)irtual (m)achine.
* This changes the state of the virtual machine.
* Returns HALT if the executed instruction was meant to halt the VM.
* .. Otherwise, returns CONT
* */
int executeInstruction(VirtualMachine* vm, Instruction ins, FILE* vmIn, FILE* vmOut)
{
	switch (ins.op)
	{
		// TODO
	case 1: //"lit"
		vm->RF[ins.r] = ins.m;
		break;
	case 2: //"rtn"
		vm->SP = vm->BP - 1;
		vm->BP = vm->stack[vm->SP + 3];
		vm->PC = vm->stack[vm->SP + 4];
		break;
	case 3: //"lod"
		vm->RF[ins.r] = vm->stack[getBasePointer(vm->stack, vm->BP, ins.l) + ins.m];
		break;
	case 4: //"sto"
		vm->stack[getBasePointer(vm->stack, vm->BP, ins.l) + ins.m] = vm->RF[ins.r];
		break;
	case 5: //"cal"
		vm->stack[vm->SP + 1] = 0;
		vm->stack[vm->SP + 2] = getBasePointer(vm->stack, vm->BP, ins.l);
		vm->stack[vm->SP + 3] = vm->BP;
		vm->stack[vm->SP + 4] = vm->PC;
		vm->BP = vm->SP + 1;
		vm->PC = ins.m;
		break;
	case 6: //"inc"
		vm->SP = vm->SP + ins.m;
		break;
	case 7: //"jmp"
		vm->PC = ins.m;
		break;
	case 8: //"jpc"
		if (vm->RF[ins.r] == 0) {
			vm->PC = ins.m;
		}
		break;
	case 9: //"sio"
		fprintf(vmOut, "%d ", vm->RF[ins.r]);
		break;
	case 10: //"sio"
		printf("\nPlease type an integer to store to a register. Press enter when finished\n");
		fscanf(vmIn, "%d", &vm->RF[ins.r]);
		break;
	case 11: //"sio"
		return HALT;
	case 12: //"neg"
		vm->RF[ins.r] = -vm->RF[ins.l];
		break;
	case 13: //"add"
		vm->RF[ins.r] = vm->RF[ins.l] + vm->RF[ins.m];
		break;
	case 14: //"sub"
		vm->RF[ins.r] = vm->RF[ins.l] - vm->RF[ins.m];
		break;
	case 15: //"mul"
		vm->RF[ins.r] = vm->RF[ins.l] * vm->RF[ins.m];
		break;
	case 16: //"div"
		vm->RF[ins.r] = vm->RF[ins.l] / vm->RF[ins.m];
		break;
	case 17: //"odd"
		vm->RF[ins.r] = vm->RF[ins.l] % 2;
		break;
	case 18: //"mod"
		vm->RF[ins.r] = vm->RF[ins.l] % vm->RF[ins.m];
		break;
	case 19: //"eql"
		vm->RF[ins.r] = vm->RF[ins.l] == vm->RF[ins.m];
		break;
	case 20: //"neq"
		vm->RF[ins.r] = vm->RF[ins.l] != vm->RF[ins.m];
		break;
	case 21: //"lss"
		vm->RF[ins.r] = vm->RF[ins.l] < vm->RF[ins.m];
		break;
	case 22: //"leq"
		vm->RF[ins.r] = vm->RF[ins.l] <= vm->RF[ins.m];
		break;
	case 23: //"gtr"
		vm->RF[ins.r] = vm->RF[ins.l] > vm->RF[ins.m];
		break;
	case 24: //"geq"
		vm->RF[ins.r] = vm->RF[ins.l] >= vm->RF[ins.m];
		break;
	default:
		fprintf(stderr, "Illegal instruction?");
		return HALT;
	}

	return CONT;
}

/**
* inp: The FILE pointer containing the list of instructions to
*         be loaded to code memory of the virtual machine.
*
* outp: The FILE pointer to write the simulation output, which
*       contains both code memory and execution history.
*
* vm_inp: The FILE pointer that is going to be attached as the input
*         stream to the virtual machine. Useful to feed input for SIO
*         instructions.
*
* vm_outp: The FILE pointer that is going to be attached as the output
*          stream to the virtual machine. Useful to save the output printed
*          by SIO instructions.
* */
void simulateVM(
	FILE* inp,
	FILE* outp,
	FILE* vm_inp,
	FILE* vm_outp)	
{
	int todoNext;
	int numOfIns;
	Instruction ins[MAX_CODE_LENGTH] = { 0 };
	// Read instructions from file
	// TODO
	numOfIns = readInstructions(inp, ins);

	// Dump instructions to the output file
	// TODO
	dumpInstructions(outp, ins,numOfIns);

	// Before starting the code execution on the virtual machine,
	// .. write the header for the simulation part (***Execution***)
	fprintf(outp, "\n***Execution***\n");
	fprintf(
		outp,
		"%3s %3s %3s %3s %3s %3s %3s %3s %3s \n",         // formatting
		"#", "OP", "R", "L", "M", "PC", "BP", "SP", "STK" // titles
	);

	// Create a virtual machine
	// TODO
	VirtualMachine vm;

	// Initialize the virtual machine
	// TODO
	initVM(&vm);

	// Fetch&Execute the instructions on the virtual machine until halting
	while (1 /* TODO: Until halt is signalled.. */)
	{
		// Fetch
		// TODO
		vm.IR = vm.PC;

		// Advance PC - before execution!
		// TODO
		vm.PC++;

		// Execute the instruction
		// TODO
		todoNext = executeInstruction(&vm, ins[vm.IR], vm_inp, vm_outp);
		// Print current state
		// TODO: Following is a possible way of printing the current state
		// .. where instrBeingExecuted is the address of the instruction at vm
		// ..  memory and instr is the instruction being executed.
		 fprintf(
		outp,
		"%3d %3s %3d %3d %3d %3d %3d %3d ",
		vm.IR, // place of instruction at memory
		opcodes[ins[vm.IR].op], ins[vm.IR].r, ins[vm.IR].l, ins[vm.IR].m, // instruction info
		vm.PC, vm.BP, vm.SP //vm.stack[vm.SP] // vm info
		);

		// Print stack info
		// TODO
		dumpStack(outp, vm.stack, vm.SP, vm.BP);
		fprintf(outp, "\n");

		if (todoNext == HALT || vm.PC == 0 || vm.PC==numOfIns) {
			break;
		}
	}

	// Above loop ends when machine halts. Therefore, dump halt message.
	fprintf(outp, "HLT\n");
	return;
}
