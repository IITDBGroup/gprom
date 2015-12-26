################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/operator_optimizer/cost_based_optimizer.o \
../src/operator_optimizer/expr_attr_factor.o \
../src/operator_optimizer/operator_merge.o \
../src/operator_optimizer/operator_optimizer.o \
../src/operator_optimizer/optimizer_prop_inference.o 

C_SRCS += \
../src/operator_optimizer/cost_based_optimizer.c \
../src/operator_optimizer/expr_attr_factor.c \
../src/operator_optimizer/operator_merge.c \
../src/operator_optimizer/operator_optimizer.c \
../src/operator_optimizer/optimizer_prop_inference.c 

OBJS += \
./src/operator_optimizer/cost_based_optimizer.o \
./src/operator_optimizer/expr_attr_factor.o \
./src/operator_optimizer/operator_merge.o \
./src/operator_optimizer/operator_optimizer.o \
./src/operator_optimizer/optimizer_prop_inference.o 

C_DEPS += \
./src/operator_optimizer/cost_based_optimizer.d \
./src/operator_optimizer/expr_attr_factor.d \
./src/operator_optimizer/operator_merge.d \
./src/operator_optimizer/operator_optimizer.d \
./src/operator_optimizer/optimizer_prop_inference.d 


# Each subdirectory must supply rules for building sources it contributes
src/operator_optimizer/cost_based_optimizer.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/operator_optimizer/cost_based_optimizer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/operator_optimizer/expr_attr_factor.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/operator_optimizer/expr_attr_factor.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/operator_optimizer/operator_merge.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/operator_optimizer/operator_merge.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/operator_optimizer/operator_optimizer.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/operator_optimizer/operator_optimizer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/operator_optimizer/optimizer_prop_inference.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/operator_optimizer/optimizer_prop_inference.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


