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
    
NVIC_INT_CTRL    EQU            0xE000ED04    ; Address of NVIC Interruptions Control Register���жϿ��ƼĴ�����
NVIC_PENDSVSET   EQU            0x10000000    ; Enable PendSV���������жϣ�ͨ����NVIC_INT_CTRL��28bitд1��
NVIC_SYSPRI14    EQU            0xE000ED22    ; System priority register (priority 14).ϵͳ���ȼ��Ĵ���(14)  ��Ӧ PendSV
NVIC_PENDSV_PRI  EQU            0xFF          ; PendSV priority value (lowest). PendSV �ж����ȼ�Ϊ���(0xFF)
    
    PRESERVE8 ; align 8

    AREA    |.text|, CODE, READONLY 
    THUMB 

;CPSID I ;PRIMASK=1�� ; ���ж�
;CPSIE I ;PRIMASK=0�� ; ���ж�
;CPSID F ;FAULTMASK=1, ; ���쳣
;CPSIE F ;FAULTMASK=0 ; ���쳣

CPU_SR_Save                     ;���жϣ������ٽ�����
    MRS     R0, PRIMASK			;��ȡ PRIMASK �� R0,R0 Ϊ����ֵ(PRIMASK:����Ĵ���ֻ��һ��λ�������λ�� 1 ʱ �͹ص����п����ε��쳣)
    CPSID   I					;PRIMASK=1,���ж�(NMI ��Ӳ�� FAULT ������Ӧ)
    BX      LR					;����

CPU_SR_Restore			    	;���жϣ��˳��ٽ�����
    MSR     PRIMASK, R0			;��ȡ R0 �� PRIMASK ��,R0 Ϊ����
    BX      LR					;����
	
CPU_Start
    CPSID    I          
    
    LDR     R0, =NVIC_SYSPRI14       ; ��ʼ��PendSV���������ж����ȼ�Ϊ255�����     
    LDR     R1, =NVIC_PENDSV_PRI          
    STRB    R1, [R0]                      
    
    LDR R4,  =0x0                    ; ��ʼ��PSPΪ0   
    MSR    PSP, R4                           
    
    LDR    R4, =NVIC_INT_CTRL        ; �ȴ���һ��PendSV�쳣
    LDR    R5, =NVIC_PENDSVSET       
    STR    R5, [R4]                   

    CPSIE    I                   

; should never get here
; a endless loop
OSStartHang                                    
    B    OSStartHang

;/******************PendSV_Handler************/
PendSV_Handler                            ;�����ֳ�
    CPSID    I                            ;���жϣ������л��ڼ�Ҫ���ж�
    MRS     R0, PSP                       ; �쳣���������л�ʱ����Ҫ�������״̬�Ĵ�����ֵ�����ø�ָ���������״̬�Ĵ�����ֵ��Ȼ�󱣴档
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
    LDR     R1, =TCB_CurP               ;r0������SPֵ��Ҫ��sp��ֵ����ǰ������ƿ���Ķ�ջָ�룬������SPָ�루OS_Tcb_CurP->StkAddr��
    LDR     R1, [R1]                    ; R1 = *R1;(R1 = TCB_CurP->StkAddr)�õ�OS_Tcb_CurP->StkAddr
    STR     R0, [R1]                    ; *R1 = R0;(*(TCB_CurP->StkAddr) = R0)��OS_Tcb_CurP=SP
    ; easy method
    ;SUB     R0, R0, #0x20
    ;STM     R0, {R4-R11}

PendSV_Handler_NoSave                ;�ָ��ֳ�
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

CPU_TASK_SCHEDULE                   ;OS context switch�������л���
    PUSH    {R4, R5}                
    LDR     R4, =NVIC_INT_CTRL       ; R4 = NVIC_INT_CTRL������ PendSV �쳣��
    LDR     R5, =NVIC_PENDSVSET      ; R5 = NVIC_PENDSVSET
    STR     R5, [R4]                 ; *R4 = R5���� NVIC_INT_CTRL д��NVIC_PENDSVSET ���� PendSV �жϣ�
    POP     {R4, R5}
    BX      LR                        ; return;
    
    align 4
    end