# Logging
## printk
Plenty of options for printing to the kernel log, all based on printk with an optional log level, like `printk(KERN_INFO "Info level something");`, or use simpler `pr_info`, `pr_error` and so on.
## dmesg
Useful for viewing or following the kernel ring buffer. Offers built-in customization and filtering. Another way (with some extra info) is `cat /dev/kmsg`, while `/proc/kmsg` is also available but works a bit differently. Eventually in most distros the logs are being saved into files using various system services like rsyslog, journald, and others.
# ps
List all threads (LWPs)
```
ps -eLo pid,tid,class,rtprio,ni,pri,psr,pcpu,stat,wchan:14,comm
```
* PID: Process ID
* TID: Thread ID
* CLS: Scheduling class (e.g., TS for time-sharing, FF for FIFO, etc.)
* RTPRIO: Real-time priority (only relevant for real-time scheduling classes)
* NI: Nice value (affects priority for time-sharing processes)
* PRI: Priority (higher values mean lower priority)
* PSR: Processor (CPU) that the process is currently assigned to
* %CPU: CPU usage of the thread
* STAT: Process state (e.g., S for sleeping, R for running, etc.)
* WCHAN: Waiting channel (kernel function in which the thread is sleeping, truncated to 14 characters)
* COMMAND: Command name of the thread
# strace
Make sure at least `first.ko` is loaded. Check `cat /dev/first`
```
strace ./user 1
strace -e trace=access,openat,close ./user 1
strace -c ./user 1
```
# Sysfs: /sys
Structured representation of device and system attributes (since Linux 2.6, 2003). Organized in a hierarchical structure that mirrors the kernel’s object model, including device classes (/sys/class), devices (/sys/devices), and module information (/sys/module).

A look inside a loaded module.
```
hexdump -C /sys/module/first/notes/.note.gnu.build-id
readelf -n first.ko | grep "Build ID"
```
# Dynamic debug
* `pr_debug`: This macro expands to dynamic_pr_debug() if CONFIG_DYNAMIC_DEBUG is set. Otherwise, if DEBUG is defined, it’s equivalent to a printk with KERN_DEBUG loglevel. If DEBUG is not defined it does nothing.
* `dev_dbg`: Also a dynamic debugging function made for device drivers specifically.
* All these are disabled by default at the start while dynamic debugging, unless `ccflags += -DDEBUG` or `#define DEBUG` is set.
```
cd /sys/kernel/debug/dynamic_debug
echo 'func update_indicator_thread +p' > control
echo 'file first.c line 101 -p' > control
echo 'module second +p' > control
rmmod second
insmod second.ko debug_level=1
```
* More options:
    * `p` enables the pr_debug() callsite.
    * `f` Include the function name
    * `l` Include line number
    * `m` Include module name
    * `t` Include thread ID
# sysctl and /proc
General interface for kernel and process information (since early 1990s). Includes files and directories representing system information, such as `/proc/cpuinfo`, `/proc/meminfo`, and per-process directories (`/proc/<pid>/`).

