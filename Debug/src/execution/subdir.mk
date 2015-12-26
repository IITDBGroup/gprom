################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/execution/exe_output_dl.o \
../src/execution/exe_output_gp.o \
../src/execution/exe_output_sql.o \
../src/execution/exe_run_query.o \
../src/execution/executor.o 

C_SRCS += \
../src/execution/exe_output_dl.c \
../src/execution/exe_output_gp.c \
../src/execution/exe_output_sql.c \
../src/execution/exe_run_query.c \
../src/execution/executor.c 

OBJS += \
./src/execution/exe_output_dl.o \
./src/execution/exe_output_gp.o \
./src/execution/exe_output_sql.o \
./src/execution/exe_run_query.o \
./src/execution/executor.o 

C_DEPS += \
./src/execution/exe_output_dl.d \
./src/execution/exe_output_gp.d \
./src/execution/exe_output_sql.d \
./src/execution/exe_run_query.d \
./src/execution/executor.d 


# Each subdirectory must supply rules for building sources it contributes
src/execution/exe_output_dl.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/execution/exe_output_dl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/execution/exe_output_gp.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/execution/exe_output_gp.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/execution/exe_output_sql.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/execution/exe_output_sql.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/execution/exe_run_query.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/execution/exe_run_query.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/execution/executor.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/execution/executor.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


