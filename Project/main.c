#include "os.h"
#include "uart1.h"
#include "stdio.h"

#define TASK_1_STK_SIZE 128
#define TASK_2_STK_SIZE 128
#define TASK_3_STK_SIZE 128
#define TASK_4_STK_SIZE 128

static STACK_TypeDef TASK_1_STK[TASK_1_STK_SIZE];
static STACK_TypeDef TASK_2_STK[TASK_2_STK_SIZE];
static STACK_TypeDef TASK_3_STK[TASK_1_STK_SIZE];
static STACK_TypeDef TASK_4_STK[TASK_2_STK_SIZE];

#define TASK1_PRIO 1
#define TASK2_PRIO 0
#define TASK3_PRIO 2
#define TASK4_PRIO 3

float cpu;
u32 mem;

void task_1()
{
	u8 ct = 0;
	while(1)
	{
		if(ct <= 5)
		{
			GPIOE->BSRR &= ~(1<<8);       
			GPIOE->BSRR |= 1<<24;         
		}
		else if(ct >= 5 && ct <= 10)
		{
			GPIOE->BSRR |= 1<<8;          
		}	
		else 
		{
			ct-=10;
		}
		SysTimeDelay(200);
		ct++;
	}
}

u32 task2count = 0;
void task_2()
{
  	while(1)
	{
		task2count++;
		display(task2count);
		if (task2count >= 9999) {
			task2count = 0;
		}
		SysTimeDelay(1);
	}

}

int main()
{
	delay_init(72);
	LED_Configuration();
	DigitalTube_Configuration();
	LCD_Configuration();

	Sys_Init();
	TaskCreate(task_1, &TASK_1_STK[TASK_1_STK_SIZE-1], TASK1_PRIO);
	TaskCreate(task_2, &TASK_2_STK[TASK_2_STK_SIZE-1], TASK2_PRIO);
	//TaskCreate(task_3, &TASK_3_STK[TASK_3_STK_SIZE-1], TASK3_PRIO);
	//TaskCreate(task_4, &TASK_4_STK[TASK_4_STK_SIZE-1], TASK4_PRIO);
  	Sys_Start();
  
 	return 0;
}
