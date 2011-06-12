################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Steel.cpp \
../src/main.cpp 

OBJS += \
./src/Steel.o \
./src/main.o 

CPP_DEPS += \
./src/Steel.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUG=1 -I"/media/z2/cpp/1105/Steel/Steel/include" -I"/media/z2/src/ogre_src_v1-7-2//OgreMain/include" -I"/media/z2/src/ogre_src_v1-7-2//include" -I"/media/z2/src/ogre_src_v1-7-2//Samples/Common/include" -I/usr/include/OIS -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


