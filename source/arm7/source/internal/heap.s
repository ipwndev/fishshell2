@;; Copyright ARM Ltd 2002. All rights reserved.

       @ AREA   Heap, DATA, NOINIT

        .global bottom_of_heap

@ Create dummy variable used to locate bottom of heap
.section .heap

bottom_of_heap:
	.skip 1


