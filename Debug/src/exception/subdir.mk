################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/exception/exception.o 

C_SRCS += \
../src/exception/exception.c 

OBJS += \
./src/exception/exception.o 

C_DEPS += \
./src/exception/exception.d 


# Each subdirectory must supply rules for building sources it contributes
src/exception/exception.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/exception/exception.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


