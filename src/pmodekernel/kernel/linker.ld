ENTRY(kentry)       
SECTIONS {
  . = 0x1000;       /* Load address for kernel in memory */

  .text : { *(.text) }   
  .rodata : { *(.rodata) } 
  .data : { *(.data) }   
  .bss : { *(.bss) }     
}