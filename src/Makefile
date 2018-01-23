all: set-pup puppy

CC = gcc
CCLD = gcc
CFLAGS = -Wall -Wextra -fdiagnostics-color=auto -fPIE -fno-omit-frame-pointer \
  -march=native
CDEBUG = -g -O0
LDFLAGS = -pie -Wl,-z,now

OBJS =  \
	compat.o edit.o driver.o files.o login.o mdmfunc.o modemio.c \
	ms-asm.o ms-c.o msgbase.o \
	pup.o pupmain.o quote.o sched.o support.o xmodem.o

.c.o:  
	$(CC) -c $(CDEBUG) $(CFLAGS) $(CVARS) $*.c  

puppy: $(OBJS)
	@rm -f puppy
	$(CCLD) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS) -pthread

set-pup: set-pup.o compat.o
	@rm -f set-pup
	$(CCLD) $(CFLAGS) $(LDFLAGS) -o $@ set-pup.o compat.o

# Clean up everything
cleanall : clean
	-rm set-pup puppy

# Clean up everything but the .EXEs
clean :
	-rm *.o
	-rm *.map

check-syntax:
	$(CC) $(CDEBUG) $(CFLAGS) $(CVARS) -o /dev/null -S $(CHK_SOURCES)
