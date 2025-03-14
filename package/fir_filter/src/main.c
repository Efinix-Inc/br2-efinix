#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "riscv.h"

#define MAX_SIZE	128

#define fir_reset()		opcode_R(CUSTOM0, 2, 1, 0, 10)
#define fir_begin()		opcode_R(CUSTOM0, 2, 1, 0, 13)
#define fir_weight(rs1)		opcode_R(CUSTOM0, 2, 1, rs1, 11)
#define fir_fifo_push(rs1)	opcode_R(CUSTOM0, 2, 1, rs1, 12)
#define fir_fifo_pop()		opcode_R(CUSTOM0, 2, 1, 0, 14)

void fir_sw(const float *input, const float *coeffs, float *out, int N, int M)
{
	int i, j;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			if (i - j >= 0)
				out[i] += coeffs[j] * input[i - j];
		}
	}
}

void fir_hw(const float *input, const float *coeffs, float *out, int N, int M)
{
	const int *sigs = (int *)input;
	const int *weights = (int *)coeffs;
	int *ret = (int *)out;
	int i;

	fir_reset();
	for (i = 0; i < M; i++)
		fir_weight(weights[i]);

	for (i = 0; i < N; i++)
		fir_fifo_push(sigs[i]);

	fir_begin();

	for (i = 0; i < M; i++)
		out[i] = 0;

	for (i = 0; i < N; i++)
		ret[i] = fir_fifo_pop();
}

int main()
{
	float signal[MAX_SIZE];
	float coeffs[] = {
		0.25, 0.5, 0.75, 1.0,
		1.25, 1.5, 1.75, 2.0,
		2.25, 2.5, 2.75, 3.0,
		3.25, 3.5, 3.75, 4.0
	};
	float out1[MAX_SIZE];
	float out2[MAX_SIZE];
	int slen, clen, i;
	clock_t begin, end;
	double time_spent[2] = {0.0};

	printf("*** Finite Impulse Response (FIR) Filter Demo ***\n");

	slen = sizeof(signal)/sizeof(float);
	clen = sizeof(coeffs)/sizeof(float);

	for (i = 0; i < MAX_SIZE; i++) {
		signal[i] = i;
	}

	printf("Run using FIR Custom Instruction\n");
	begin = clock();
	fir_hw(signal, coeffs, out1, slen, clen);
	end = clock();
	time_spent[0] += (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Time taken for FIR custom instruction is %lf\n", time_spent[0]);

	printf("\nRun using FIR software\n");
	begin = clock();
	fir_sw(signal, coeffs, out2, slen, clen);
	end = clock();
	time_spent[1] += (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Time taken for FIR software is %lf\n", time_spent[1]);

	printf("\nSummary\n");
	printf("function\ttime\n");
	printf("-----------------------\n");
	printf("fir_hw()\t%lf\n", time_spent[0]);
	printf("fir_sw()\t%lf\n\n", time_spent[1]);
	printf("fir_hw() speed up: %lf\n", time_spent[1] / time_spent[0]);

	return 0;
}
