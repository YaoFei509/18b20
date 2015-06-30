#
#  温度探头
#
#  姚飞
#

#for Fedora use 
#REFIX=sdcc-

PREFIX=

#
CC=$(PREFIX)sdcc
PACKIHX=$(PREFIX)packihx

# for STC11F04E
STC11FLAGS = -DSTC11F04E=1

# for STC15F104E
STC15FLAGS =  --iram-size 128 --no-xinit-opt 

＃for STC15W204S
STC15W204SFLAGS = -DSTC15W204S=1 

all: temp_prob.ihx  temp_prob_11f04e.ihx  temp_prob_15w204s.ihx temp_prob_stc15f.ihx

temp_prob.ihx:	temp_prob.rel ds18b20.rel
	$(CC) $^
	$(PACKIHX) $@ > temp_prob.hex

temp_prob_11f04e.ihx: temp_prob_11.rel ds18b20_1t_11.rel
	$(CC) $(STC11FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_11f04e.hex

temp_prob_15w204s.ihx: temp_prob_15w204.rels ds18b20_1t.rel
	$(CC) $(STC11FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_15w204s.hex

downld: temp_prob.ihx
	stcisp -f $<

temp_prob_stc15f.ihx: temp_prob_stc15f.rel soft_uart.rel ds18b20_1t.rel
	$(CC) $(STC15FLAGS) -o $@ $^
	$(PACKIHX) $@ > temp_prob_stc15f.hex

temp_prob.rel: temp_prob.c
	$(CC) -c $<

temp_prob_11.rel: temp_prob.c
	$(CC) $(STC11FLAGS) -o $@ -c $<

temp_prob_15w204s.rel: temp_prob.c
	$(CC) $(STC15W204SFLAGS) -o $@ -c $<


temp_prob_stc15f.rel: temp_prob_stc15f.c
	$(CC) $(STC15FLAGS) -c $<

soft_uart.rel: soft_uart.c  
	$(CC) $(STC15FLAGS) -c $<

ds18b20.rel: ds18b20.c
	$(CC) -c $<

ds18b20_1t.rel: ds18b20_1t.c
	$(CC) $(STC15FLAGS) -c $<

ds18b20_1t_11.rel: ds18b20_1t.c
	$(CC) -o $@ $(STC11FLAGS) -c $<

clean:
	-rm -f *.ihx *.hex *.asm *.lnk *.lst *.map *.mem *.rel *.o  *.sym *.rst *~ *.lk *.bin

