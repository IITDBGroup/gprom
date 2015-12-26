################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/node/node.o 

C_SRCS += \
../src/model/node/node.c 

OBJS += \
./src/model/node/node.o 

C_DEPS += \
./src/model/node/node.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/node/node.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/node/node.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


