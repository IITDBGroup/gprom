################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/model/set/hashmap.o \
../src/model/set/set.o \
../src/model/set/vector.o 

C_SRCS += \
../src/model/set/hashmap.c \
../src/model/set/set.c \
../src/model/set/vector.c 

OBJS += \
./src/model/set/hashmap.o \
./src/model/set/set.o \
./src/model/set/vector.o 

C_DEPS += \
./src/model/set/hashmap.d \
./src/model/set/set.d \
./src/model/set/vector.d 


# Each subdirectory must supply rules for building sources it contributes
src/model/set/hashmap.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/set/hashmap.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/set/set.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/set/set.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/model/set/vector.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/model/set/vector.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


