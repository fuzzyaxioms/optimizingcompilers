TARGET =	project
LIBS =		-lsimple -lcheck -lsuif -luseful
OBJS =		doproc.o main.o bitvecset.o print.o instr.o block.o cfg.o dfa.o \
dom.o reachdefs.o loop.o licm.o constant.o dce.o copyprop.o cse.o temp.o interpreter.o

all:            $(TARGET)
install-bin:    install-prog

include $(SUIFHOME)/Makefile.std
