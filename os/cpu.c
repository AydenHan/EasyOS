#include "cpu.h"

/******************ȫ�ֱ�������***************/

TCB_Ptr   TCB_CurP;        
TCB_Ptr   TCB_NextP;

u32  OSTick;
u32  CPU_SR;   
u8   SCHEDULE = 1;

u32       TaskReadyTable;      
TASK_TCB  TCB[TASKS_NUM + 1];   
PRIO_TypeDef	PrioCur;		          
PRIO_TypeDef	PrioNext;             	

/*
 --- ��������                                                                    
 - *p_Task:  ��������ַ
 - *p_Stack������ջ��ָ��
 - prio:     �������ȼ�
*/
void STM32_TaskCreate(TASK_TypeDef task, STACK_TypeDef *stack, PRIO_TypeDef prio)
{
    STACK_TypeDef  *p_stk;
    p_stk      = stack;                           //����ջ��ָ��
    p_stk      = (STACK_TypeDef *)((STACK_TypeDef)(p_stk) & 0xFFFFFFF8u);
    
    *(--p_stk) = (STACK_TypeDef)0x01000000uL;     // xPSR
    *(--p_stk) = (STACK_TypeDef)task;             // Entry Point��PCָ�룩
    // �����壬ֻΪ��--p_stk
    *(--p_stk) = (STACK_TypeDef)0x14141414uL;     // R14 (LR)
    *(--p_stk) = (STACK_TypeDef)0x12121212uL;     // R12
    *(--p_stk) = (STACK_TypeDef)0x03030303uL;     // R3
    *(--p_stk) = (STACK_TypeDef)0x02020202uL;     // R2
    *(--p_stk) = (STACK_TypeDef)0x01010101uL;     // R1
    *(--p_stk) = (STACK_TypeDef)0x00000000u;      // R0
    *(--p_stk) = (STACK_TypeDef)0x11111111uL;     // R11
    *(--p_stk) = (STACK_TypeDef)0x10101010uL;     // R10
    *(--p_stk) = (STACK_TypeDef)0x09090909uL;     // R9
    *(--p_stk) = (STACK_TypeDef)0x08080808uL;     // R8
    *(--p_stk) = (STACK_TypeDef)0x07070707uL;     // R7
    *(--p_stk) = (STACK_TypeDef)0x06060606uL;     // R6
    *(--p_stk) = (STACK_TypeDef)0x05050505uL;     // R5
    *(--p_stk) = (STACK_TypeDef)0x04040404uL;     // R4
    
    TCB[prio].StackTop = p_stk;
    TCB[prio].Delay = 0;
    TCB[prio].State = TASK_READY;
}

void OS_Start(void)
{
    TCB_NextP = &TCB[PrioNext];
    CPU_Start();
}

/*
 - SysTick_Config : �ں˺�������ʼ��Systick���������������жϣ�����Ϊ�����жϼ�Ľ�����
 - SystemCoreClock : STM32F1����ı�����ϵͳʱ��Ƶ��
*/
void SysTickInit()
{
	  SysTick_Config(SystemCoreClock / TICKS_PER_SEC);
}

extern void SysTaskSchedule(void);

void SysTick_Handler(void)
{
    ENTER_CRITICAL();    //�����ٽ���
    OSTick++;
    SysTaskSchedule();
    EXIT_CRITICAL();     //�˳��ٽ���
}
