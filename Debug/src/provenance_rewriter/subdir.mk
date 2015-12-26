################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/provenance_rewriter/prov_rewriter_main.o \
../src/provenance_rewriter/prov_schema.o \
../src/provenance_rewriter/prov_utility.o 

C_SRCS += \
../src/provenance_rewriter/prov_rewriter_main.c \
../src/provenance_rewriter/prov_schema.c \
../src/provenance_rewriter/prov_utility.c 

OBJS += \
./src/provenance_rewriter/prov_rewriter_main.o \
./src/provenance_rewriter/prov_schema.o \
./src/provenance_rewriter/prov_utility.o 

C_DEPS += \
./src/provenance_rewriter/prov_rewriter_main.d \
./src/provenance_rewriter/prov_schema.d \
./src/provenance_rewriter/prov_utility.d 


# Each subdirectory must supply rules for building sources it contributes
src/provenance_rewriter/prov_rewriter_main.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/prov_rewriter_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/provenance_rewriter/prov_schema.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/prov_schema.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/provenance_rewriter/prov_utility.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/prov_utility.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


