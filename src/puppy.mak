# Sample makefile  
APPVER=6.1
!include <win32.mak>  

all: set-pup.exe puppy.exe

.c.obj:  
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c  

puppy.exe: pupmain.obj quote.obj sched.obj support.obj xmodem.obj 
  $(link) $(ldebug) $(conlflags) -out:puppy.exe $** $(conlibs)

set-pup.exe: set-pup.obj
  $(link) $(ldebug) $(conlflags) -out:set-pup.exe $** $(conlibs)

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map

# cl /c /ZI /W3 /WX- /sdl /Od /Oy- /D WIN32 /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"Debug\\" /Fd"Debug\vc140.pdb"

