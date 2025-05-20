/*
 * axi_reg_test.c
 *
 *  Created on: Sep 27, 2021
 *      Author: Akbar
 */


#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>		// open function
#include <unistd.h>		// close function
#include <sys/mman.h>

int axi_reg_test()
{

   // access to 32Bytes of address 0x400C_0000:
    int memoryFD = open("/dev/mem", O_RDWR);
    uint32_t *u32P = mmap(0, 0x20, PROT_READ | PROT_WRITE, MAP_SHARED, memoryFD, (off_t)0x400C0000);
    uint32_t myVal = u32P[0x18/4];
    printf("reading from axi port: %x\n", myVal);
    munmap((void*)u32P, 0x20);

    close(memoryFD);

    return 0;
}
