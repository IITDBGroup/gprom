################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../test/test_analysis.o \
../test/test_copy.o \
../test/test_dl.o \
../test/test_equal.o \
../test/test_exception.o \
../test/test_expr.o \
../test/test_hash.o \
../test/test_hashmap.o \
../test/test_libgprom.o \
../test/test_list.o \
../test/test_logger.o \
../test/test_main.o \
../test/test_mem_mgr.o \
../test/test_metadata_lookup.o \
../test/test_metadata_postgres.o \
../test/test_parameter.o \
../test/test_parse.o \
../test/test_parser.o \
../test/test_pi_cs_rewrite.o \
../test/test_rewriter.o \
../test/test_serializer.o \
../test/test_set.o \
../test/test_string.o \
../test/test_string_utils.o \
../test/test_to_string.o \
../test/test_translate.o \
../test/test_update_analysis.o \
../test/test_vector.o \
../test/test_visit.o 

C_SRCS += \
../test/test_analysis.c \
../test/test_copy.c \
../test/test_dl.c \
../test/test_equal.c \
../test/test_exception.c \
../test/test_expr.c \
../test/test_hash.c \
../test/test_hashmap.c \
../test/test_libgprom.c \
../test/test_list.c \
../test/test_logger.c \
../test/test_main.c \
../test/test_mem_mgr.c \
../test/test_metadata_lookup.c \
../test/test_metadata_postgres.c \
../test/test_parameter.c \
../test/test_parse.c \
../test/test_parser.c \
../test/test_pi_cs_rewrite.c \
../test/test_rewriter.c \
../test/test_serializer.c \
../test/test_set.c \
../test/test_string.c \
../test/test_string_utils.c \
../test/test_to_string.c \
../test/test_translate.c \
../test/test_update_analysis.c \
../test/test_vector.c \
../test/test_visit.c 

OBJS += \
./test/test_analysis.o \
./test/test_copy.o \
./test/test_dl.o \
./test/test_equal.o \
./test/test_exception.o \
./test/test_expr.o \
./test/test_hash.o \
./test/test_hashmap.o \
./test/test_libgprom.o \
./test/test_list.o \
./test/test_logger.o \
./test/test_main.o \
./test/test_mem_mgr.o \
./test/test_metadata_lookup.o \
./test/test_metadata_postgres.o \
./test/test_parameter.o \
./test/test_parse.o \
./test/test_parser.o \
./test/test_pi_cs_rewrite.o \
./test/test_rewriter.o \
./test/test_serializer.o \
./test/test_set.o \
./test/test_string.o \
./test/test_string_utils.o \
./test/test_to_string.o \
./test/test_translate.o \
./test/test_update_analysis.o \
./test/test_vector.o \
./test/test_visit.o 

C_DEPS += \
./test/test_analysis.d \
./test/test_copy.d \
./test/test_dl.d \
./test/test_equal.d \
./test/test_exception.d \
./test/test_expr.d \
./test/test_hash.d \
./test/test_hashmap.d \
./test/test_libgprom.d \
./test/test_list.d \
./test/test_logger.d \
./test/test_main.d \
./test/test_mem_mgr.d \
./test/test_metadata_lookup.d \
./test/test_metadata_postgres.d \
./test/test_parameter.d \
./test/test_parse.d \
./test/test_parser.d \
./test/test_pi_cs_rewrite.d \
./test/test_rewriter.d \
./test/test_serializer.d \
./test/test_set.d \
./test/test_string.d \
./test/test_string_utils.d \
./test/test_to_string.d \
./test/test_translate.d \
./test/test_update_analysis.d \
./test/test_vector.d \
./test/test_visit.d 


# Each subdirectory must supply rules for building sources it contributes
test/test_analysis.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_analysis.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_copy.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_copy.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_dl.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_dl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_equal.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_equal.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_exception.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_exception.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_expr.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_expr.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_hash.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_hash.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_hashmap.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_hashmap.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_libgprom.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_libgprom.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_list.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_logger.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_logger.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_main.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_main.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_mem_mgr.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_mem_mgr.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_metadata_lookup.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_metadata_lookup.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_metadata_postgres.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_metadata_postgres.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_parameter.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_parameter.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_parse.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_parse.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_parser.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_parser.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_pi_cs_rewrite.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_pi_cs_rewrite.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_rewriter.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_rewriter.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_serializer.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_serializer.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_set.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_set.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_string.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_string.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_string_utils.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_string_utils.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_to_string.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_to_string.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_translate.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_translate.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_update_analysis.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_update_analysis.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_vector.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_vector.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

test/test_visit.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/test/test_visit.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


