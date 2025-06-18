################################################################################
# MRS Version: 1.9.1
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/RingMem.c \
../APP/app_usb.c \
../APP/peripheral.c \
../APP/peripheral_main.c 

OBJS += \
./APP/RingMem.o \
./APP/app_usb.o \
./APP/peripheral.o \
./APP/peripheral_main.o 

C_DEPS += \
./APP/RingMem.d \
./APP/app_usb.d \
./APP/peripheral.d \
./APP/peripheral_main.d 


# Each subdirectory must supply rules for building sources it contributes
APP/%.o: ../APP/%.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

