#include "os.h"

#define IdleStackSize	50		                 
static STACK_TypeDef	IdleStack[IdleStackSize]; 			

/*
 --- 将任务放入就绪列表中
 - prio: 任务优先级
*/
#define SetTaskReady(prio)	(	TaskReadyTable |= 0x01<<prio	)	

/*
 --- 将任务从就绪列表中移除
 - prio: 任务优先级
*/							          
#define DelTaskReady(Prio)	(	TaskReadyTable &= ~(0x01<<Prio)	)

/*
 --- 初始化系统
*/
void Sys_Init(void)
{
	TaskReadyTable = 0;
	PrioCur = PrioNext = TASKS_NUM;
	SysTickInit();
}

/*
 --- 系统启动
*/
void Sys_Start(void)
{
	u32 i;
	TaskCreate(IdleTask, &IdleStack[IdleStackSize-1], TASKS_NUM);			// 创建一个空闲任务
	
	for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);	// 查找最高优先级的任务
	
    PrioCur = PrioNext = i;   // 运行最高优先级任务
	
    OS_Start();              							
}

/*
 --- 创建任务                                                                    
 - *p_Task:  任务函数地址
 - *p_Stack：任务栈顶指针
 - prio:     任务优先级
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
 --- 任务挂起
 - prio: 任务优先级
*/
void TaskSuspend(PRIO_TypeDef prio)
{
	u32 i;
	
	ENTER_CRITICAL();
	TCB[prio].Delay = 0;
	TCB[prio].State = TASK_SUSPEND;
	DelTaskReady(prio);				// 从任务就绪表上去除标志位
	EXIT_CRITICAL();
		
	if(PrioCur == prio)				// 当要挂起的任务为当前任务	重新调度		
	{
		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}	
}

/*
 --- 任务恢复
 - prio: 任务优先级
*/
void TaskResume(PRIO_TypeDef prio)
{
	u32 i;

	ENTER_CRITICAL();
	SetTaskReady(prio);				// 从任务就绪表上重置标志位	
    TCB[prio].Delay = 0;			// 将时间计时设为0,延时到	
    TCB[prio].State = TASK_READY;	// 将时间计时设为0,延时到	
	EXIT_CRITICAL();
	
	if(PrioCur == prio)				// 当要挂起的任务为当前任务	重新调度	-------????????	
	{
		for(i = 0; (i < TASKS_NUM) && (!(TaskReadyTable & (0x01<<i))); i++);
		
		PrioNext = i;	
		PrioCur  = i;
		TCB_NextP = &TCB[i];
		CPU_TASK_SCHEDULE();
	}	
}

/*
 --- 任务延时
 - ticks: 延时的节拍数
*/
void SysTimeDelay(TICKS_TypeDef ticks)
{
	u32 i = 0;
	if (ticks)								        
	{
		ENTER_CRITICAL();
		DelTaskReady(PrioCur);				 
		TCB[PrioCur].Delay = ticks;			// 设置任务延时节拍数   
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
 --- 被SysTick中断调用，用于延时中任务的计时、定时切换任务
*/
void SysTaskSchedule(void)
{
	u32 i;

	if(SCHEDULE == 1)
	{
		for(i = 0; i < TASKS_NUM; i++)					// 刷新各任务剩余延时时间 
		{
			if(TCB[i].State == TASK_DELAY )
			{
				TCB[i].Delay--;
				if(TCB[i].Delay == 0)					// 必须是由定时器减时的才行
				{
					TCB[PrioCur].State = TASK_READY;	
					SetTaskReady(i);		    		//修改任务就绪表 使任务可以重新运行		
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
 --- 空闲任务
*/
void IdleTask(void)
{	
	while(1)
	{
		IdleHook();
	}
}

/*
 --- 获取运行的总节拍数
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
	// if(totalCount == 0)			//只在开机的第一次进入执行
	// {
	// 	NO_SCHEDULE();			//停止调度器
	// 	tick = GetSysTick();
	// 	while(GetSysTick() - tick < CPU_USAGE_CALC_TICK)
	// 	{
	// 		totalCount++;
	// 	}
	// 	EN_SCHEDULE();			//开启调度器
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
 --- 获取CPU使用率
*/
float GetSysCPU(void)
{
	return cpuUsage;
}


/*
 --- 获取堆栈最大使用率
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

