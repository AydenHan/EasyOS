#include "os.h"

#define IdleStackSize	50		                 
static STACK_TypeDef	IdleStack[IdleStackSize]; 			

/*
 --- �������������б���
 - prio: �������ȼ�
*/
#define SetTaskReady(prio)	(	TaskReadyTable |= 0x01<<prio	)	

/*
 --- ������Ӿ����б����Ƴ�
 - prio: �������ȼ�
*/							          
#define DelTaskReady(Prio)	(	TaskReadyTable &= ~(0x01<<Prio)	)

/*
 --- ��ʼ��ϵͳ
*/
void Sys_Init(void)
{
	TaskReadyTable = 0;
	PrioCur = PrioNext = TASKS_NUM;
	SysTickInit();
}

/*
 --- ϵͳ����
*/
void Sys_Start(void)
{
	u32 i;
	TaskCreate(IdleTask, &IdleStack[IdleStackSize-1], TASKS_NUM);			// ����һ����������
	
	for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);	// ����������ȼ�������
	
    PrioCur = PrioNext = i;   // ����������ȼ�����
	
    OS_Start();              							
}

/*
 --- ��������                                                                    
 - *p_Task:  ��������ַ
 - *p_Stack������ջ��ָ��
 - prio:     �������ȼ�
*/
void TaskCreate(void (*p_Task)(void), STACK_TypeDef *p_Stack, PRIO_TypeDef prio)
{
    if(prio <= TASKS_NUM)
    {
		STM32_TaskCreate(p_Task, p_Stack, prio);  	
		SetTaskReady(prio);      			 
    }
}

/*
 --- �������
 - prio: �������ȼ�
*/
void TaskSuspend(PRIO_TypeDef prio)
{
	u32 i;
	
	ENTER_CRITICAL();
	TCB[prio].Delay = 0;
	TCB[prio].State = TASK_SUSPEND;
	DelTaskReady(prio);				// �������������ȥ����־λ
	EXIT_CRITICAL();
		
	if(PrioCur == prio)				// ��Ҫ���������Ϊ��ǰ����	���µ���		
	{
		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}	
}

/*
 --- ����ָ�
 - prio: �������ȼ�
*/
void TaskResume(PRIO_TypeDef prio)
{
	u32 i;

	ENTER_CRITICAL();
	SetTaskReady(prio);				// ����������������ñ�־λ	
    TCB[prio].Delay = 0;			// ��ʱ���ʱ��Ϊ0,��ʱ��	
    TCB[prio].State = TASK_READY;	// ��ʱ���ʱ��Ϊ0,��ʱ��	
	EXIT_CRITICAL();
	
	if(PrioCur == prio)				// ��Ҫ���������Ϊ��ǰ����	���µ���	-------????????	
	{
		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}	
}

/*
 --- ������ʱ
 - ticks: ��ʱ�Ľ�����
*/
void SysTimeDelay(TICKS_TypeDef ticks)
{
	u32 i = 0;
	if (ticks)								        
	{
		ENTER_CRITICAL();
		DelTaskReady(PrioCur);				 
		TCB[PrioCur].Delay = ticks;			// ����������ʱ������   
		TCB[PrioCur].State = TASK_DELAY;	
		EXIT_CRITICAL();

		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}
}

/*
 --- ��SysTick�жϵ��ã�������ʱ������ļ�ʱ����ʱ�л�����
*/
void SysTaskSchedule(void)
{
	u32 i;

	if(SCHEDULE == 1)
	{
		for(i = 0; i < TASKS_NUM; i++)					// ˢ�¸�����ʣ����ʱʱ�� 
		{
			if(TCB[i].State == TASK_DELAY )
			{
				TCB[i].Delay--;
				if(TCB[i].Delay == 0)					// �������ɶ�ʱ����ʱ�Ĳ���
				{
					TCB[PrioCur].State = TASK_READY;	
					SetTaskReady(i);		    		//�޸���������� ʹ���������������		
				}
			}
		}

		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}
}

/*
 --- ��������
*/
void IdleTask(void)
{	
	while(1)
	{
		IdleHook();
	}
}

/*
 --- ��ȡ���е��ܽ�����
*/
u32 GetSysTick(void)
{
	return OSTick;
}

#define CPU_USAGE_CALC_TICK    1000

static	u32 totalCount= 0 ;
static	u32 count = 0;	
static float cpuUsage = 0;

void IdleHook(void)
{
	u32 tick;
	// if(totalCount == 0)			//ֻ�ڿ����ĵ�һ�ν���ִ��
	// {
	// 	NO_SCHEDULE();			//ֹͣ������
	// 	tick = GetSysTick();
	// 	while(GetSysTick() - tick < CPU_USAGE_CALC_TICK)
	// 	{
	// 		totalCount++;
	// 	}
	// 	EN_SCHEDULE();			//����������
	// }
	
	// count = 0;
	
	// tick = GetSysTick();
	// while(GetSysTick() - tick < CPU_USAGE_CALC_TICK)
	// {
	// 	count++;
		
	// }
	// if(count < totalCount)
	// {
	// 	count = totalCount - count;
	// 	cpuUsage = (float)(count*100.0/(float)totalCount);
	// }
	while(1)
	{
		tick++;
	}
}

/*
 --- ��ȡCPUʹ����
*/
float GetSysCPU(void)
{
	return cpuUsage;
}


/*
 --- ��ȡ��ջ���ʹ����
*/
u8 GetStackMaxUsage(STACK_TypeDef *stack, u32 size)
{
	u32 i;
	u8 percent;
	for(i = 0; i < size; i++)
	{
		if(stack[i] != 0)
			break;
	}
	
	i = size - i;
	percent = i*100/size;

	return percent;
}

