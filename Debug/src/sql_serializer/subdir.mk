################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/sql_serializer/query_block_to_sql.o \
../src/sql_serializer/sql_serializer.o \
../src/sql_serializer/sql_serializer_oracle.o 

C_SRCS += \
../src/sql_serializer/query_block_to_sql.c \
../src/sql_serializer/sql_serializer.c \
../src/sql_serializer/sql_serializer_oracle.c 

OBJS += \
./src/sql_serializer/query_block_to_sql.o \
./src/sql_serializer/sql_serializer.o \
./src/sql_serializer/sql_serializer_oracle.o 

C_DEPS += \
./src/sql_serializer/query_block_to_sql.d \
./src/sql_serializer/sql_serializer.d \
./src/sql_serializer/sql_serializer_oracle.d 


# Each subdirectory must supply rules for building sources it contributes
src/sql_serializer/query_block_to_sql.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/sql_serializer/query_block_to_sql.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sql_serializer/sql_serializer.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/sql_serializer/sql_serializer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/sql_serializer/sql_serializer_oracle.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/sql_serializer/sql_serializer_oracle.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


