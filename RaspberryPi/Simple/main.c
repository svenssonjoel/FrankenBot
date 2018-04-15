#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include <pthread.h>
#include <time.h>

void usleep(int t) {
  struct timespec t_sleep;
  struct timespec t_rem;

  t_sleep.tv_sec = 0;
  t_sleep.tv_nsec = (t * 1000);
  
  nanosleep(&t_sleep, &t_rem); 

}


#define GPIO_SET_REG(X)      ((X)/32)
#define GPIO_SET_OFFSET(X)   ((X)%32)
#define GPIO_SET(X)          (*(volatile uint32_t*)(gpio_base + gpio_set_offset + GPIO_SET_REG(X))  = (1 << GPIO_SET_OFFSET(X)))
#define GPIO_CLR(X)          (*(volatile uint32_t*)(gpio_base + gpio_clr_offset + GPIO_SET_REG(X))  = (1 << GPIO_SET_OFFSET(X)))
#define GPIO_READ(X)         (*(volatile uint32_t*)(gpio_base + gpio_rd_offset + GPIO_SET_REG(X)) & (1 << GPIO_SET_OFFSET(X)))


const uint32_t gpio_fsel_offset = 0;
const uint32_t gpio_set_offset  = 7; // 0x1C/4;
const uint32_t gpio_clr_offset  = 10; // 0x28/4;
const uint32_t gpio_rd_offset   = 13; // 0x34/4


volatile uint32_t* gpio_base;  

volatile int left_speed = 0;
volatile int right_speed = 0; 


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

void gpio_set_input(int pin) {

  volatile uint32_t *reg_ptr = (volatile uint32_t *)((uint32_t)gpio_base + gpio_fsel_offset + (pin/10)*4); 
  uint32_t pin_tripple = (pin % 10) * 3;
  uint32_t clr_mask = 0x7;
  uint32_t curr_state = *reg_ptr;
 

  printf("gpio: %p\n", gpio_base);
  printf("Tripple: %d\n", pin_tripple);
  printf("Address: %p\n", reg_ptr);
  printf("Value: %x\n", curr_state);
  printf("--------------------\n");
  
  *reg_ptr = (curr_state & (~(clr_mask << pin_tripple)));
}


/*  ------------------------------------------------------------
    
    ------------------------------------------------------------ */ 

void *motor_control(void *a) {
  (void *) a;

  while (1) {
    //printf("motor_control\n");

    if (left_speed > 0) {
      GPIO_CLR(22);
      GPIO_SET(27);
    } else if (left_speed < 0){
      GPIO_CLR(27);
      GPIO_SET(22); 
    } else {
      GPIO_CLR(27);
      GPIO_CLR(22);
    }
    
    if (right_speed > 0) {
      GPIO_CLR(9);
      GPIO_SET(11);
    } else if (right_speed < 0) {
      GPIO_CLR(11);
      GPIO_SET(9); 
    } else {
      GPIO_CLR(11);
      GPIO_CLR(9); 
    }

    
    usleep(1); 
  }

}

float distance() {

  int i; 
  struct timespec pulse_length;
  struct timespec rem;
  pulse_length.tv_sec = 0;
  pulse_length.tv_nsec = 11000;

  struct timespec t_start;
  struct timespec t_end;

  float dist = 0;
  float diff_sec = 0; 

  // intiate pulses 
  GPIO_SET(5);
  nanosleep(&pulse_length, &rem);
  GPIO_CLR(5);
  
  while (GPIO_READ(6) == 0) {
    clock_gettime(CLOCK_MONOTONIC, &t_start);
  }

  while (GPIO_READ(6)); 

  clock_gettime(CLOCK_MONOTONIC, &t_end);

  diff_sec = (t_end.tv_sec + (t_end.tv_nsec / 1000000000.0)) -
    (t_start.tv_sec + (t_start.tv_nsec / 1000000000.0));
  

  dist = 17150 * diff_sec; 

  return dist; 
  
}


/*  ------------------------------------------------------------
    
    ------------------------------------------------------------ */ 

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

  // Initialize MOTOR IOs
  gpio_set_output(17);
  gpio_set_output(27);
  gpio_set_output(22);
 
  
  gpio_set_output(10);
  gpio_set_output(9);
  gpio_set_output(11);

  // setup sensor io 
  gpio_set_output(5); // trig
  GPIO_SET(5);
  usleep(2);
  GPIO_CLR(5);
  gpio_set_input(6); // echo 
  

  // enable motors
  GPIO_SET(17); // enable
  GPIO_SET(10); // enable

  
  
  // Create a thread 
  pthread_t motor_control_thread;

  if (pthread_create(&motor_control_thread, NULL, motor_control, NULL)) {
    printf("Error creating thread\n");
    return 1; 
  }

  while (1) {
    float dist = distance();

    if (dist < 11) {
      left_speed = -100;
      right_speed = 100;
    } else {
      left_speed = 100;
      right_speed = 100;
    }
  }
    
}
