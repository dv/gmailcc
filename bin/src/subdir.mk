################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Client.cpp \
../src/Log.cpp \
../src/LogSink.cpp \
../src/MailBox.cpp \
../src/MailDatabase.cpp \
../src/MailLink.cpp \
../src/MailRecord.cpp \
../src/Options.cpp \
../src/main.cpp 

OBJS += \
./src/Client.o \
./src/Log.o \
./src/LogSink.o \
./src/MailBox.o \
./src/MailDatabase.o \
./src/MailLink.o \
./src/MailRecord.o \
./src/Options.o \
./src/main.o 

CPP_DEPS += \
./src/Client.d \
./src/Log.d \
./src/LogSink.d \
./src/MailBox.d \
./src/MailDatabase.d \
./src/MailLink.d \
./src/MailRecord.d \
./src/Options.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0  -fno-operator-names -Wno-write-strings -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


