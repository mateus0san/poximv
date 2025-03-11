#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define SUCCESS 0
#define ERROR 1

/* 32 KiB for memory and instructions */
#define MAX_MEMORY 32 * 1024

uint8_t readfile(FILE *, uint8_t *, char *, char *);
uint8_t writefile(FILE *, uint8_t *, char *);

int main(int argc, char *argv[])
{
	char *prog, *arq1, *arq2;
	FILE *input, *output;
	uint8_t memory[MAX_MEMORY];

	prog = argv[0]; /* program name */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input.hex output.out\n", prog);
		exit(10);
	}
	arq1 = argv[1];	/* input file name */
	arq2 = argv[2];	/* output file name */
	if ((input = fopen(arq1, "r")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", prog, arq1);
		exit(20);
	}
	if ((output = fopen(arq2, "w")) == NULL) {
		fprintf(stderr,  "%s: can't open %s\n", prog, arq2);
		exit(30);
	}
	if (readfile(input, memory, prog, arq1))
		exit(40);
	if (writefile(output, memory, prog))
		exit(50);
	return SUCCESS;
}

uint8_t readfile(FILE *input, uint8_t memory[], char *prog, char *arq1)
{
	int16_t c;
	char hex[] = "00";
	int8_t i = 0; // index to hex
	uint8_t *mem = memory;

	while ((c = getc(input)) != EOF) {
		if (c == '@') {
			while (( c = getc(input)) != '\n')
				;
			continue;
		}
		if (isspace(c))
			continue;
		if (isxdigit(c))
			hex[i++] = c;
		else
			fprintf(stderr, "unknown data type in input: '%c'\n", c);
		if (hex[i] == '\0') {
			i = 0;
			sscanf(hex, "%x", (unsigned int *)mem);
			if ((memory+MAX_MEMORY) == mem) {
				fprintf(stderr, "%s: %s too big.\n", prog, arq1);
				return ERROR;
			}
			mem++;
		}
	}
	return SUCCESS;
}

