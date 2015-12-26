################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/rewriter.o 

C_SRCS += \
../src/rewriter.c 

OBJS += \
./src/rewriter.o 

C_DEPS += \
./src/rewriter.d 


# Each subdirectory must supply rules for building sources it contributes
src/rewriter.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/rewriter.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


