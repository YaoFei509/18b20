
CC=sdcc-sdcc -mmcs51

all: temp_prob.ihx

temp_prob.ihx:	temp_prob.c
	$(CC) $<

downld: temp_prob.ihx
	stcisp -f $<

clean:
	-rm -f temp_prob.ihx *.asm *.lnk *.lst *.map *.mem *.rel *.sym *.rst *~ *.lk *.bin

