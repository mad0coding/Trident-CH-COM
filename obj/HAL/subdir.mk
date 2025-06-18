################################################################################
# MRS Version: 1.9.1
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../HAL/MCU.c \
../HAL/RTC.c \
../HAL/SLEEP.c 

OBJS += \
./HAL/MCU.o \
./HAL/RTC.o \
./HAL/SLEEP.o 

C_DEPS += \
./HAL/MCU.d \
./HAL/RTC.d \
./HAL/SLEEP.d 


# Each subdirectory must supply rules for building sources it contributes
HAL/MCU.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/HAL/MCU.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/RTC.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/HAL/RTC.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/SLEEP.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/HAL/SLEEP.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

