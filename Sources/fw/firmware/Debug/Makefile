################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../firmware.c \
../function.c


PREPROCESSING_SRCS +=  \
../func.s


ASM_SRCS += 


OBJS +=  \
firmware.o \
func.o \
function.o

OBJS_AS_ARGS +=  \
firmware.o \
func.o \
function.o

C_DEPS +=  \
firmware.d \
func.d \
function.d

C_DEPS_AS_ARGS +=  \
firmware.d \
func.d \
function.d

OUTPUT_FILE_PATH +=firmware.elf

OUTPUT_FILE_PATH_AS_ARGS +=firmware.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

# AVR32/GNU C Compiler





./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 20100110.0
	$(QUOTE)D:\WinAVR\bin\avr-gcc.exe$(QUOTE)  -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atmega48p -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<"
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler
./func.o: .././func.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 20100110.0
	$(QUOTE)D:\WinAVR\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega48p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<"
	@echo Finished building: $<
	


./%.o: .././%.s
	@echo Building file: $<
	@echo Invoking: AVR32/GNU Assembler : 20100110.0
	$(QUOTE)D:\WinAVR\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atmega48p -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<"
	@echo Finished building: $<
	




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 20100110.0
	$(QUOTE)D:\WinAVR\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="firmware.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atmega48p 
	@echo Finished building target: $@
	"D:\WinAVR\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "firmware.elf" "firmware.hex"
	"D:\WinAVR\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "firmware.elf" "firmware.eep" || exit 0
	"D:\WinAVR\bin\avr-objdump.exe" -h -S "firmware.elf" > "firmware.lss"
	"D:\WinAVR\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "firmware.elf" "firmware.srec"
	"D:\WinAVR\bin\avr-size.exe" "firmware.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "firmware.elf" "firmware.a" "firmware.hex" "firmware.lss" "firmware.eep" "firmware.map" "firmware.srec"
	