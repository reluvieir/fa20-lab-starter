#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lfsr.h"

// Linear Feedback Shift Register 线性反馈移位寄存器模拟

void lfsr_calculate(uint16_t *reg) {    // uint16_t 表示整个整数占16位bit，每一格仍然只是1bit，这里传入的是一个整数的地址，不是数组
    /* YOUR CODE HERE */
    int num_0 = *reg & 1;
    int num_2 = (*reg>>2) & 1;
    int num_3 = (*reg>>3) & 1;
    int num_5 = (*reg>>5) & 1;

    int temp = ((num_0 ^ num_2) ^ num_3) ^ num_5;

    *reg = (*reg>>1) | (temp<<15);
}

