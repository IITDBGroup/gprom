################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/utility/sort_helpers.o \
../src/utility/string_utils.o 

C_SRCS += \
../src/utility/sort_helpers.c \
../src/utility/string_utils.c 

OBJS += \
./src/utility/sort_helpers.o \
./src/utility/string_utils.o 

C_DEPS += \
./src/utility/sort_helpers.d \
./src/utility/string_utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/utility/sort_helpers.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/utility/sort_helpers.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/utility/string_utils.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/utility/string_utils.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


