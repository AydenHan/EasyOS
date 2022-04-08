#ifndef __CPU_H
#define __CPU_H

#include "stm32f10x.h"

/*************** 结构变量定义 ***************/

typedef unsigned int  STACK_TypeDef;
typedef unsigned int  PRIO_TypeDef;
typedef unsigned int  TICKS_TypeDef;
typedef void (*TASK_TypeDef)(void);

// 任务状态
typedef enum TASK_STATUS
{
  TASK_READY = 0,//就绪
  TASK_DELAY = 1,//延时等待
	TASK_SUSPEND = 2,//挂起
} TASK_STATUS;

// TCB控制块
typedef struct 
{
  STACK_TypeDef *StackTop;      //任务栈顶
	TICKS_TypeDef Delay;          //任务延时时钟
	TASK_STATUS   State;          //任务状态
} TASK_TCB, *TCB_Ptr;

/*************** 全局变量声明 ***************/

#define TICKS_PER_SEC  1000     // 设置一秒内的时钟节拍数             
#define TASKS_NUM		   5        // 设定运行任务的数量,不包含空闲任务 

extern  u32           OSTick;
extern  u32           CPU_SR;   // 保存PRIMASK(中断开关的状态控制)
extern  u8            SCHEDULE; // 是否允许任务调度的状态控制

extern  u32           TaskReadyTable;       // 就绪任务列表
extern  TASK_TCB      TCB[TASKS_NUM + 1];   // 定义任务控制块TCB数组
extern  PRIO_TypeDef	PrioCur;		          // 当前任务的优先级
extern  PRIO_TypeDef	PrioNext;             // 下一个运行任务的优先级	
extern  TCB_Ptr       TCB_CurP;             // 当前任务的TCB指针
extern  TCB_Ptr       TCB_NextP;            // 下一个运行任务的TCB指针

/*************** 汇编函数声明 ***************/

u32  CPU_SR_Save(void);
void  CPU_SR_Restore(u32 sr);
void  CPU_TASK_SCHEDULE(void);              // 任务调度
void  CPU_Start(void);                      // 系统启动

/*************** 中断控制相关 ***************/

#define  ENTER_CRITICAL()  {  CPU_SR = CPU_SR_Save();  }   // 关中断
#define  EXIT_CRITICAL()   {  CPU_SR_Restore(CPU_SR); }

#define  NO_SCHEDULE()     {  SCHEDULE = 0; }   // 停止定时调度，但不停止定时器
#define  EN_SCHEDULE()     {  SCHEDULE = 1; }

/*************** 函数声明 ***************/

void STM32_TaskCreate(TASK_TypeDef task, STACK_TypeDef *stack, PRIO_TypeDef prio);
void OS_Start(void);
void SysTickInit(void);             

#endif
