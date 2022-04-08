	IMPORT    TCB_CurP
    IMPORT    TCB_NextP
	;IMPORT	  OSTaskSwHook
    
    ;EXPORT    OS_ENTER_CRITICAL
    ;EXPORT    OS_EXIT_CRITICAL
    EXPORT    CPU_Start
    EXPORT    PendSV_Handler
    EXPORT    CPU_TASK_SCHEDULE
	EXPORT    CPU_SR_Save
	EXPORT    CPU_SR_Restore
    
NVIC_INT_CTRL    EQU            0xE000ED04    ; Address of NVIC Interruptions Control Register（中断控制寄存器）
NVIC_PENDSVSET   EQU            0x10000000    ; Enable PendSV（触发软中断，通过给NVIC_INT_CTRL第28bit写1）
NVIC_SYSPRI14    EQU            0xE000ED22    ; System priority register (priority 14).系统优先级寄存器(14)  对应 PendSV
NVIC_PENDSV_PRI  EQU            0xFF          ; PendSV priority value (lowest). PendSV 中断优先级为最低(0xFF)
    
    PRESERVE8 ; align 8

    AREA    |.text|, CODE, READONLY 
    THUMB 

;CPSID I ;PRIMASK=1， ; 关中断
;CPSIE I ;PRIMASK=0， ; 开中断
;CPSID F ;FAULTMASK=1, ; 关异常
;CPSIE F ;FAULTMASK=0 ; 开异常

CPU_SR_Save                     ;关中断（进入临界区）
    MRS     R0, PRIMASK			;读取 PRIMASK 到 R0,R0 为返回值(PRIMASK:这个寄存器只有一个位，当这个位置 1 时 就关掉所有可屏蔽的异常)
    CPSID   I					;PRIMASK=1,关中断(NMI 和硬件 FAULT 可以响应)
    BX      LR					;返回

CPU_SR_Restore			    	;开中断（退出临界区）
    MSR     PRIMASK, R0			;读取 R0 到 PRIMASK 中,R0 为参数
    BX      LR					;返回
	
CPU_Start
    CPSID    I          
    
    LDR     R0, =NVIC_SYSPRI14       ; 初始化PendSV，并设置中断优先级为255，最低     
    LDR     R1, =NVIC_PENDSV_PRI          
    STRB    R1, [R0]                      
    
    LDR R4,  =0x0                    ; 初始化PSP为0   
    MSR    PSP, R4                           
    
    LDR    R4, =NVIC_INT_CTRL        ; 先触发一次PendSV异常
    LDR    R5, =NVIC_PENDSVSET       
    STR    R5, [R4]                   

    CPSIE    I                   

; should never get here
; a endless loop
OSStartHang                                    
    B    OSStartHang

;/******************PendSV_Handler************/
PendSV_Handler                            ;保存现场
    CPSID    I                            ;关中断，任务切换期间要关中断
    MRS     R0, PSP                       ; 异常处理或进程切换时，需要保存程序状态寄存器的值，可用该指令读出程序状态寄存器的值，然后保存。
    CBZ     R0, PendSV_Handler_NoSave     
;-----------------------------------------------------------------------------------------    
    SUB     R0, R0, #0x20                 ; R0 = R0 - 0x20;  
    ; store R4 
    STR     R4 , [R0]                     ; *R0 = R4;
    ADD     R0, R0, #0x4                  ; R0 = R0 + 0x4;
    STR     R5 , [R0]                     ; store R5 
    ADD     R0, R0, #0x4               
    STR     R6 , [R0]                     ; store R6
    ADD     R0, R0, #0x4                
    STR     R7 , [R0]                     ; store R7 
    ADD     R0, R0, #0x4            
    STR     R8 , [R0]                     ; store R8 
    ADD     R0, R0, #0x4             
    STR     R9, [R0]                      ; store R9
    ADD     R0, R0, #0x4            
    STR     R10, [R0]                     ; store R10 
    ADD     R0, R0, #0x4            
    STR     R11, [R0]                     ; store R11 
    ADD     R0, R0, #0x4            

    SUB     R0, R0, #0x20           
;----------------------------------------------------------------------------------    
    LDR     R1, =TCB_CurP               ;r0保存着SP值，要把sp赋值给当前任务控制块里的堆栈指针，即保存SP指针（OS_Tcb_CurP->StkAddr）
    LDR     R1, [R1]                    ; R1 = *R1;(R1 = TCB_CurP->StkAddr)得到OS_Tcb_CurP->StkAddr
    STR     R0, [R1]                    ; *R1 = R0;(*(TCB_CurP->StkAddr) = R0)即OS_Tcb_CurP=SP
    ; easy method
    ;SUB     R0, R0, #0x20
    ;STM     R0, {R4-R11}

PendSV_Handler_NoSave                ;恢复现场
    LDR     R0, =TCB_CurP            ; R0 = OSTCBCur;
    LDR     R1, =TCB_NextP           ; R1 = OSTCBNext;
    LDR     R2, [R1]                 ; R2 = OSTCBNext->OSTCBStkPtr;
    STR     R2, [R0]                 ; *R0 = R2;(OSTCBCur->OSTCBStkPtr = OSTCBNext->OSTCBStkPtr)
    
    LDR     R0, [R2]                 ; R0 = *R2;(R0 = OSTCBNext->OSTCBStkPtr)
    LDR     R4, [R0]                 
    ADD     R0, R0, #0x4   
    LDR     R5, [R0]                 
    ADD     R0, R0, #0x4    
    LDR     R6, [R0]                 
    ADD     R0, R0, #0x4     
    LDR     R7 , [R0]                
    ADD     R0, R0, #0x4      
    LDR     R8 , [R0]                
    ADD     R0, R0, #0x4    
    LDR     R9 , [R0]               
    ADD     R0, R0, #0x4    
    LDR     R10 , [R0]              
    ADD     R0, R0, #0x4    
    LDR     R11 , [R0]              
    ADD     R0, R0, #0x4             
    
    MSR     PSP, R0                  ; PSP = R0;(PSP = OSTCBNext->OSTCBStkPtr)

    ORR     LR, LR, #0x04           ; LR = LR | 0x04;    ; P42  ; P139 (key word: EXC_RETURN)  ; use PSP
    CPSIE     I                     ; OS_EXIT_CRITICAL();
    BX    LR                        ; return;

CPU_TASK_SCHEDULE                   ;OS context switch（任务切换）
    PUSH    {R4, R5}                
    LDR     R4, =NVIC_INT_CTRL       ; R4 = NVIC_INT_CTRL（触发 PendSV 异常）
    LDR     R5, =NVIC_PENDSVSET      ; R5 = NVIC_PENDSVSET
    STR     R5, [R4]                 ; *R4 = R5（向 NVIC_INT_CTRL 写入NVIC_PENDSVSET 触发 PendSV 中断）
    POP     {R4, R5}
    BX      LR                        ; return;
    
    align 4
    end