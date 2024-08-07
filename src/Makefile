#Make File for Gcc Windows

CFLAGS = -g -O2 -fomit-frame-pointer -Wall -I../core
#PLATFORM = Linux
#PLATFORM = Windows

PLATFORM = $(shell uname)
EXT = .exe

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
	EXT = .exe
endif

MAKE_DEF = CFLAGS="$(CFLAGS)"

all: core/GNUmakefile account/GNUmakefile char/GNUmakefile zone/GNUmakefile
	cd core ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	@echo "-------- Core Comipled."
	@echo

	cd account ; make $(MAKE_DEF) $@ ; cd ..
	mv Odin-Account$(EXT) ..
	@echo
	@echo "-------- Account Compiled."
	@echo

	cd char ; make $(MAKE_DEF) $@ ; cd ..
	mv Odin-Character$(EXT) ..
	@echo
	@echo "-------- Character Compiled."
	@echo

	cd zone ; make $(MAKE_DEF) $@ ; cd ..
	mv Odin-Zone$(EXT) ..
	@echo 
	@echo "-------- Zone Compiled."
	@echo
	@echo "-------- Server Compilation Done."

a:
	cd core ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	@echo "-------- Core Comipled."
	@echo
	cd account ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	mv Odin-Account$(EXT) ..
	@echo
	@echo "-------- Account Compiled."

c:
	cd core ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	@echo "-------- Core Comipled."
	@echo
	cd char ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	mv Odin-Character$(EXT) ..
	@echo
	@echo "-------- Character Compiled."

z:
	cd core ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	@echo "-------- Core Comipled."
	@echo
	cd zone ; make $(MAKE_DEF) $@ ; cd ..
	@echo
	mv Odin-Zone$(EXT) ..
	@echo
	@echo "-------- Zone Compiled."

clean: 
	cd core ; make $(MAKE_DEF) $@ ; cd ..
	cd account ; make $(MAKE_DEF) $@ ; cd ..
	cd char ; make $(MAKE_DEF) $@ ; cd ..
	cd zone ; make $(MAKE_DEF) $@ ; cd ..
	rm -f ../*$(EXT)
	@echo
	@echo "-------- Src Cleaned."
