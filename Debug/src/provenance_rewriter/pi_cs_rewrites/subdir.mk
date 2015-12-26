################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.o \
../src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.o 

C_SRCS += \
../src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.c \
../src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.c 

OBJS += \
./src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.o \
./src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.o 

C_DEPS += \
./src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.d \
./src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.d 


# Each subdirectory must supply rules for building sources it contributes
src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/pi_cs_rewrites/pi_cs_composable.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/pi_cs_rewrites/pi_cs_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


