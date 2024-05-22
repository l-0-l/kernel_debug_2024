obj-m += first.o second.o

# Path to the kernel build directory - adjust as needed
KDIR := /home/me/work/meetup/linux-6.8.9/modules/lib/modules/6.8.9/build

# User-space program compiler
CC := gcc

# User-space program sources
USER_BIN := user
USER_SRC := user.c
OOPS_BIN := oops
OOPS_SRC := oops.c

# Debugging aids for modules
EXTRA_CFLAGS := -g

all: modules $(USER_BIN) $(OOPS_BIN)

modules:
	make -C $(KDIR) M=$(PWD) modules C=1 EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

$(USER_BIN): $(USER_SRC)
	$(CC) -o $@ $<

$(OOPS_BIN): $(OOPS_SRC)
	$(CC) -o $@ $<

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(USER_BIN) $(OOPS_BIN)