struct CSR {
	const char *x_label;
	uint32_t x;
} csr[] = {
	{ "mstatus", 0 },
	{ "mie", 0 },
	{ "mtvec", 0 },
	{ "mepc", 0 },
	{ "mcause", 0 },
	{ "mtval", 0 },
	{ "mip", 80 }
};
struct REGISTERS {
	const char *x_label;
	uint32_t x;
} rg[] = {
	{ "zero", 0 },
	{ "ra", 0 },
	{ "sp", 0 },
	{ "gp", 0 },
	{ "tp", 0 },
	{ "t0", 0 },
	{ "t1", 0 },
	{ "t2", 0 },
	{ "s0", 0 },
	{ "s1", 0 },
	{ "a0", 0 },
	{ "a1", 0x80200000 },
	{ "a2", 0x00001028 },
	{ "a3", 0 },
	{ "a4", 0 },
	{ "a5", 0 },
	{ "a6", 0 },
	{ "a7", 0 },
	{ "s2", 0 },
	{ "s3", 0 },
	{ "s4", 0 },
	{ "s5", 0 },
	{ "s6", 0 },
	{ "s7", 0 },
	{ "s8", 0 },
	{ "s9", 0 },
	{ "s10", 0 },
	{ "s11", 0 },
	{ "t3", 0 },
	{ "t4", 0 },
	{ "t5", 0 },
	{ "t6", 0 };
const uint32_t OFFSET = 0x80000000;

#define GET_RD(instruction) ((instruction >> 7) & 0x1F)
#define GET_RS1(instruction) ((instruction >> 15) & 0x1F)
#define GET_RS2(instruction) ((instruction >> 20) & 0x1F)
#define GET_FUNCT3(instruction) ((instruction >> 12) & 0x7)
#define GET_FUNCT7(instruction) ((instruction >> 25) & 0x7F)

/* R instructions */
void add(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:add %s,%s,%s %s=0x%08x+0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], x[rs1]+x[rs2]);
	x[rd] = x[rs1] + x[rs2];
}
void sub(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:sub %s,%s,%s %s=0x%08x-0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], x[rs1] - x[rs2]);
	x[rd] = x[rs1] - x[rs2];
}
void xor(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:xor %s,%s,%s %s=0x%08x^0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], x[rs1] ^ x[rs2]);
	x[rd] = x[rs1] ^ x[rs2];
}
void or(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:or  %s,%s,%s %s=0x%08x|0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], x[rs1] | x[rs2]);
	x[rd] = x[rs1] | x[rs2];
}
void and(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:and %s,%s,%s %s=0x%08x&0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2],  x[rs1] & x[rs2]);
	x[rd] = x[rs1] & x[rs2];
}
void sll(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:sll %s,%s,%s %s=0x%08x<<%d=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2] & 0x1F, x[rs1] << x[rs2]);
	x[rd] = x[rs1] << x[rs2];
}
void srl(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:srl %s,%s,%s %s=0x%08x>>%u=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2] & 0x1f, x[rs1] >> x[rs2]);
	x[rd] = x[rs1] >> x[rs2];
}
void sra(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:sra %s,%s,%s %s=0x%08x>>>%u=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2] & 0x1f, (int32_t)x[rs1] >> x[rs2]);
	x[rd] = (int32_t)x[rs1] >> x[rs2];
}
void slt(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:slt %s,%s,%s %s=(0x%08x<0x%08x)=%u\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], ((int32_t)x[rs1]) < ((int32_t)x[rs2]) ? 1 : 0);
	x[rd] = ((int32_t)x[rs1]) < ((int32_t)x[rs2]) ? 1 : 0;
}
void sltu(const uint8_t rd, const uint8_t rs1, const uint8_t rs2, uint32_t pc, FILE *output) {
	fprintf(output, "0x%08x:sltu %s,%s,%s %s=(0x%08x<0x%08x)=%u\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], (x[rs1] < x[rs2]) ? 1 : 0);
	x[rd] = (x[rs1] < x[rs2]) ? 1 : 0;
}

void mul(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	fprintf(output, "0x%08x:mul %s,%s,%s %s=0x%08x*0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], (x[rs1] * x[rs2]) & 0xFFFFFFFF);
	x[rd] = (x[rs1] * x[rs2]) & 0xFFFFFFFF;
}
void mulh(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int64_t result = ((int64_t)(int32_t)x[rs1]) * ((int64_t)(int32_t)x[rs2]);
	fprintf(output, "0x%08x:mulh %s,%s,%s %s=0x%08x*0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], (int32_t)result >> 32);
	x[rd] = (int32_t)(result >> 32);
}
void mulsu(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int64_t result = ((int64_t)(int32_t)x[rs1]) * ((uint64_t)x[rs2]);
	fprintf(output, "0x%08x:mulhsu %s,%s,%s %s=0x%08x*0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], (int32_t)(result >> 32));
	x[rd] = (int32_t)(result >> 32);
}
void mulu(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int64_t result = (((uint64_t)(x[rs1])) * ((uint64_t)(x[rs2]))) >> 32;
	fprintf(output, "0x%08x:mulhu %s,%s,%s %s=0x%08x*0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], result & 0xFFFFFFFF);
	x[rd] = result & 0xFFFFFFFF;
}
void divr(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{	int32_t result;
	if (x[rs2] != 0)
		result = (((int32_t)x[rs1]) / ((int32_t)x[rs2]));
	else
		result = 0xFFFFFFFF;
	fprintf(output, "0x%08x:div %s,%s,%s %s=0x%08x/0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], result);
	x[rd] = result;
}
void divu(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int32_t result;
	if (x[rs2] != 0)
		result = x[rs1] / (x[rs2]);
	else
		result = 0xFFFFFFFF;
	fprintf(output, "0x%08x:divu %s,%s,%s %s=0x%08x/0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], result);
	x[rd] = result;
}
void rem(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int32_t result
	if (x[rs2] != 0)
		result = ((int32_t)x[rs1]) % ((int32_t)x[rs2]);
	else
		result = x[rs1];
	fprintf(output, "0x%08x:rem %s,%s,%s %s=0x%08x%%0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], result);
	x[rd] = result;
}
void remu(uint8_t rd, uint8_t rs1, uint8_t rs2, uint32_t pc, FILE *output)
{
	int32_t result
	if (x[rs2] != 0)
		result = x[rs1] % x[rs2];
	else
		result = x[rs1];
	fprintf(output, "0x%08x:remu %s,%s,%s %s=0x%08x%%0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], x_label[rs2], x_label[rd], x[rs1], x[rs2], result);
	x[rd] = result;
}


/* R instructions */

void R(FILE *output, const uint32_t instruction, uint8_t memory[], uint32_t pc, char *prog)
{
	const uint8_t rd = GET_RD(instruction);
	const uint8_t rs1 = GET_RS1(instruction);
	const uint8_t rs2 = GET_RS2(instruction);
	const uint8_t funct7 = GET_FUNCT7(instruction);
	const uint8_t funct3 = GET_FUNCT3(instruction);
	uint8_t status = SUCCESS;
 
	if (funct7 == 0x1) {
		switch (funct3) {
			case 0x0:
				mul(rd, rs1, rs2, pc, output);
				break;
			case 0x1:
				mulh(rd, rs1, rs2, pc, output);
				break;
			case 0x2:
				mulsu(rd, rs1, rs2, pc, output);
				break;
			case 0x3:
				mulu(rd, rs1, rs2, pc, output);
				break;
			case 0x4:
				divr(rd, rs1, rs2, pc, output);
				break;
			case 0x5:
				divu(rd, rs1, rs2, pc, output);
				break;
			case 0x6:
				rem(rd, rs1, rs2, pc, output);
				break;
			case 0x7:
				remu(rd, rs1, rs2, pc, output);
				break;
			default:
				status = ERROR;
		}
	} else {
		switch (funct3) {
		case 0x0:	/* and and sub */
			if (funct7 == 0x00)
				add(rd, rs1, rs2, pc, output);
			else if (funct7 == 0x20)
				sub(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x1:	/* sll */
			if (funct7 == 0x00)
				sll(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x2:	/* slt */
			if (funct7 == 0x00)
				slt(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x3:	/* sltu */
			if (funct7 == 0x00)
				sltu(rd, rs1, rs2, pc,output);
			else
				status = ERROR;
			break;
		case 0x4:	/* xor */
			if (funct7 == 0x00)
				xor(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x5:	/* srl and sra */
			if (funct7 == 0x00)
				srl(rd, rs1, rs2, pc, output);
			else if (funct7 == 0x20)
				sra(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x6:	/* or */
			if (funct7 == 0x00)
				or(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		case 0x7:	/* and */
			if (funct7 == 0x00)
				and(rd, rs1, rs2, pc, output);
			else
				status = ERROR;
			break;
		default:
			status = ERROR;
			break;
		}
	}
	if (status == ERROR) {
		fprintf(stderr, "unknown R:%x\n", instruction);
	}
}

void addi(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:addi %s,%s,0x%03x %s=0x%08x+0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, x[rs1] + simm);
	x[rd] = x[rs1] + simm;
}
void xori(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:xori %s,%s,0x%03x %s=0x%08x^0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, x[rs1] ^ simm);
	x[rd] = x[rs1] ^ simm;
}
void ori(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:ori %s,%s,0x%03x %s=0x%08x|0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, x[rs1] | simm);
	x[rd] = x[rs1] | simm;
}
void andi(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:andi %s,%s,0x%03x %s=0x%08x&0x%08x=0x%08x\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, x[rs1] & simm);
	x[rd] = x[rs1] & simm;
}
void slli(FILE *output, const uint8_t rd, const uint8_t rs1, const int8_t imm5, uint32_t pc) {
	fprintf(output, "0x%08x:slli %s,%s,%u %s=0x%08x<<%u=0x%08x\n", pc, x_label[rd], x_label[rs1], imm5, x_label[rd], x[rs1], imm5, x[rs1] << imm5);
	x[rd] = x[rs1] << imm5;
}
void srli(FILE *output, const uint8_t rd, const uint8_t rs1, const int8_t imm5, uint32_t pc) {
	fprintf(output, "0x%08x:srli %s,%s,%u %s=0x%08x>>%u=0x%08x\n", pc, x_label[rd], x_label[rs1], imm5, x_label[rd], x[rs1], imm5, x[rs1] >> imm5);
	x[rd] = x[rs1] >> imm5;
}
void srai(FILE *output, const uint8_t rd, const uint8_t rs1, const int8_t imm5, uint32_t pc) {
	fprintf(output, "0x%08x:srai %s,%s,%u %s=0x%08x>>>%u=0x%08x\n", pc, x_label[rd], x_label[rs1], imm5, x_label[rd], x[rs1], imm5, (int32_t)x[rs1] >> imm5);
	x[rd] = (int32_t)x[rs1] >> imm5;
}
void slti(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:slti %s,%s,0x%03x %s=(0x%08x<0x%08x)=u\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, ((int32_t)x[rs1]) < ((int32_t)simm) ? 1 : 0);
	x[rd] = ((int32_t)x[rs1]) < ((int32_t)simm) ? 1 : 0;
}
void sltiu(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:sltiu %s,%s,0x%03x %s=(0x%08x<0x%08x)=%u\n", pc, x_label[rd], x_label[rs1], simm & 0xFFF, x_label[rd], x[rs1], simm, x[rs1] < ((uint32_t)simm) ? 1 : 0);
	x[rd] = x[rs1] < ((uint32_t)simm) ? 1 : 0;
}

void Iimm(FILE *output, const uint32_t instruction, const uint8_t funct3, const uint8_t rd, const uint8_t rs1, const int16_t imm, uint32_t pc, char *prog)
{
	const int32_t simm = (imm >> 11) ? 0xFFFFF000 | imm : imm;	/* imm sign extension */
	const int8_t imm5 = imm & 0x1F;	/* imm 5 lower bits */
	const int8_t imm7 = imm >> 5;	/* imm 7 upper bits */
	int8_t status = SUCCESS;
	switch (funct3) {
		case 0x0:	/* addi */
			addi(output, rd, rs1, simm, pc);
			break;
		case 0x1:	/* slli */
			if (imm7 == 0x00)
				slli(output, rd, rs1, imm5, pc);
			else
				status = ERROR;
			break;
		case 0x2:	/* slti */
			slti(output, rd, rs1, simm, pc);
			break;
		case 0x3:
			sltiu(output, rd, rs1, simm, pc);
			break;
		case 0x4:	/* xori */
			xori(output, rd, rs1, simm, pc);
			break;
		case 0x5:	/* srli and srai */
			if (imm7 == 0x00)
				srli(output, rd, rs1, imm5, pc);
			else if (imm7 == 0x20)
				srai(output, rd, rs1, imm5, pc);
			else
				status = ERROR;
			break;
		case 0x6: /* ori */
			ori(output, rd, rs1, simm, pc);
			break;
		case 0x7: /* andi */
			andi(output, rd, rs1, simm, pc);
			break;
		default:
			status = ERROR;
			break;
	}
	if (status == ERROR)
		fprintf(stderr, "%s: unknown I instruction %x\n", prog, instruction);
}
// factoring fprintf functions, the priority is read! I stopped here
void lb(FILE *output, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	if (posi < MAX_MEMORY && posi >= 0) {
		x[rd] = (int8_t)(memory[posi]);
		fprintf(output, "0x%08x:lb %s,0x%03x(%s) %s=mem[0x%08x]=0x%08x\n", pc, x_label[rd], simm, x_label[rs1], x_label[rd], posi+OFFSET, x[rd]);
	} else printf("lb out of memory\n");
}
void lh(FILE *output, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	if (posi < MAX_MEMORY-2 && posi >= 0) {
		x[rd] = ((int16_t *)(memory+posi))[0];
		fprintf(output, "0x%08x:lh	%s,0x%03x(%s)	%s=mem[0x%08x]=0x%08x\n", pc, x_label[rd], simm, x_label[rs1], x_label[rd], posi+OFFSET, x[rd]);
	} else printf("lh out of memory\n");
}
void lw(FILE *output, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	if (posi % 4 != 0)
		printf("achei borra\n");
	if (posi < MAX_MEMORY-4 && posi >= 0) {
		x[rd] = ((int32_t *)(memory+posi))[0];
		fprintf(output, "0x%08x:lw %s,0x%03x(%s) %s=mem[0x%08x]=0x%08x\n", pc, x_label[rd], simm & 0xFFF, x_label[rs1], x_label[rd], posi+OFFSET, x[rd]);
	} else printf("lw out of memory\n");
}
void lbu(FILE *output, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	if (posi < MAX_MEMORY && posi >= 0) {
		x[rd] = memory[posi] & 0xFF;
		fprintf(output, "0x%08x:lbu %s,0x%03x(%s) %s=mem[0x%08x]=0x%08x\n", pc, x_label[rd], simm, x_label[rs1], x_label[rd], posi+OFFSET, x[rd]);
	} else printf("lbu out of memory\n");
}
void lhu(FILE *output, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	if (posi < MAX_MEMORY-2 && posi >= 0) { 
		x[rd] = (uint32_t)(((uint16_t *)(memory+posi))[0]);
		fprintf(output, "0x%08x:lhu	%s,0x%03x(%s)	%s=mem[0x%08x]=0x%08x\n", pc, x_label[rd], simm, x_label[rs1], x_label[rd], posi+OFFSET, x[rd]);
	} else printf("lhu out of memory\n");
}
void Iload(FILE *output, const uint32_t instruction, const uint8_t funct3, uint8_t memory[], const uint8_t rd, const uint8_t rs1, const int16_t imm, uint32_t *pc, char *prog)
{
	const int32_t simm = (imm >> 11) ? 0xFFFFF000 | imm : imm;
	if (imm % 4  != 0){
			//mstatus
			csr[0].x = 0x00001800;
			//mcause
			csr[4].x = 0x2;
			// *pc = mtvec
			*pc = csr[2].x - 4;
			fprintf(output, ">exception:illegal_instruction cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
			//tval = imm
			csr[5].x = simm;
			return;
	}
	if (rd == 0 || rs1 == 0) {	
			//mstatus
			csr[0].x = 0x00001800;
			//mcause
			csr[4].x = 0x2;
			// *pc = mtvec
			*pc = csr[2].x - 4;
			fprintf(output, ">exception:load_fault cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
			//tval = imm
			csr[5].x = simm;
			return;
	} 
	switch (funct3) {
		case 0x0:			
			lb(output, memory, rd, rs1, simm, *pc);
			break;
		case 0x1:	
			lh(output, memory, rd, rs1, simm, *pc);
			break;
		case 0x2:
			lw(output, memory, rd, rs1, simm, *pc);
			break;
		case 0x4:	
			lbu(output, memory, rd, rs1, simm, *pc);
			break;
		case 0x5:	
			lhu(output, memory, rd, rs1, simm, *pc);
			break;
		default:
			fprintf(stderr, "%s: unknown I instruction %x\n", prog, instruction);
			break;
	}
}

void jarl(FILE *output, const uint8_t rd, const uint8_t rs1, const int32_t simm, uint32_t *pc) {
	uint32_t aux = x[rs1];
	fprintf(output, "0x%08x:jalr %s,%s,0x%03x pc=0x%08x+0x%08x,%s=0x%08x\n", *pc, x_label[rd], x_label[rs1], simm, x[rs1], simm, x_label[rd], *pc+4);
	x[rd] = *pc + 4;
	*pc = x[rs1] + simm;
	*pc -= 4;
}
void Ijump(FILE *output, const uint32_t instruction, const uint8_t funct3, const uint8_t rd, const uint8_t rs1, const int16_t imm, uint32_t *pc, char *prog)
{
	const uint32_t simm = (imm >> 11) ? 0xFFFFF000 | imm : imm;

	if (funct3 == 0x0)
		jarl(output, rd, rs1, simm, pc);
	else
		fprintf(stderr, "%s: unknwon instruction %x\n", prog, instruction);
}
void ecall(FILE *output, uint32_t pc) {
	fprintf(output, "0x%08x:ecall\n", pc);
	exit(11);
}
void ebreak(FILE *output, uint32_t pc) {
	fprintf(output, "0x%08x:ebreak\n", pc);
	exit(0);
}
uint16_t getcsr(uint16_t csr_index)
{
	switch (csr_index) {
		case 0x300:
			return 0;
		case 0x304:
			return 1;
		case 0x305:
			return 2;
		case 0x341:
			return 3;
		case 0x342:
			return 4;
		case 0x343:
			return 5;
		case 0x344:
			return 6;
		default:
			return -1;
	}
}
void csrrw(FILE *output, const uint8_t rd, const uint8_t rs1, uint16_t c, uint32_t pc)
{
	uint32_t aux = x[rs1];
	fprintf(output, "0x%08x:csrrw %s,%s,%s %s=%s=0x%08x,%s=%s=0x%08x\n", pc, x_label[rd], csr[c].x_label, x_label[rs1], x_label[rd], csr[c].x_label, csr[c].x, csr[c].x_label, x_label[rs1], x[rs1]);
	x[rd] = csr[c].x;
	csr[c].x = aux;
}
void csrrs(FILE *output, const uint8_t rd, const uint8_t rs1, uint16_t c, uint32_t pc)
{
	uint32_t aux = x[rs1];
	fprintf(output, "0x%08x:csrrs %s,%s,%s %s=%s=0x%08x,%s|=%s=0x%08x|0x%08x=0x%08x\n", pc, x_label[rd], csr[c].x_label, x_label[rs1], x_label[rd], csr[c].x_label, csr[c].x, csr[c].x_label, x_label[rs1], csr[c].x, aux, csr[c].x | aux);
	x[rd] = csr[c].x;
	csr[c].x = csr[c].x | aux;
}
void mret(FILE *output, uint32_t *pc)
{
	fprintf(output, "0x%08x:mret pc=0x%08x\n", *pc ,csr[3].x);
	csr[0].x = 0x00000080;
	*pc = csr[3].x-4;

	
}
void Icsr(FILE *output, const uint32_t instruction, const uint8_t funct3, const int16_t imm, uint32_t *pc, char *prog, const uint8_t rd, const uint8_t rs1)
{
	uint16_t c = getcsr((uint16_t)(instruction >> 20));
	if (funct3 == 0x0 && imm == 0x0)
		ecall(output, *pc);
	else if (funct3 == 0x0 && imm == 0x1)
		ebreak(output, *pc);
	else if (imm == 0b001100000010 && funct3 == 0 && rd == 0 && rs1 == 0)
		mret(output, pc);
	else
		switch (funct3) {
			case 0x1:
				csrrw(output, rd, rs1, c, *pc);
				break;
			case 0x2:
				csrrs(output, rd, rs1, c, *pc);
				break;
			case 0x3:
				//csrrc(output, rd, rs1, csr, *pc);
			case 0x5:
				//csrrwi(output, rd, rs1, csr, *pc);
			case 0x6:
				//csrrsi(output, rd, rs1, csr, *pc);
			case 0x7:
				// csrrci(output, rd, rs1, csr, *pc);
				printf("csrs...\n");
				break;
			default:
				fprintf(stderr, "%s: unknwon instruction %x\n", prog, instruction);
		}
}

void I(FILE *output, const uint32_t instruction, uint8_t memory[], uint32_t *pc, char *prog, const uint8_t opcode)
{
	const uint8_t rd = GET_RD(instruction);
	const uint8_t rs1 = GET_RS1(instruction);
	const uint8_t funct3 = GET_FUNCT3(instruction);
	const int16_t imm = instruction >> 20;

	switch (opcode) {
		case 0b0010011:
			Iimm(output, instruction, funct3, rd, rs1, imm, *pc, prog);
			return;
		case 0b0000011:
			Iload(output, instruction, funct3, memory, rd, rs1, imm, pc, prog);
			return;
		case 0b1100111:
			Ijump(output, instruction, funct3, rd, rs1, imm, pc, prog);
			return;
		case 0b1110011:
			Icsr(output, instruction, funct3, imm, pc, prog, rd, rs1);
			return;
	}
}

void sb(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint8_t memory[], uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	memory[posi] = x[rs2] & 0xFF;
	fprintf(output, "0x%08x:sb %s,0x%03x(%s) mem[0x%08x]=0x%02x\n", pc, x_label[rs2], simm & 0xFFF, x_label[rs1], posi+OFFSET, memory[posi] & 0xFF);
}
void sh(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint8_t memory[], uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	uint16_t *mem = ((uint16_t *)(memory+posi));
	*mem = x[rs2] & 0xFFFF;
	fprintf(output, "0x%08x:sh %s,0x%03x(%s) mem[0x%08x]=0x%04x\n", pc, x_label[rs2], simm & 0xFFF, x_label[rs1], posi+OFFSET, *mem & 0xFFFF);
}
void sw(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint8_t memory[], uint32_t pc) {
	uint16_t posi = x[rs1]+simm-OFFSET;
	uint32_t *mem = ((uint32_t *)(memory+posi));
	*mem = x[rs2];
	fprintf(output, "0x%08x:sw %s,0x%03x(%s) mem[0x%08x]=0x%08x\n", pc, x_label[rs2], simm & 0xFFF, x_label[rs1], posi+OFFSET, *mem);
}

void S(FILE *output, const uint32_t instruction, uint8_t memory[], uint32_t pc, char *prog)
{
	const int16_t imm = ((instruction >> 7) & 0x1F) | ((instruction >> 25) << 5);
	const int32_t simm = (imm >> 11) ? 0xFFFFF000 | imm : imm;
	const uint8_t funct3 = GET_FUNCT3(instruction);
	const uint8_t rs1 = GET_RS1(instruction);
	const uint8_t rs2 = GET_RS2(instruction);

	switch (funct3) {
		case 0x0:
			if (rs1 == 0) {	
			//mstatus
			csr[0].x = 0x00001800;
			//mcause
			csr[4].x = 0x5;
			// pc = mtvec
			pc = csr[2].x - 4;
			fprintf(output, ">exception:store_fault cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
			//tval = instruction
			csr[5].x = simm;
			} else
			sb(output, rs1, rs2, simm, memory, pc);
			break;
		case 0x1:
			if (rs1 == 0) {	
			//mstatus
			csr[0].x = 0x00001800;
			//mcause
			csr[4].x = 0x5;
			// pc = mtvec
			pc = csr[2].x - 4;
			fprintf(output, ">exception:store_fault cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
			//tval = instruction
			csr[5].x = simm;
			} else
			sh(output, rs1, rs2, simm, memory, pc);
			break;
		case 0x2:
			if (rs1 == 0) {	
			//mstatus
			csr[0].x = 0x00001800;
			//mcause
			csr[4].x = 0x5;
			// pc = mtvec
			pc = csr[2].x - 4;
			fprintf(output, ">exception:store_fault cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
			//tval = instruction
			csr[5].x = simm;
			} else
			sw(output, rs1, rs2, simm, memory, pc);
			break;
		default:
			fprintf(stderr, "%s: unknown S instruction %x\n", prog, instruction);
			break;
	}
}
void beq(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (x[rs1] == x[rs2] && simm != 0x000)
		*pc += simm << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:beq %s,%s,0x%03x (0x%08x==0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], simm, x[rs1], x[rs2], x[rs1]==x[rs2], *pc);
	*pc -= 4;
}
void bne(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (x[rs1] != x[rs2] && simm != 0x000)
		*pc += simm << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:bne %s,%s,0x%03x (0x%08x!=0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], simm & 0xFFF, x[rs1], x[rs2], x[rs1]!=x[rs2], *pc);
	*pc -= 4;
}
void blt(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (((int32_t)x[rs1]) < ((int32_t)x[rs2]) && simm != 0x000)
		*pc += simm << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:blt %s,%s,0x%03x (0x%08x<0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], (simm & 0xFFF), x[rs1], x[rs2], (int32_t)x[rs1]<(int32_t)x[rs2], *pc);
	*pc -= 4;
}
void bge(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (((int32_t)x[rs1]) >= ((int32_t)x[rs2]) && simm != 0x000)

		*pc += simm << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:bge %s,%s,0x%03x (0x%08x>=0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], simm & 0xFFF, x[rs1], x[rs2], ((int32_t)x[rs1])>=((int32_t)x[rs2]), *pc);
	*pc -= 4;
}
void bltu(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (x[rs1] < x[rs2] && simm != 0x000)
		*pc += ((uint32_t)simm) << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:bltu %s,%s,0x%03x (0x%08x<0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], simm & 0xFFF, x[rs1], x[rs2], x[rs1]<x[rs2], *pc);
	*pc -= 4;
}
void bgeu(FILE *output, const uint8_t rs1, const uint8_t rs2, const int32_t simm, uint32_t *pc)
{
	uint32_t instAdress = *pc;
	if (x[rs1] >= x[rs2] && simm != 0x00)
		*pc += ((uint32_t)simm) << 1;
	else
		*pc += 4;
	fprintf(output, "0x%08x:bgeu %s,%s,0x%03x (0x%08x>=0x%08x)=%d->pc=0x%08x\n", instAdress, x_label[rs1], x_label[rs2], simm & 0xFFF, x[rs1], x[rs2], x[rs1]>=x[rs2], *pc);
	*pc -= 4;
}

void B(FILE *output, const uint32_t instruction, uint32_t *pc, char *prog)
{
	const int16_t imm = ((instruction >> 31) << 11) | (((instruction >> 25) & 0x3F) << 4) | (((instruction >> 8) & 0xF)) | (((instruction >> 7) & 0b1) << 10);
	const int32_t simm = (imm >> 11) ? (0xFFFFF000 | imm) : imm;
	const uint8_t funct3 = GET_FUNCT3(instruction);
	const uint8_t rs1 = GET_RS1(instruction);
	const uint8_t rs2 = GET_RS2(instruction);

	switch (funct3) {
		case 0x0:
			beq(output, rs1, rs2, simm, pc);
			break;
		case 0x1:
			bne(output, rs1, rs2, simm, pc);
			break;
		case 0x4:
			blt(output, rs1, rs2, simm, pc);
			break;
		case 0x5:
			bge(output, rs1, rs2, simm, pc);
			break;
		case 0x6:
			bltu(output, rs1, rs2, simm, pc);
			break;
		case 0x7:
			bgeu(output, rs1, rs2, simm, pc);
			break;
		default:
			fprintf(stderr, "%s: unknwon instruction %x\n", prog, instruction);
			break;
	}
}
void lui(FILE *output, const uint8_t rd, const int32_t simm, uint32_t pc) {
	fprintf(output, "0x%08x:lui %s,0x%05x %s=", pc, x_label[rd], (simm & 0xFFFFF), x_label[rd]);
	x[rd] = simm << 12;
	fprintf(output, "0x%08x\n", x[rd]);
}
void auipc(FILE *output, const uint8_t rd, const int32_t simm, uint32_t pc)
{
	x[rd] = pc + (simm << 12);
	fprintf(output, "0x%08x:auipc %s,0x%05x %s=0x%08x+0x%08x=0x%08x\n", pc, x_label[rd], simm & 0x1F, x_label[rd], pc, simm << 12, x[rd]);
}

void U(FILE *output, const uint32_t instruction, uint32_t pc, char *prog, const uint8_t opcode)
{
	const int32_t imm = instruction >> 12;
	const int32_t simm = (imm >> 19) ? 0xFFF00000 | imm : imm;
	const uint8_t rd = GET_RD(instruction);

	if (opcode == 0b0110111)
		lui(output, rd, simm, pc);
	if (opcode == 0b0010111)
		auipc(output, rd, simm, pc);
}

void jal(FILE *output, const uint8_t rd, const int32_t simm, uint32_t *pc)
{	const uint32_t instAdress = *pc;

	x[rd] = *pc + 4;
	*pc = *pc +(simm << 1);
	fprintf(output, "0x%08x:jal %s,0x%05x pc=0x%08x,%s=0x%08x\n", instAdress, x_label[rd], simm & 0xFFFFF, *pc, x_label[rd], x[rd]);
	if (*pc - OFFSET >= 4)
		*pc -= 4;
}
void J(FILE *output, const uint32_t instruction, uint32_t *pc, char *prog)
{
	const uint8_t rd = GET_RD(instruction);
	const int32_t imm20 = (((instruction >> 31) << 19) | (((instruction & (0xFF << 12)) >> 12) << 11) | (((instruction & (0b1 << 20)) >> 20) << 10) | ((instruction & (0b1111111111 << 21)) >> 21));
	const uint32_t simm = (imm20 >> 19) ? (0xFFF00000) | imm20 : (imm20);

	jal(output, rd, simm, pc);
}
uint8_t writefile(FILE *output, uint8_t memory[], char *prog)
{
	uint32_t pc = OFFSET;

	while ((pc - OFFSET) < MAX_MEMORY) {
		const uint32_t instruction = ((uint32_t *)(memory+pc-OFFSET))[0];
		const uint8_t opcode = instruction & 0x7F;
		switch (opcode) {
			case 0b0110011:
				R(output, instruction, memory, pc, prog);
				break;
			case 0b0010011:
			case 0b0000011:
			case 0b1100111:
			case 0b1110011:
				I(output, instruction, memory, &pc, prog, opcode);
				break;
			case 0b0100011:
				S(output, instruction, memory, pc, prog);
				break;
			case 0b1100011:
				B(output, instruction, &pc, prog);
				break;
			case 0b0110111:
			case 0b0010111:
				U(output, instruction, pc, prog, opcode);
				break;
			case 0b1101111:
				J(output, instruction, &pc, prog);
				break;
			default:
				//mstatus
				csr[0].x = 0x00001800;
				//mcause
				csr[4].x = 0x1;
				// pc = mtvec
				pc = csr[2].x - 4;
				fprintf(output, ">exception:instruction_fault cause=0x%08x,epc=0x%08x,tval=0x%08x\n", csr[4].x, csr[3].x, csr[5].x);
				// mtval = instruction
				csr[5].x = instruction;
				printf("unknown opcode. %x\n", instruction);
				break;
		}
		pc += 4;
		x[0] = 0;
	}
	return SUCCESS;
}
