
CC=occ
ODIR=o
_OBJ=main.a gteHelp.a tileData.a screen.a play.a display.a scoredisp.a scores.a save.a title.a
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS= 

all:  $(ODIR)/._soko.r sokoban

sokoban: $(OBJ)
	$(CC) -o $@ $(OBJ)
	iix chtyp -t S16 -a 0xDB03 $@

$(ODIR)/main.a: main.c gte.h errors.h globals.h externs.h scores.h display.h

$(ODIR)/tileData.a: tileData.c

$(ODIR)/screen.a: screen.c globals.h

$(ODIR)/play.a: play.c  globals.h

$(ODIR)/display.a: display.c display.h help.h globals.h externs.h scores.h

$(ODIR)/scoredisp.a: scoredisp.c display.h globals.h externs.h scores.h

$(ODIR)/scores.a: scores.c scores.h globals.h externs.h 

$(ODIR)/save.a: save.c globals.h externs.h 


$(ODIR)/%.a: %.c $(DEPS)
	@mkdir -p o
	$(CC) -w -1 -c -O -1 -o $@ $< 

$(ODIR)/._soko.r:  sokoRez.rez sokoRez.equ 
	@mkdir -p o
	occ -o $(ODIR)/soko.r sokoRez.rez
	cp $@ ._sokoban

clean:
	@rm -f $(ODIR)/*.a $(ODIR)/*.root sokoban $(ODIR)/soko.r $(ODIR)/._soko.r ._sokoban

