################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/expression/expr_to_sql.o \
../src/model/expression/expression.o 

C_SRCS += \
../src/model/expression/expr_to_sql.c \
../src/model/expression/expression.c 

OBJS += \
./src/model/expression/expr_to_sql.o \
./src/model/expression/expression.o 

C_DEPS += \
./src/model/expression/expr_to_sql.d \
./src/model/expression/expression.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/expression/expr_to_sql.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/expression/expr_to_sql.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/expression/expression.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/expression/expression.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


