
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>


#define GPIO_SET_REG(X)      ((X)/32)
#define GPIO_SET_OFFSET(X)   ((X)%32)
#define GPIO_SET(X)          (*(gpio_base + gpio_set_offset + (GPIO_SET_REG(X))) |= (1 << GPIO_SET_OFFSET(X)))
#define GPIO_CLR(X)          (*(gpio_base + gpio_clr_offset + (GPIO_SET_REG(X))) |= (1 << GPIO_SET_OFFSET(X)))


const uint32_t gpio_fsel_offset = 0;
const uint32_t gpio_set_offset  = 7; // 0x1C/4;
const uint32_t gpio_clr_offset  = 10; // 0x28/4;


volatile uint32_t* gpio_base;  



void gpio_set_output(int pin) {

  volatile uint32_t *reg_ptr = (volatile uint32_t *)((uint32_t)gpio_base + gpio_fsel_offset + (pin/10)*4); 
  uint32_t pin_tripple = (pin % 10) * 3;
  uint32_t clr_mask = 0x7;
  uint32_t set_mask = 0x1;
  uint32_t curr_state = *reg_ptr;
 

  printf("gpio: %p\n", gpio_base);
  printf("Tripple: %d\n", pin_tripple);
  printf("Address: %p\n", reg_ptr);
  printf("Value: %x\n", curr_state);
  printf("--------------------\n");
  
  *reg_ptr = (curr_state & (~(clr_mask << pin_tripple)));
  *reg_ptr |= set_mask << pin_tripple;
    
}


volatile uint32_t *gpio;

int main(int argc, char **argv) {

  int fd;

  if ((fd = open("/dev/gpiomem", O_RDWR | O_SYNC)) < 0) {
    printf("Error opening /dev/gpiomem\n");
    return -1;
  }

  gpio = (uint32_t *)mmap(0,
			  4096,
			  PROT_READ | PROT_WRITE,
			  MAP_SHARED,
			  fd,
			  0);
  //0x3F200000);
  close(fd);
  
  if (gpio == MAP_FAILED) {
    printf("Error mapping memory\n");
    return -1;
  }

  printf("gpio: %p\n", gpio);
  gpio_base = (volatile uint32_t*) gpio;

 
  gpio_set_output(17); 
  gpio_set_output(27);
  gpio_set_output(22);
 
  
  gpio_set_output(10);
  gpio_set_output(9);
  gpio_set_output(11);


  GPIO_SET(17); // enable
  GPIO_SET(10); // enable

  GPIO_SET(22);
  GPIO_SET(11);

  sleep(4);
  
  GPIO_CLR(22);
  GPIO_CLR(11);

  
}
