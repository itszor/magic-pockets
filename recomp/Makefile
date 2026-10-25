# Static recompilation of Magic Pockets: the game (gen/: Pockets as Loader
# unpacks it at &8000, ../tools/unpack.py), the intro (genintro/: Intro,
# likewise), Loader itself (genload/, at &707E0) and TrackerModule
# (gentracker/, at &1800000), from SuperSpider's analyse and gen, and game.c,
# linked with libroger (the RISC OS runtime).
ROGER   ?= $(HOME)/stuff/libroger
CC      ?= gcc
CFLAGS  ?= -O2 -g -w -fno-strict-aliasing $(if $(TRACE),-DRECOMP_TRACE)
GEN     := $(wildcard gen/code*.c gen/entries.c genintro/code*.c genintro/entries.c \
             genload/code*.c genload/entries.c gentracker/code*.c gentracker/entries.c)
OBJ     := $(GEN:.c=.o) game.o
LIBS    := $(shell pkg-config --libs sdl3) -ljpeg -lm -lpthread
INC     := -I$(ROGER)/include

pockets: $(OBJ) $(ROGER)/libroger.a
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(ROGER)/libroger.a $(LIBS)

$(ROGER)/libroger.a: FORCE
	$(MAKE) -C $(ROGER) libroger.a

%.o: %.c $(ROGER)/include/roger.h $(ROGER)/include/roger_cpu.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Code/data analysis: SuperSpider, into ../analysis/<name>.json, then
# superspider gen. Each image's ../analysis/<name>.roots adds entry points
# to the analysis: "make roots" merges the gap entries a --trace-gaps run
# logged (untranslated.log, or LOG=...) into them, so the next "make regen"
# translates them.
SS      ?= $(HOME)/stuff/superspider
SSBIN   := $(SS)/_build/default/bin/main.exe
AN      := ../analysis
APP     := ../game/!Pockets
GAME    := ../unpacked/pockets
INTRO   := ../unpacked/intro
LOADER  := $(firstword $(wildcard $(APP)/Loader,*) $(APP)/Loader)
TRACKER := $(firstword $(wildcard $(APP)/PM,*) $(APP)/PM)
# Loader's range (&707E0, 2276 bytes) and the programs' (both at &8000, up to
# Intro's end): each is --extern to the others' analyses, as they call each
# other at fixed addresses (the programs re-enter Loader at &707FC)
LOADER_RANGE := 707e0-710c4
PROG_RANGE   := 8000-70584
LOG     ?= untranslated.log

# --roots for ../analysis/<name>.roots, if there is one
roots = $(if $(wildcard $(AN)/$(1).roots),--roots $(AN)/$(1).roots)

# $(call analyse,name,image,base[,options])
define analyse
	$(SSBIN) analyse "$(2)" $(3) $(4) --full $(call roots,$(1)) -o $(AN)/$(1).json
endef

regen:
	cd $(SS) && dune build
	mkdir -p $(AN)
	$(call analyse,pockets,$(GAME),8000,--exec a614 --extern $(LOADER_RANGE))
	$(call analyse,intro,$(INTRO),8000,--exec 8088 --extern $(LOADER_RANGE))
	$(call analyse,loader,$(LOADER),707e0,--extern $(PROG_RANGE))
	$(call analyse,tracker,$(TRACKER),1800000,--module)
	rm -rf gen genintro genload gentracker
	$(SSBIN) gen "$(GAME)" 8000 $(AN)/pockets.json --c gen --leaders=dispatch
	$(SSBIN) gen "$(INTRO)" 8000 $(AN)/intro.json --c genintro --leaders=dispatch \
	    --prefix=I --tabprefix=recompI --files=1 --covtag=I
	$(SSBIN) gen "$(LOADER)" 707e0 $(AN)/loader.json --c genload --leaders=dispatch \
	    --prefix=L --tabprefix=recompL --files=1 --covtag=L
	$(SSBIN) gen "$(TRACKER)" 1800000 $(AN)/tracker.json --c gentracker --leaders=dispatch \
	    --prefix=M --tabprefix=recompM --files=1 --covtag=M

roots:
	python3 -I $(SS)/tools/gap_roots.py "$(GAME)" 8000 $(AN)/pockets.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(INTRO)" 8000 $(AN)/intro.roots $(LOG)
	python3 -I $(SS)/tools/gap_roots.py "$(LOADER)" 707e0 $(AN)/loader.roots $(LOG)

clean:
	rm -f gen*/*.o game.o pockets

.PHONY: FORCE regen roots clean
