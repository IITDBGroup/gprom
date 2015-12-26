################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/list/list.o 

C_SRCS += \
../src/model/list/list.c 

OBJS += \
./src/model/list/list.o 

C_DEPS += \
./src/model/list/list.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/list/list.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/list/list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


