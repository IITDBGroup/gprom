################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/provenance_rewriter/game_provenance/gp_bottom_up_program.o \
../src/provenance_rewriter/game_provenance/gp_main.o \
../src/provenance_rewriter/game_provenance/gp_top_down_program.o 

C_SRCS += \
../src/provenance_rewriter/game_provenance/gp_bottom_up_program.c \
../src/provenance_rewriter/game_provenance/gp_main.c \
../src/provenance_rewriter/game_provenance/gp_top_down_program.c 

OBJS += \
./src/provenance_rewriter/game_provenance/gp_bottom_up_program.o \
./src/provenance_rewriter/game_provenance/gp_main.o \
./src/provenance_rewriter/game_provenance/gp_top_down_program.o 

C_DEPS += \
./src/provenance_rewriter/game_provenance/gp_bottom_up_program.d \
./src/provenance_rewriter/game_provenance/gp_main.d \
./src/provenance_rewriter/game_provenance/gp_top_down_program.d 


# Each subdirectory must supply rules for building sources it contributes
src/provenance_rewriter/game_provenance/gp_bottom_up_program.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/game_provenance/gp_bottom_up_program.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/provenance_rewriter/game_provenance/gp_main.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/game_provenance/gp_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/provenance_rewriter/game_provenance/gp_top_down_program.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/provenance_rewriter/game_provenance/gp_top_down_program.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


