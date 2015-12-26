################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/libgprom/libgprom.o 

C_SRCS += \
../src/libgprom/libgprom.c 

OBJS += \
./src/libgprom/libgprom.o 

C_DEPS += \
./src/libgprom/libgprom.d 


# Each subdirectory must supply rules for building sources it contributes
src/libgprom/libgprom.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/libgprom/libgprom.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


