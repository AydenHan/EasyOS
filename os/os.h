#ifndef __XXOS_H
#define __XXOS_H

#include "cpu.h"

void Sys_Init(void);
void Sys_Start(void);
void TaskCreate(void (*p_Task)(void), STACK_TypeDef *p_Stack, PRIO_TypeDef Prio);
void TaskSuspend(PRIO_TypeDef Prio);
void TaskResume(PRIO_TypeDef Prio);
void SysTimeDelay(TICKS_TypeDef ticks);

float GetSysCPU(void);
u32 GetSysTick(void);
u8 GetStackMaxUsage(STACK_TypeDef* stack, u32 size);

void IdleTask(void);
void IdleHook(void);

#endif
