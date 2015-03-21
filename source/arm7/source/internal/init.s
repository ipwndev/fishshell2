    .global __use_no_semihosting
    .global __use_two_region_memory
    
    .global _sys_exit
    .global _ttywrch
    .global __user_initial_stackheap
    .global bottom_of_heap

.section .text
    
_sys_exit:
	@b {pc}
	b _sys_exit
	
_ttywrch:
	bx lr
	
__user_initial_stackheap:
	@r0 中的堆基址
	@r1 中的堆栈基址，即堆栈区中的最高地址
	@r2 中的堆限制
	@r3 中的堆栈限制，即堆栈区中的最低地址
	
	@IMPORT bottom_of_heap ; defined in heap.s

	ldr r0,=bottom_of_heap
	ldr r2,=0x0380f900-0x500
	ldr r1,=0x0380f900
	ldr r3,=0

	bx lr

    
