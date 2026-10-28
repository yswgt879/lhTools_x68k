# Makefile for lhTools package
#
# Copyright 1992-2014 Christophe Gottheimer <cgh75015@gmail.com>
#
# lhTools is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License version 2 as published by the Free
# Software Foundation.  Note that I am not granting permission to redistribute
# or modify nsimII under the terms of any later version of the General Public
# License.
# 
# This program is distributed in the hope that it will be useful (or at
# least amusing), but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program (in the file "COPYING"); if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

ROOT	= $(PWD)
UP	= `dirname $(PWD)`
REFS	= $(UP)/refs/bin.refs

LHASM	= ../lhasm
LHASMFLAGS	= -EE -I $(UP)

ifeq ($(LSTFILE),yes)
LHASMFLAGS	+= -L $(@:%.bin=%.lst)
endif

BASIC	= bas.bas bas2.bas ernerl.bas getkey.bas inasm.bas m.bas te5.bas
ASM	= 2000.asm d1.asm fr1.asm mem0.asm ra1.asm s.asm tall.asm te3.asm test.asm tt.asm \
	  C.asm d2.asm ifelse.asm nop.asm raS.asm s2.asm te.asm te4.asm test4.asm ttt.asm \
	  bf.asm do.asm j.asm op.asm raise.asm strgfy.asm te1.asm te6.asm tmach.asm until.asm \
	  build.asm ernerl.asm keyw.asm ra.asm reserve.asm te2.asm tes.asm tsub.asm while.asm \
	  b.as c.as mac.as t.as t2.as v.as v2.as tljne.asm rrdrld.asm varsym.asm and.asm \
	  symnok.asm origin.asm

BINARIES = $(ASM:%.asm=%.bin) $(ASM:%.as=%.bin) $(BASIC:%.bas=%.bin)

all:	$(LHASM) tests
	@\rm -f .pipe

tests:	$(BINARIES)
	@(declare -i e=0; r=$(REFS); rm -f .tests.res \
	  echo "Checking MD5 with references..."; \
          if [ ! -e $$r ]; then \
	    echo "References file $$r is missing"; exit 1; \
	  fi; \
	  for f in *.bin; do \
	    m=`md5sum $$f | awk '{print $$1}'`; \
	    c=`grep "asm/$$f" $$r 2>/dev/null | awk '{print $$1}'`; \
	    if [ "$$c" != "$$m" -a "$$f" != "strgfy.bin" ]; then \
	      echo "Test failed for $$f"; e=$$e+1; echo -n $$e >.tests.res; \
	    fi; \
	  done; \
	  if [ -e .tests.res -a ! -z .tests.res ]; then \
            echo "`cat .tests.res` test(s) have failed! :/"; \
	    rm -f .tests.res; \
	    exit 1; \
          fi; \
	  echo "Tests passed OK :)"; exit 0 )

ernerl_bas.bin:	ernerl.bas

j.bin:		j.asm
	@($(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null 2>.pipe || \
	  (grep "j.asm line 8: !!!FATAL!!! Displacement > 255 bytes" .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -Jloop=0 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "j.asm line 8: !!!FATAL!!! Displacement > 255 bytes" .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) -J -o $@ $< >/dev/null)

t.bin:	t.as
	@($(LHASM) $(LHASMFLAGS) -D VAR55c=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "t.as line 30: !!!FATAL!!! Variable '%55c' is never defined." .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -D VAR10z=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "t.as line 165: !!!FATAL!!! Variable '%01z' is never defined." .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null)

t2.bin:	t2.as
	@($(LHASM) $(LHASMFLAGS) -D VAR55c=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "t2.as line 27: !!!FATAL!!! Variable '%55c' is never defined." .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -D VAR10z=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "t2.as line 165: !!!FATAL!!! Variable '%01z' is never defined." .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null)

tall.bin:	tall.asm
	@($(LHASM) $(LHASMFLAGS) -A DODO=BE -A MYSYM=E24A -o $@ $< >/dev/null)

test.bin:	test.asm
	@($(LHASM) $(LHASMFLAGS) -D DOERR=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep "test.asm line 17: !!!FATAL!!! SBR can not go backward" .pipe && \
           echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null)

symnok.bin:	symnok.asm
	@($(LHASM) $(LHASMFLAGS) -D USESYM=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep 'symnok.asm line 4: !!!FATAL!!! Unexpected arguments "CLA"' .pipe && \
	   echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -D USEVAR=1 -o $@ $< >/dev/null 2>.pipe || \
	  (grep 'symnok.asm line 10: !!!FATAL!!! Unexpected arguments "RCF"' .pipe && \
	   echo "Error expected -> OK"))
	@($(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null)

$(LHASM):
	@(echo "Building $(LHASM)...")
	@(cd $(UP); $(MAKE) `basename $(LHASM)`)

.SUFFIXES:	.asm .as .bas .bin
.asm.bin:
	@$(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null

.as.bin:
	@$(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null

.bas.bin:
	@$(LHASM) $(LHASMFLAGS) -o $@ $< >/dev/null

clean clean-version clean-all distclean:
	@\rm -f *.bin .pipe *.txt *.lst *.log *.frag *.sym .tests.res

