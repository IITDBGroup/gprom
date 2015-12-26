################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/configuration/option.o \
../src/configuration/option_parser.o 

C_SRCS += \
../src/configuration/option.c \
../src/configuration/option_parser.c 

OBJS += \
./src/configuration/option.o \
./src/configuration/option_parser.o 

C_DEPS += \
./src/configuration/option.d \
./src/configuration/option_parser.d 


# Each subdirectory must supply rules for building sources it contributes
src/configuration/option.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/configuration/option.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/configuration/option_parser.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/configuration/option_parser.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


