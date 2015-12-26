################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../src/parser/dl_parser.lex.o \
../src/parser/dl_parser.tab.o \
../src/parser/jp_parser.lex.o \
../src/parser/jp_parser.tab.o \
../src/parser/parser.o \
../src/parser/parser_dl.o \
../src/parser/parser_hive.o \
../src/parser/parser_jp.o \
../src/parser/parser_oracle.o \
../src/parser/parser_postgres.o \
../src/parser/postgres_parser.lex.o \
../src/parser/postgres_parser.tab.o \
../src/parser/sql_parser.lex.o \
../src/parser/sql_parser.tab.o 

C_SRCS += \
../src/parser/dl_parser.lex.c \
../src/parser/dl_parser.tab.c \
../src/parser/jp_parser.lex.c \
../src/parser/jp_parser.tab.c \
../src/parser/parser.c \
../src/parser/parser_dl.c \
../src/parser/parser_hive.c \
../src/parser/parser_jp.c \
../src/parser/parser_oracle.c \
../src/parser/parser_postgres.c \
../src/parser/postgres_parser.lex.c \
../src/parser/postgres_parser.tab.c \
../src/parser/sql_parser.c \
../src/parser/sql_parser.lex.c \
../src/parser/sql_parser.tab.c 

OBJS += \
./src/parser/dl_parser.lex.o \
./src/parser/dl_parser.tab.o \
./src/parser/jp_parser.lex.o \
./src/parser/jp_parser.tab.o \
./src/parser/parser.o \
./src/parser/parser_dl.o \
./src/parser/parser_hive.o \
./src/parser/parser_jp.o \
./src/parser/parser_oracle.o \
./src/parser/parser_postgres.o \
./src/parser/postgres_parser.lex.o \
./src/parser/postgres_parser.tab.o \
./src/parser/sql_parser.o \
./src/parser/sql_parser.lex.o \
./src/parser/sql_parser.tab.o 

C_DEPS += \
./src/parser/dl_parser.lex.d \
./src/parser/dl_parser.tab.d \
./src/parser/jp_parser.lex.d \
./src/parser/jp_parser.tab.d \
./src/parser/parser.d \
./src/parser/parser_dl.d \
./src/parser/parser_hive.d \
./src/parser/parser_jp.d \
./src/parser/parser_oracle.d \
./src/parser/parser_postgres.d \
./src/parser/postgres_parser.lex.d \
./src/parser/postgres_parser.tab.d \
./src/parser/sql_parser.d \
./src/parser/sql_parser.lex.d \
./src/parser/sql_parser.tab.d 


# Each subdirectory must supply rules for building sources it contributes
src/parser/dl_parser.lex.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/dl_parser.lex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/dl_parser.tab.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/dl_parser.tab.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/jp_parser.lex.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/jp_parser.lex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/jp_parser.tab.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/jp_parser.tab.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser_dl.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser_dl.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser_hive.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser_hive.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser_jp.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser_jp.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser_oracle.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser_oracle.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/parser_postgres.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/parser_postgres.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/postgres_parser.lex.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/postgres_parser.lex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/postgres_parser.tab.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/postgres_parser.tab.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/sql_parser.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/sql_parser.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/sql_parser.lex.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/sql_parser.lex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/parser/sql_parser.tab.o: /Users/xun/Documents/database_project/p1/provenance-rewriter-prototype/src/parser/sql_parser.tab.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


