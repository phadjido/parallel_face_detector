# general options
CC = gcc-13
CFLAGS = -g -ggdb -Wall # -pg

.SUFFIXES:
.SUFFIXES: .c $(SUFFIXES)
LIBRARIES = -lm


SRCS =	Convolver.c FastConv.c FeatureMap.c NN.c Detector.c mem.c ConvolverFine.c \
	FastConvUtils.c MemUtils.c Neuron.c main.c tanh_lup.c LpiImage.c
OBJS =	Convolver.o FastConv.o FeatureMap.o NN.o Detector.o mem.o ConvolverFine.o \
	FastConvUtils.o MemUtils.o Neuron.o main.o tanh_lup.o LpiImage.o
HDRS =	Convolver.h ConvolverFine.h FastConv.h FastConvUtils.h FeatureMap.h LpiImage.h \
	MemUtils.h NN.h Neuron.h mem.h memDecl.h memValues.h tanh_lup.h	Makefile

TARGET = ../facedetect
REMOVE = rm -f

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS)  -o $(TARGET) $(OBJS) $(LDFLAGS) -lm

.c.o:
	$(CC) $(CFLAGS) -c $<


Convolver.o: Convolver.c $(HDRS)
FastConv.o: FastConv.c $(HDRS)
FeatureMap.o: FeatureMap.c $(HDRS)
NN.o: NN.c $(HDRS)
Detector.o: Detector.c $(HDRS)
mem.o: mem.c $(HDRS)
ConvolverFine.o: ConvolverFine.c $(HDRS)
FastConvUtils.o: FastConvUtils.c $(HDRS)
MemUtils.o: MemUtils.c $(HDRS)
Neuron.o: Neuron.c $(HDRS)
main.o: main.c $(HDRS)
tanh_lup.o: tanh_lup.c $(HDRS)
LpiImage.o: LpiImage.c $(HDRS)


clean:
	$(REMOVE) *.o $(TARGET)
	$(REMOVE) *.il *.da gmon.out core*

