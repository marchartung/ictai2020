##
##  Template makefile for Standard, Profile, Debug, Release, and Release-static versions
##
##    eg: "make rs" for a statically linked release version.
##        "make d"  for a debug version (no optimizations).
##        "make"    for the standard version (optimized, but with debug information and assertions active)
##	  	  "make da" for debug with analyzer
##		  "make ra" for release with analyzer

PWD        = $(shell pwd)
EXEC      ?= $(notdir $(PWD))

CSRCS      = $(wildcard $(PWD)/*.cc) 
DSRCS      = $(foreach dir, $(DEPDIR), $(filter-out $(MROOT)/$(dir)/Main.cc, $(wildcard $(MROOT)/$(dir)/*.cc)))
CHDRS      = $(wildcard $(PWD)/*.h)
COBJS      = $(CSRCS:.cc=.o) $(DSRCS:.cc=.o)

ACOBJS     = $(addsuffix a,  $(COBJS))
PCOBJS     = $(addsuffix p,  $(COBJS))
PACOBJS     = $(addsuffix pa,  $(COBJS))
DCOBJS     = $(addsuffix d,  $(COBJS))
DACOBJS     = $(addsuffix da,  $(COBJS))
RCOBJS     = $(addsuffix r,  $(COBJS))
RACOBJS     = $(addsuffix ra,  $(COBJS))


CXX       ?= g++ -std=c++14 # -fsanitize=address -fno-omit-frame-pointer
CFLAGS    ?= -Wall -Wno-parentheses
LFLAGS    ?= -Wall

COPTIMIZE ?= -O3

CFLAGS    += -I$(MROOT) -D __STDC_LIMIT_MACROS -D __STDC_FORMAT_MACROS
LFLAGS    += -lz

.PHONY : s a p pa d da r rs clean 

s:	$(EXEC)
a:	$(EXEC)_ana
p:	$(EXEC)_profile
pa:	$(EXEC)_profile_ana
d:	$(EXEC)_debug
da: $(EXEC)_debug_ana
r:	$(EXEC)_release
ra: $(EXEC)_release_ana
rs:	$(EXEC)_static

libs:	lib$(LIB)_standard.a
libp:	lib$(LIB)_profile.a
libd:	lib$(LIB)_debug.a
libr:	lib$(LIB)_release.a

## Compile options
%.o:			CFLAGS +=$(COPTIMIZE) -g -D DEBUG
%.oa:			CFLAGS +=$(COPTIMIZE) -g -D DEBUG -D COMPLETESATANALYZER_ANALYZE
%.op:			CFLAGS +=$(COPTIMIZE) -pg -g -D NDEBUG
%.opa:			CFLAGS +=$(COPTIMIZE) -pg -g -D NDEBUG -D COMPLETESATANALYZER_ANALYZE
%.od:			CFLAGS +=-O0 -g -D DEBUG
%.oda:			CFLAGS +=-O0 -g -D DEBUG -D COMPLETESATANALYZER_ANALYZE
%.or:			CFLAGS +=$(COPTIMIZE) -g -D NDEBUG
%.ora:			CFLAGS +=$(COPTIMIZE) -g -D NDEBUG -D COMPLETESATANALYZER_ANALYZE

## Link options
$(EXEC):		LFLAGS += -g
$(EXEC)_ana:		LFLAGS += -g
$(EXEC)_profile:	LFLAGS += -g -pg
$(EXEC)_profile_ana:	LFLAGS += -g -pg
$(EXEC)_debug:		LFLAGS += -g
$(EXEC)_debug_ana:		LFLAGS += -g
#$(EXEC)_release:	LFLAGS += ...
$(EXEC)_static:		LFLAGS += --static

## Dependencies
$(EXEC):			$(COBJS)
$(EXEC)_ana:		$(ACOBJS)
$(EXEC)_profile:	$(PCOBJS)
$(EXEC)_profile_ana:	$(PACOBJS)
$(EXEC)_debug:		$(DCOBJS)
$(EXEC)_debug_ana:		$(DACOBJS)
$(EXEC)_release:	$(RCOBJS)
$(EXEC)_release_ana:	$(RACOBJS)
$(EXEC)_static:		$(RCOBJS)

lib$(LIB)_standard.a:	$(filter-out */Main.o,  $(COBJS))
lib$(LIB)_profile.a:	$(filter-out */Main.op, $(PCOBJS))
lib$(LIB)_debug.a:	$(filter-out */Main.od, $(DCOBJS))
lib$(LIB)_release.a:	$(filter-out */Main.or, $(RCOBJS))


## Build rule
%.o %.oa %.op %.opa %.od %.oda %.or %.ora:	%.cc
	@echo Compiling: $(subst $(MROOT)/,,$@)
	@$(CXX) $(CFLAGS) -c -o $@ $<

## Linking rules (standard/profile/debug/release)
$(EXEC) $(EXEC)_ana $(EXEC)_profile $(EXEC)_profile_ana $(EXEC)_debug $(EXEC)_debug_ana $(EXEC)_release $(EXEC)_release_ana $(EXEC)_static:
	@echo Linking: "$@ ( $(foreach f,$^,$(subst $(MROOT)/,,$f)) )"
	@$(CXX) $^ $(LFLAGS) -o $@

## Library rules (standard/profile/debug/release)
lib$(LIB)_standard.a lib$(LIB)_profile.a lib$(LIB)_release.a lib$(LIB)_debug.a:
	@echo Making library: "$@ ( $(foreach f,$^,$(subst $(MROOT)/,,$f)) )"
	@$(AR) -rcsv $@ $^

## Library Soft Link rule:
libs libp libd libr:
	@echo "Making Soft Link: $^ -> lib$(LIB).a"
	@ln -sf $^ lib$(LIB).a

## Clean rule
clean:
	@rm -f $(EXEC) $(EXEC)_ana $(EXEC)_profile $(EXEC)_profile_ana $(EXEC)_debug $(EXEC)_debug_ana $(EXEC)_release $(EXEC)_release_ana $(EXEC)_static \
	  $(COBJS) $(ACOBJS) $(PCOBJS) $(PACOBJS) $(DCOBJS) $(DACOBJS) $(RCOBJS) $(RACOBJS) *.core depend.mk 

## Make dependencies
depend.mk: $(CSRCS) $(CHDRS)
	@echo Making dependencies
	@$(CXX) $(CFLAGS) -I$(MROOT) \
	   $(CSRCS) -MM | sed 's|\(.*\):|$(PWD)/\1 $(PWD)/\1r $(PWD)/\1d $(PWD)/\1p:|' > depend.mk
	@for dir in $(DEPDIR); do \
	      if [ -r $(MROOT)/$${dir}/depend.mk ]; then \
		  echo Depends on: $${dir}; \
		  cat $(MROOT)/$${dir}/depend.mk >> depend.mk; \
	      fi; \
	  done

-include $(MROOT)/mtl/config.mk
-include depend.mk
