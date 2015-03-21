    IMPORT __use_no_semihosting
    IMPORT __use_two_region_memory
;    IMPORT __use_realtime_heap
    
    EXPORT _sys_exit
    EXPORT _ttywrch
    EXPORT __user_initial_stackheap

    AREA     globals,CODE,READONLY
    
_sys_exit
	b {pc}
	
_ttywrch
	bx lr
	
__user_initial_stackheap
	;r0 中的堆基址
	;r1 中的堆栈基址，即堆栈区中的最高地址
	;r2 中的堆限制
	;r3 中的堆栈限制，即堆栈区中的最低地址
	
	IMPORT bottom_of_heap ; defined in heap.s

	ldr r0,=bottom_of_heap
	ldr r2,=0x023ff000-0x23e00-0x13e00-0x21800-0x10000-0xf00 ;
	ldr r1,=0x0b003f00
	ldr r3,=0x0b000000
	
	bx lr

    END
    
