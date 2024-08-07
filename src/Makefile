#Make File for Gcc Windows

CFLAGS = -g -O2 -Wall -I../Core

#PLATFORM = Unix
#PLATFORM = Linux
#PLATFORM = Windows

PLATFORM = $(shell uname)

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
	EXT = .exe
endif

MAKE_DEF = CFLAGS="$(CFLAGS)"

all: Core/GNUmakefile Account/GNUmakefile Char/GNUmakefile Zone/GNUmakefile
	make -C Core $(MAKE_DEF) $@
	@echo
	@echo "-------- Core Comipled."
	@echo

	make -C Account $(MAKE_DEF) $@
	mv Odin-Account$(EXT) ..
	@echo
	@echo "-------- Account Compiled."
	@echo

	make -C Char $(MAKE_DEF) $@
	mv Odin-Character$(EXT) ..
	@echo
	@echo "-------- Character Compiled."
	@echo

	make -C Zone $(MAKE_DEF) $@
	mv Odin-Zone$(EXT) ..
	@echo 
	@echo "-------- Zone Compiled."
	@echo
	@echo "-------- Server Compilation Done."

a:
	make -C Core $(MAKE_DEF) $@
	@echo
	@echo "-------- Core Comipled."
	@echo
	make -C Account $(MAKE_DEF) $@
	@echo
	mv Odin-Account$(EXT) ..
	@echo
	@echo "-------- Account Compiled."

c:
	make -C Core $(MAKE_DEF) $@
	@echo
	@echo "-------- Core Comipled."
	@echo
	make -C Char $(MAKE_DEF) $@
	@echo
	mv Odin-Character$(EXT) ..
	@echo
	@echo "-------- Character Compiled."

z:
	make -C Core $(MAKE_DEF) $@
	@echo
	@echo "-------- Core Comipled."
	@echo
	make -C Zone $(MAKE_DEF) $@
	@echo
	mv Odin-Zone$(EXT) ..
	@echo
	@echo "-------- Zone Compiled."

clean: 
	make -C Core $(MAKE_DEF) $@
	make -C Account $(MAKE_DEF) $@
	make -C Char $(MAKE_DEF) $@
	make -C Zone $(MAKE_DEF) $@
	rm -f ../*.exe
	@echo
	@echo "-------- Src Cleaned."
