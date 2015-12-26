################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/datalog/datalog_model.o \
../src/model/datalog/datalog_model_checker.o 

C_SRCS += \
../src/model/datalog/datalog_model.c \
../src/model/datalog/datalog_model_checker.c 

OBJS += \
./src/model/datalog/datalog_model.o \
./src/model/datalog/datalog_model_checker.o 

C_DEPS += \
./src/model/datalog/datalog_model.d \
./src/model/datalog/datalog_model_checker.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/datalog/datalog_model.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/datalog/datalog_model.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/datalog/datalog_model_checker.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/datalog/datalog_model_checker.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


