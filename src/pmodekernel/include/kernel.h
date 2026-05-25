#ifndef KERNEL_H
#define KERNEL_H

#define NULL ((void*)0)
__attribute__((section(".boot"))) void kentry(void);

#endif