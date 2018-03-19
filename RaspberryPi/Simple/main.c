
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>


#define BCM_PERI_BASE 0x20000000
#define GPIO_BASE     (BCM_PERI_BASE + 0x200000)
#define GPIO_FSEL(X)  (((X)/10)*4 + BCM_PERI_BASE) 
#define GPIO_SET_BASE (GPIO_PERI_BASE + 0x1C)
//#define GPIO_SET(X)   ((

volatile uint32_t *gpio;

int main(int argc, char **argv) {

  int fd;

  if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
    printf("Error opening /dev/mem\n");
    return -1;
  }

  gpio = (uint32_t *)mmap(0,
			  getpagesize(),
			  PROT_READ | PROT_WRITE,
			  MAP_SHARED,
			  fd,
			  GPIO_BASE);

  
  if (gpio == MAP_FAILED) {
    printf("Error mapping memory\n");
    return -1;
  }

  for (int i = 0; i < 55; i ++) {
    printf("%#08X\n", GPIO_FSEL(i));
  }

  

  
}
