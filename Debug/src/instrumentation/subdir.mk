################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/instrumentation/memory_instrumentation.o \
../src/instrumentation/timing_instrumentation.o 

C_SRCS += \
../src/instrumentation/memory_instrumentation.c \
../src/instrumentation/timing_instrumentation.c 

OBJS += \
./src/instrumentation/memory_instrumentation.o \
./src/instrumentation/timing_instrumentation.o 

C_DEPS += \
./src/instrumentation/memory_instrumentation.d \
./src/instrumentation/timing_instrumentation.d 


# Each subdirectory must supply rules for building sources it contributes
src/instrumentation/memory_instrumentation.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/instrumentation/memory_instrumentation.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/instrumentation/timing_instrumentation.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/instrumentation/timing_instrumentation.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


