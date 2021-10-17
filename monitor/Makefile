CC = gcc

ifndef TARGET_CPU
  TARGET_CPU = x86
endif

ifeq ($(TARGET_CPU), x86)
  CPUDEFS = -D_X86_ -DWIN32 -D_WIN32
else
  ifeq ($(TARGET_CPU), x64)
    CPUDEFS = -D_AMD64 -DWIN64 -D_WIN64
  else
    CPUDEFS = -D_X86_ -DWIN32 -D_WIN32
  endif
endif

OBJDIR = $(TARGET_CPU)_gcc
OUTDIR = ..\$(TARGET_CPU)_gcc

ifndef TARGET
  TARGET = release
endif

OBJS = $(OBJDIR)\$(TARGET)\autoclean.o \
$(OBJDIR)\$(TARGET)\defs.o \
$(OBJDIR)\$(TARGET)\log.o \
$(OBJDIR)\$(TARGET)\monitor.o \
$(OBJDIR)\$(TARGET)\monutils.o \
$(OBJDIR)\$(TARGET)\patsegment.o \
$(OBJDIR)\$(TARGET)\pattern.o \
$(OBJDIR)\$(TARGET)\port.o \
$(OBJDIR)\$(TARGET)\portlist.o \
$(OBJDIR)\$(TARGET)\sec_api.o \
$(OBJDIR)\$(TARGET)\stdafx.o

DLL = $(OUTDIR)\$(TARGET)\mfilemon.dll
LIBS = -lstdc++ -lwinspool
ifeq ($(TARGET), release)
  FLAGS = -c -O2 -fomit-frame-pointer -municode -mdll -mwin32
  LFLAGS = --shared -s --enable-stdcall-fixup
  DEFS = -DNDEBUG $(CPUDEFS) -D_WINDOWS -D_USRDLL -DMFILEMON_EXPORTS -DMFILEMONLANG=0x409 -DMINGW_HAS_SECURE_API
else
  FLAGS = -c -g -municode -mdll -mwin32
  LFLAGS = --shared -g --enable-stdcall-fixup
  DEFS = -D_DEBUG $(CPUDEFS) -D_WINDOWS -D_USRDLL -DMFILEMON_EXPORTS -DMFILEMONLANG=0x409 -DMINGW_HAS_SECURE_API
endif

all : $(DLL)

clean :
	CMD.EXE /C "DEL /Q $(OBJS)"
	CMD.EXE /C "DEL /Q $(DLL)"

$(DLL) : objdir outdir $(OBJS)
	$(CC) $(LFLAGS) -o$(DLL) $(OBJS) mfilemon.def $(LIBS)

objdir :
	CMD.EXE /C "IF NOT EXIST $(OBJDIR) MKDIR $(OBJDIR)"
	CMD.EXE /C "IF NOT EXIST $(OBJDIR)\$(TARGET) MKDIR $(OBJDIR)\$(TARGET)"

outdir :
	CMD.EXE /C "IF NOT EXIST $(OUTDIR) MKDIR $(OUTDIR)"
	CMD.EXE /C "IF NOT EXIST $(OUTDIR)\$(TARGET) MKDIR $(OUTDIR)\$(TARGET)"

$(OBJDIR)\$(TARGET)\autoclean.o : ..\common\autoclean.cpp ..\common\autoclean.h ..\common\stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\autoclean.o ..\common\autoclean.cpp

$(OBJDIR)\$(TARGET)\defs.o : ..\common\defs.cpp ..\common\defs.h ..\common\stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\defs.o ..\common\defs.cpp

$(OBJDIR)\$(TARGET)\log.o : log.cpp log.h port.h stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\log.o log.cpp

$(OBJDIR)\$(TARGET)\monitor.o : monitor.cpp monitor.h pattern.h portlist.h stdafx.h ..\common\autoclean.h ..\common\monutils.h ..\common\config.h ..\common\defs.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\monitor.o monitor.cpp

$(OBJDIR)\$(TARGET)\monutils.o : ..\common\monutils.cpp ..\common\monutils.h ..\common\stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\monutils.o ..\common\monutils.cpp

$(OBJDIR)\$(TARGET)\patsegment.o : patsegment.cpp patsegment.h port.h stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\patsegment.o patsegment.cpp

$(OBJDIR)\$(TARGET)\pattern.o : pattern.cpp pattern.h patsegment.h port.h stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\pattern.o pattern.cpp

$(OBJDIR)\$(TARGET)\port.o : port.cpp port.h stdafx.h ..\common\autoclean.h ..\common\defs.h ..\common\monutils.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\port.o port.cpp

$(OBJDIR)\$(TARGET)\portlist.o : portlist.cpp portlist.h pattern.h stdafx.h ..\common\autoclean.h ..\common\monutils.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\portlist.o portlist.cpp
	
$(OBJDIR)\$(TARGET)\sec_api.o : ..\common\sec_api.c ..\common\sec_api.h ..\common\stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\sec_api.o ..\common\sec_api.c

$(OBJDIR)\$(TARGET)\stdafx.o : stdafx.cpp stdafx.h
	$(CC) $(FLAGS) $(DEFS) -o$(OBJDIR)\$(TARGET)\stdafx.o stdafx.cpp

.PHONY : all
.PHONY : clean
.PHONY : objdir
.PHONY : outdir
