ENTRY(kentry)       
SECTIONS {
  . = 0x1000;       /* Load address for kernel in memory */

  .boot : {
    *(.boot)
  }

  .boot.data : {
    *(.boot.data)
  }

  _higher_half_lma = .;

  . = 0xC0001000 + _higher_half_lma - 0x1000;
  .text : AT(_higher_half_lma) 
   { *(.text) }   
  .rodata : AT(_higher_half_lma + SIZEOF(.text))
   { *(.rodata) } 
  .data : AT(_higher_half_lma + SIZEOF(.text) + SIZEOF(.rodata))
   { *(.data) }   
  .bss : AT(_higher_half_lma + SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data))
   { *(.bss) }     
}

