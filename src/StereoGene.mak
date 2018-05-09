#****************************************************************************#
#SeSiMCMC. Looking - for - motifs by MCMC project. (c) A. Favorov 2001
#$Id: gibbslfm.mak 1034 2009-04-17 14:07:27Z favorov $
#****************************************************************************#


exename=StereoGene
od=./obj

srcdirlist=.

empty=
space=$(empty) $(empty)
includeflags = $(foreach dir,$(subst :,$(space),$(srcdirlist)),$(INCLUDEKEY)$(dir)) $(INCLUDECLOSETERM)
#this strange invocation is just preparing -I flag from srcdirlist.

OPTIMISE=YES

include ccvars

.PHONY: all objs clean

vpath %.c $(srcdirlist)
vpath %.cpp $(srcdirlist)

all: $(exename)$(EXEEXT) 

OBJS=$(od)/fourierCorr.o \
	$(od)/bTrack.o \
	$(od)/kernel.o \
	$(od)/formula.o \
	$(od)/mann.o \
	$(od)/map.o \
	$(od)/out.o \
	$(od)/outLC.o \
	$(od)/trackPrepare.o \
	$(od)/util.o \
	$(od)/mixfft.o \
	$(od)/parsePrm.o \
	$(od)/data.o \
	$(od)/sg_utils.o \
	$(od)/sparse.o \
	$(od)/main_SG.o 
	
objs:$(OBJS)

$(od)/%.o: %.c
	$(CC) $(CCFLAGS) $< -o $@
	
$(od)/%.o: %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

$(exename)$(EXEEXT): $(OBJS)
	$(CPP) -o $(exename)$(EXEEXT) $(OBJS) $(LINKFLAGS)
	chmod 755 $(exename)$(EXEEXT) 

#generated by g++ -MM *.c*
$(od)/fourierCorr.o: fourierCorr.cpp track_util.h util.h sg_util.h
$(od)/bTrack.o: bTrack.cpp track_util.h util.h sg_util.h
$(od)/kernel.o: kernel.cpp track_util.h util.h sg_util.h
$(od)/kernel.o: formula.cpp track_util.h util.h sg_util.h
$(od)/mann.o: mann.cpp track_util.h util.h sg_util.h
$(od)/map.o: map.cpp track_util.h util.h sg_util.h
$(od)/out.o: out.cpp track_util.h util.h sg_util.h
$(od)/outLC.o: outLC.cpp track_util.h util.h sg_util.h
$(od)/sparse.o: sparse.cpp track_util.h util.h sg_util.h
$(od)/trackPrepare.o: trackPrepare.cpp track_util.h util.h sg_util.h
$(od)/util.o: util.cpp track_util.h util.h sg_util.h
$(od)/mixfft.o: mixfft.c
$(od)/parsePrm.o: parsePrm.cpp track_util.h util.h sg_util.h
$(od)/data.o: data.cpp track_util.h util.h sg_util.h
$(od)/sg_utils.o: sg_utils.cpp track_util.h util.h sg_util.h
$(od)/main_SG.o: main_SG.cpp track_util.h util.h sg_util.h

clean:
	rm -f $(OBJS)
	rm -r -f *~
