# Windows NMAKE makefile
APPVER=6.1
!include <win32.mak>  

all: set-pup.exe puppy.exe

info:
	-echo cc = $(cc)
	-echo cdebug = $(cdebug)
	-echo cflags = $(cflags)
	-echo cvarsmt = $(cvarsmt)
	-echo link = $(link)
	-echo ldebug = $(ldebug)
	-echo conlflags = $(conlflags)
	-echo conlibsmt = $(conlibsmt)

.c.obj:  
  $(cc) $(cdebug) $(cflags) $(cvarsmt) $*.c  

puppy.exe: compat.obj edit.obj files.obj login.obj mdmfunc.obj modemio.obj ms-asm.obj ms-c.obj msgbase.obj printf.obj pup.obj pupmain.obj quote.obj sched.obj support.obj xmodem.obj 
  $(link) $(ldebug) $(conlflags) -out:puppy.exe $** $(conlibsmt)

set-pup.exe: set-pup.obj compat.obj
  $(link) $(ldebug) $(conlflags) -out:set-pup.exe $** $(conlibsmt)

# Clean up everything
cleanall : clean
    -del *.exe

# Clean up everything but the .EXEs
clean :
    -del *.obj
    -del *.map

# from this makefile
# cc     = cl
# cdebug = -Zi -Od -DDEBUG
# cflags = -c -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo -GS -D_AMD64_=1 -DWIN64 -D_WIN64 -DWIN32 -D_WIN32 -W4 -D_WINNT -D_WIN32_WINNT=0x0601 -DNTDDI_VERSION=0x06010000 -D_WIN32_IE=0x0800 -DWINVER=0x0601
# cvarsmt = -D_MT -MTd
# link = link
# ldebug = /DEBUG /DEBUGTYPE:cv
# conlflags = /INCREMENTAL:NO /NOLOGO -subsystem:console,6.1
# conlibsmt = kernel32.lib ws2_32.lib mswsock.lib advapi32.lib

# windows 10 visual studio 2017 standard
# cl /c /ZI /W1 /WX- /diagnostics:classic /Od /Gm /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"x64\Debug\\" /Fd"x64\Debug\vc141.pdb" /Gd /TC /errorReport:prompt edit.c files.c login.c mdmfunc.c modemio.c "ms-c.c" msgbase.c printf.c pup.c pupmain.c quote.c sched.c "set-pup.c" support.c xmodem.c
# cl /c /Zi /W3 /WX- /diagnostics:classic /Od /Oy- /D _WIN32 /D _DEBUG /D _CONSOLE /Gm /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"Debug\\" /Fd"Debug\vc141.pdb" /Gd /TC /analyze- /errorReport:prompt edit.c files.c login.c mdmfunc.c modemio.c "ms-c.c" msgbase.c printf.c pup.c pupmain.c quote.c sched.c "set-pup.c" support.c xmodem.c
# link "/OUT:E:\Source\Repos\puppybbs\src\Debug\puppybbs.exe" /INCREMENTAL kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST "/MANIFESTUAC:level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG:FASTLINK "/PDB:E:\Source\Repos\puppybbs\src\Debug\puppybbs.pdb" /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT "/IMPLIB:E:\Source\Repos\puppybbs\src\Debug\puppybbs.lib" /MACHINE:X86 Debug\edit.obj


# from this makefile
# cl -Zi -Od -DDEBUG -c -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo -GS -D_AMD64_=1 -DWIN64 -D_WIN64  -DWIN32 -D_WIN32 -W4 -D_WINNT -D_WIN32_WINNT=0x0601 -DNTDDI_VERSION=0x06010000 -D_WIN32_IE=0x0800 -DWINVER=0x0601  -D_MT -MTd
# windows 7 defaults
# cl /c /ZI /W3 /WX- /sdl /Od /Oy- /D _WIN32 /D _DEBUG /D _CONSOLE /D _UNICODE /D UNICODE /Gm /EHsc /RTC1 /MDd /GS /fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"Debug\\" /Fd"Debug\vc140.pdb"


# from sdkddkver.h for windows 10
# -D_WIN32_WINNT= 0x0603 -DNTDDI_VERSION=0x06030000 -D_WIN32_IE=0x0A00 -DWINVER=0x0603