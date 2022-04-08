#ifndef __CPU_H
#define __CPU_H

#include "stm32f10x.h"

/*************** �ṹ�������� ***************/

typedef unsigned int  STACK_TypeDef;
typedef unsigned int  PRIO_TypeDef;
typedef unsigned int  TICKS_TypeDef;
typedef void (*TASK_TypeDef)(void);

// ����״̬
typedef enum TASK_STATUS
{
  TASK_READY = 0,//����
  TASK_DELAY = 1,//��ʱ�ȴ�
	TASK_SUSPEND = 2,//����
} TASK_STATUS;

// TCB���ƿ�
typedef struct 
{
  STACK_TypeDef *StackTop;      //����ջ��
	TICKS_TypeDef Delay;          //������ʱʱ��
	TASK_STATUS   State;          //����״̬
} TASK_TCB, *TCB_Ptr;

/*************** ȫ�ֱ������� ***************/

#define TICKS_PER_SEC  1000     // ����һ���ڵ�ʱ�ӽ�����             
#define TASKS_NUM		   5        // �趨�������������,�������������� 

extern  u32           OSTick;
extern  u32           CPU_SR;   // ����PRIMASK(�жϿ��ص�״̬����)
extern  u8            SCHEDULE; // �Ƿ�����������ȵ�״̬����

extern  u32           TaskReadyTable;       // ���������б�
extern  TASK_TCB      TCB[TASKS_NUM + 1];   // ����������ƿ�TCB����
extern  PRIO_TypeDef	PrioCur;		          // ��ǰ��������ȼ�
extern  PRIO_TypeDef	PrioNext;             // ��һ��������������ȼ�	
extern  TCB_Ptr       TCB_CurP;             // ��ǰ�����TCBָ��
extern  TCB_Ptr       TCB_NextP;            // ��һ�����������TCBָ��

/*************** ��ຯ������ ***************/

u32  CPU_SR_Save(void);
void  CPU_SR_Restore(u32 sr);
void  CPU_TASK_SCHEDULE(void);              // �������
void  CPU_Start(void);                      // ϵͳ����

/*************** �жϿ������ ***************/

#define  ENTER_CRITICAL()  {  CPU_SR = CPU_SR_Save();  }   // ���ж�
#define  EXIT_CRITICAL()   {  CPU_SR_Restore(CPU_SR); }

#define  NO_SCHEDULE()     {  SCHEDULE = 0; }   // ֹͣ��ʱ���ȣ�����ֹͣ��ʱ��
#define  EN_SCHEDULE()     {  SCHEDULE = 1; }

/*************** �������� ***************/

void STM32_TaskCreate(TASK_TypeDef task, STACK_TypeDef *stack, PRIO_TypeDef prio);
void OS_Start(void);
void SysTickInit(void);             

#endif
