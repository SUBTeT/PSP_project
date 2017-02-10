TARGET = MAZE
OBJS = main.o

PSPSDK=$(shell psp-config --pspsdk-path)
INCS = $(PSPSDK)/include $(PSPSDK)/DXP_v0.5.7/include
INCDIR = $(INCS)

CFLAGS = -Wall -G0 -O3 -fomit-frame-pointer -mgp32 -mlong32

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
ENCRYPT = 1

LIBDIR = $(PSPSDK)/DXP_v0.5.7/lib
LDFLAGS =
LIBS  = $(PSPSDK)/DXP_v0.5.7/lib/dxlibp_UTF8.a
LIBS += -lpspgu -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec\
        -lpsputility -lpspvalloc -lpsppower

EXTRA_TARGETS   = EBOOT.PBP
PSP_EBOOT_TITLE = MAZE
PSP_EBOOT_ICON  = NULL
PSP_EBOOT_ICON1 = NULL
PSP_EBOOT_UNKPNG= NULL
PSP_EBOOT_PIC1  = NULL
PSP_EBOOT_SND0  = NULL

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# 以下、任意で（開発しているソースに依存）
main.o          : main.c
