################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Steel/src/Camera.cpp \
../Steel/src/Engine.cpp \
../Steel/src/Inanimate.cpp \
../Steel/src/InputManager.cpp \
../Steel/src/Level.cpp 

OBJS += \
./Steel/src/Camera.o \
./Steel/src/Engine.o \
./Steel/src/Inanimate.o \
./Steel/src/InputManager.o \
./Steel/src/Level.o 

CPP_DEPS += \
./Steel/src/Camera.d \
./Steel/src/Engine.d \
./Steel/src/Inanimate.d \
./Steel/src/InputManager.d \
./Steel/src/Level.d 


# Each subdirectory must supply rules for building sources it contributes
Steel/src/%.o: ../Steel/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUG=1 -I"/media/z2/cpp/1105/Steel/Steel/include" -I"/media/z2/src/ogre_src_v1-7-2//OgreMain/include" -I"/media/z2/src/ogre_src_v1-7-2//include" -I"/media/z2/src/ogre_src_v1-7-2//Samples/Common/include" -I/usr/include/OIS -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


