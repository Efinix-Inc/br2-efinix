#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "riscv.h"

#define tiny_encryption_lowerword(rs1, rs2) opcode_R(CUSTOM0, 0x00, 0x00, rs1, rs2)
#define tiny_encryption_upperword(rs1, rs2) opcode_R(CUSTOM0, 0x01, 0x00, rs1 ,rs2)

#define ITERATION       1024

void software_tiny_encrypt(uint32_t v0, uint32_t v1, uint32_t *rv0, uint32_t *rv1)
{
        uint32_t sum = 0, i;
        uint32_t delta = 0x9e3779b9;
        uint32_t k0 = 0x01234567;
        uint32_t k1 = 0x89abcdef;
        uint32_t k2 = 0x13579248;
        uint32_t k3 = 0x248a0135;

        for (i = 0; i < ITERATION; i++) {
                sum += delta;
                v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
                v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        }

        *rv0 = v0;
        *rv1 = v1;
}

void hardware_tiny_encrypt(uint32_t num1, uint32_t num2,
                uint32_t *result_ci0, uint32_t *result_ci1)
{
        *result_ci0 = tiny_encryption_lowerword(num1, num2);
        *result_ci1 = tiny_encryption_upperword(0x0, 0x0);
}

int main()
{
        uint32_t num1, num2;
        uint32_t result_ci0, result_ci1, result_s0, result_s1;
        double time_spent = 0.0;
        clock_t begin, end;

        num1 = 0x84425820;
        num2 = 0xdeadbe11;

        printf("Tiny encryption custom instruction demo\n");

        begin = clock();
        hardware_tiny_encrypt(num1, num2, &result_ci0, &result_ci1);
        end = clock();
        time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
        printf("using custom instruction method: elapsed time %f seconds\n", time_spent);

        time_spent = 0.0;
        begin = clock();
        software_tiny_encrypt(num1, num2, &result_s0, &result_s1);
        end = clock();
        time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
        printf("using software method: elapsed time %f seconds\n", time_spent);

        if (result_ci0 != result_s0 || result_ci1 != result_s1) {
                printf("custom instruction and software output results mismatch\n");
                return -1;
        } else {
                printf("custom instruction and software output results are matched\n");
        }

        printf("Tiny encryption custom instruction demo run successfully\n");

        return 0;
}
