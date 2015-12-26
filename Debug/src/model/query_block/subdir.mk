################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/query_block/query_block.o 

C_SRCS += \
../src/model/query_block/query_block.c 

OBJS += \
./src/model/query_block/query_block.o 

C_DEPS += \
./src/model/query_block/query_block.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/query_block/query_block.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/query_block/query_block.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


