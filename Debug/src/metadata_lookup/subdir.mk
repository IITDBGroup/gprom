################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/metadata_lookup/metadata_lookup.o \
../src/metadata_lookup/metadata_lookup_external.o \
../src/metadata_lookup/metadata_lookup_oracle.o \
../src/metadata_lookup/metadata_lookup_postgres.o 

C_SRCS += \
../src/metadata_lookup/metadata_lookup.c \
../src/metadata_lookup/metadata_lookup_external.c \
../src/metadata_lookup/metadata_lookup_oracle.c \
../src/metadata_lookup/metadata_lookup_postgres.c 

OBJS += \
./src/metadata_lookup/metadata_lookup.o \
./src/metadata_lookup/metadata_lookup_external.o \
./src/metadata_lookup/metadata_lookup_oracle.o \
./src/metadata_lookup/metadata_lookup_postgres.o 

C_DEPS += \
./src/metadata_lookup/metadata_lookup.d \
./src/metadata_lookup/metadata_lookup_external.d \
./src/metadata_lookup/metadata_lookup_oracle.d \
./src/metadata_lookup/metadata_lookup_postgres.d 


# Each subdirectory must supply rules for building sources it contributes
src/metadata_lookup/metadata_lookup.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/metadata_lookup/metadata_lookup.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/metadata_lookup/metadata_lookup_external.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/metadata_lookup/metadata_lookup_external.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/metadata_lookup/metadata_lookup_oracle.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/metadata_lookup/metadata_lookup_oracle.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/metadata_lookup/metadata_lookup_postgres.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/metadata_lookup/metadata_lookup_postgres.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


