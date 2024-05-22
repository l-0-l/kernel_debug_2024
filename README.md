# kernel_debug_2024
Materials for Linux kernel debug meetup 2024

# File contents
* common.h - some common stuff for the rest of the files
* first.c - kernel module
* second.c - kernel module interracting with first.ko
* user.c - user space app interracting with second.ko
* oops.c - user space app interracting with second.ko
* Makefile - don't forget to adjust to your kernel location
* ins and rem - inserting and removing both modules on the target
* linux-6.8.9-configs/enable.conf - configs enabled
* linux-6.8.9-configs/disable.conf - configs disabled
* linux-6.8.9-configs/.config - the final kernel configuration file
* start_vm.sh - a script for starting qemu as needed for the demo
* presentation.md - obviously the live demo itself
