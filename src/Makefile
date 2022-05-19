#
# @(#)Makefile
#
# Copyright 2000-2001, Aaron Ardiri     (mailto:aaron@ardiri.com)
#                      Bas van de Sande (mailto:bass@infosupport.com)
# All rights reserved.
#
# This file was generated as part of the "Hexxagon" program developed 
# for the Palm Computing Platform designed by Palm: 
#
#   http://www.palm.com/ 
#
# The contents of this file is confidential and proprietrary in nature 
# ("Confidential Information"). Redistribution or modification without 
# prior consent of the original author is prohibited. 
#

# compiler commands 
PREFIX   = 
CC       = $(PREFIX)m68k-palmos-gcc
PILRC    = $(PREFIX)pilrc
BUILDPRC = $(PREFIX)build-prc
PRC2BIN  = $(PREFIX)prc2bin
ENCRYPT  = regcode/encrypt

# source / outputs
BINFILE = hexxagon
EXEC    = $(BINFILE)_$(LANG)
MSEG    = $(EXEC)-sections.o $(EXEC)-sections.ld
OBJS    = palm.o device.o graphics.o game.o help_$(LANG).o register.o gccfix.o $(MSEG)
TEMP    = tmp.out

# compiler flags 
CCFLAGS = -O2 -Wall -palmos3.5 -DLANG_$(LANG) -DPROTECTION_$(PROTECTION)
# CCFLAGS = -g -O2 -Wall -palmos3.5 -DLANG_$(LANG) -DPROTECTION_$(PROTECTION)

# compile requirements
all:
	LANG=en PROTECTION=ON  make hexxagon
	LANG=en PROTECTION=OFF make hexxagon_unprotected

hexxagon: $(EXEC) $(EXEC).def bin.stamp
	$(BUILDPRC) $(EXEC).def $(BINFILE) *.bin
	$(PRC2BIN) $(EXEC).prc > $(TEMP)
	cp @message code0007.bin regcode/
	$(ENCRYPT) code0002.bin code0006.bin
	cp @message code0007.bin
	$(BUILDPRC) $(EXEC).def *.bin
	make clean
	cp $(EXEC).prc ..

hexxagon_unprotected: $(EXEC) $(EXEC).def bin.stamp
	$(BUILDPRC) $(EXEC).def $(BINFILE) *.bin
	make clean
	cp $(EXEC).prc ../$(EXEC)_reg.prc

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(CCFLAGS) -o $(BINFILE) 

$(MSEG): $(EXEC).def
	m68k-palmos-multigen $(EXEC).def
	$(CC) -c -o $(EXEC)-sections.o $(EXEC)-sections.s

bin.stamp: $(EXEC).rcp
	$(PILRC) -q -D PROTECTION_$(PROTECTION) $(EXEC).rcp

# compile rules
.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CCFLAGS) $<

# clean-up funtions
clean:
	rm -f *.[oa] *sections.* *.bin *.hdr bin.res *.grc *~ $(TEMP) $(BINFILE)

distclean: 
	rm -f *.[oa] *sections.* *.bin *.hdr bin.res *.[pg]rc *~ $(TEMP) $(BINFILE)
