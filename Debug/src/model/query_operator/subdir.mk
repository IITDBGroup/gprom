################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/query_operator/query_operator.o \
../src/model/query_operator/query_operator_model_checker.o \
../src/model/query_operator/schema_utility.o 

C_SRCS += \
../src/model/query_operator/query_operator.c \
../src/model/query_operator/query_operator_model_checker.c \
../src/model/query_operator/schema_utility.c 

OBJS += \
./src/model/query_operator/query_operator.o \
./src/model/query_operator/query_operator_model_checker.o \
./src/model/query_operator/schema_utility.o 

C_DEPS += \
./src/model/query_operator/query_operator.d \
./src/model/query_operator/query_operator_model_checker.d \
./src/model/query_operator/schema_utility.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/query_operator/query_operator.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/query_operator/query_operator.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/query_operator/query_operator_model_checker.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/query_operator/query_operator_model_checker.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/query_operator/schema_utility.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/query_operator/schema_utility.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


