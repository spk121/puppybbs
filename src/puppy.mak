# Sample makefile  

!include <win32.mak>  

all: puppy.exe

.c.obj:  
  $(cc) $(cdebug) $(cflags) $(cvars) $*.c  

puppy.exe: support.obj xmodem.obj 
  $(link) $(ldebug) $(conflags) -out:puppy.exe $** $(conlibs) lsapi32.lib  


# cl /c /ZI /W3 /WX- /sdl /Od /Oy- /D WIN32 /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"Debug\\" /Fd"Debug\vc140.pdb"

