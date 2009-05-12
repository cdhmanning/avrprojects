
TARGET_NAME := usarttest
CDHMLIB     := ../cdhmlib
C_FILES     :=  main.c 
C_FILES	    +=  $(CDHMLIB)/twi_avr.c $(CDHMLIB)/timer0.c 
C_FILES     +=  $(CDHMLIB)/myusart.c $(CDHMLIB)/myinterrupts.c
C_FILES     +=  $(CDHMLIB)/myeeprom.c

OBJ_FILES   := $(C_FILES:.c=.o)
C_FLAGS     := -Os -I$(CDHMLIB)
C_FLAGS	    +=  -mmcu=atmega48 -Wall -g
C_FLAGS     +=  -Werror-implicit-function-declaration
C_FLAGS	    +=  -DTWI_SLAVE_ENABLE=1

TARGET_HEX  := $(TARGET_NAME).hex
TARGET_ELF  := $(TARGET_NAME).elf

CC          := avr-gcc
OBJCOPY     := avr-objcopy
SIZE        := avr-size
OBJDUMP	    := avr-objdump

.PHONY: default_action
default_action: load

.PHONY: compile
compile: $(TARGET_HEX)

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex $< $@
	$(SIZE) $<
	$(OBJDUMP) -h $<

$(TARGET_ELF): $(OBJ_FILES)
	$(CC) $^ $(C_FLAGS) -o $@ -Wl,-Map,$@.map 

$(OBJ_FILES): %.o: %.c
	$(CC) -c $(C_FLAGS) -o $@ $<

.PHONY: load
load: $(TARGET_HEX)
	avrdude -p m48 -c USBtiny -Uflash:w:$(TARGET_HEX):i


.PHONY: clean
clean:
	rm $(TARGET_HEX) $(TARGET_ELF) $(OBJ_FILES)