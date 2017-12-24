all: set-pup puppy

CFLAGS = -Wall -Wextra
CDEBUG = -g -O1

OBJS = edit.o driver.o files.o login.o mdmfunc.o ms-c.o msgbase.o \
	printf.o pup.o pupmain.o quote.o sched.o support.o xmodem.o

.c.o:  
	$(CC) $(CDEBUG) $(CFLAGS) $(CVARS) $*.c  

puppy: $(OBJS)
	$(LINK) $(ldebug) $(conlflags) -out:puppy.exe $** $(conlibs)

set-pup: set-pup.o compat.o
	$(LINK) $(ldebug) $(conlflags) -out:set-pup.exe $** $(conlibs)

# Clean up everything
cleanall : clean
	-rm set-pup puppy

# Clean up everything but the .EXEs
clean :
	-rm *.o
	-rm *.map

check-syntax:
	$(CC) $(CDEBUG) $(CFLAGS) $(CVARS) -o /dev/null -S $(CHK_SOURCES)