Loglevel sequence for printk: `<console> <default_message> <min_console> <default_console>`
1. Console - log level for messages to be printed to the console.
2. Default - log level for messages without an explicitly set log level (like `printk("just a message")`).
3. Minimum - console log level override for minimum severity.
4. Default console - boot time log level setting.
```
sysctl -w kernel.printk="7 4 1 7"
echo "9 4 1 9" > /proc/sys/kernel/printk
sysctl -a | grep "second\."
sysctl -w second.debug_level=1
sysctl -w second.message="Now hey from the above!"
cat /proc/sys/second/message
sysctl -w second.trigger_action=1 # Check the dmesg now!
```
# Oops
```
sysctl kernel.panic_on_oops
> kernel.panic_on_oops = 0
echo 0 > /proc/sys/kernel/panic_on_oops
./oops
```
# ftrace
This is not a tool but a subsystem. There are tools that provide helpful wrappers, like the useful trace-cmd that won't be discussed here.
```
echo function > /sys/kernel/tracing/current_tracer
echo ':mod:first' > /sys/kernel/tracing/set_ftrace_filter
echo > /sys/kernel/tracing/trace
echo 1 > /sys/kernel/tracing/tracing_on
echo 0 > /sys/kernel/tracing/tracing_on
cat /sys/kernel/tracing/trace

echo > /sys/kernel/tracing/set_ftrace_filter
echo function_graph > /sys/kernel/tracing/current_tracer
echo get_my_indicator > /sys/kernel/tracing/set_graph_function
echo > /sys/kernel/tracing/trace
echo 1 > /sys/kernel/tracing/max_graph_depth
echo 1 > /sys/kernel/tracing/tracing_on
echo 0 > /sys/kernel/tracing/tracing_on
cat /sys/kernel/tracing/trace
```
# SysRq
Mostly underrated, extremely useful. Easy to add custom operations.
```
cat /proc/sys/kernel/sysrq
echo ' ' > /proc/sysrq-trigger
echo p > /proc/sysrq-trigger
echo m > /proc/sysrq-trigger
echo y > /proc/sysrq-trigger
```
# perf
Very useful and versatile performance tracing tool.
```
ps -e | grep indicator
perf stat -e context-switches,cache-misses,cpu-clock -p <PID> -- sleep 10

perf top -p <PID> # top functions by resources

perf record -g -p 738 -- bash -c "for i in {1..1000}; do ./user 1; done" # function call profiling, interactive
perf report
```
# GDB
Can't debug a running kernel directly (for that there's the KDB and `kgdb_breakpoint()`), not designed for that. Must be used with a gdb server:
* Built-in in Qemu, called `gdbstub`, and enabled with `-s` or `-gdb tcp::1234` and optionally `-S`. Kernel with KDBG support is not required.
* KGDB by adding to Linux cmdline (hardware and driver support is required, especially for network):
    * For serial: `kgdboc=ttyS0,115200` (oc = over console)
    * For network: `kgdboe=eth0,192.168.0.10,192.168.0.11` (oe = over ethernet)
    * Optionally add `kgdbwait`, similar to `-S` in Qemu.
* Breakpoints:
    * `break` - Software breakpoint, insering a function in code, like `INT3` on x86 and `BKPT` on ARM.
    * `hbreak` - Hardware breakpoint, 
```
gdb vmlinux

(gdb) source scripts/gdb/vmlinux-gdb.py
(gdb) target remote :1234
(gdb) lx-lsmod
(gdb) lx-symbols ../session1/example1/
(gdb) p my_ioctl
(gdb) lx-ps
(gdb) p -pretty -- *(struct task_struct *) <ADDRESS>
(gdb) p -pretty -- (*(struct task_struct *) <ADDRESS>)->pid
(gdb) p -pretty -- $lx_task_by_pid(333)->pid

(gdb) p -pretty -- $lx_module("first")
(gdb) p -pretty -- $lx_module("first")->state

(gdb) p $lx_module("first")->num_syms
(gdb) p *(struct kernel_symbol*)$lx_module("first")->syms
(gdb) p *(struct kernel_symbol*)($lx_module("first")->syms + 1)
(gdb) p *(struct kernel_symbol*)($lx_module("first")->syms + 2)
(gdb) p (char*)&$lx_module("first")->syms[0]->name_offset + $lx_module("first")->syms[0]->name_offset
(gdb) p (char*)&$lx_module("first")->syms[1]->name_offset + $lx_module("first")->syms[1]->name_offset
(gdb) p (char*)&$lx_module("first")->syms[2]->name_offset + $lx_module("first")->syms[2]->name_offset

(gdb) layout src # Or swith with 'Ctrl + x o', 'Ctrl + x a' exit
(gdb) break panic

echo c > /proc/sysrq-trigger

(gdb) break my_ioctl # Why two breakpoints?
(gdb) info breakpoints
(gdb) disassemble my_ioctl # The reason is multiple entry points.
(gdb) list 45,65
(gdb) backtrace
(gdb) info args
(gdb) info locals # Stuff that's optimized out is just not yet there in the regs.
(gdb) print cmd
(gdb) set cmd = 0x141
(gdb) continue
```
