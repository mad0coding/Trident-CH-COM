################################################################################
# MRS Version: 1.9.1
# 自动生成的文件。不要编辑！
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SRC/StdPeriphDriver/CH59x_adc.c \
../SRC/StdPeriphDriver/CH59x_clk.c \
../SRC/StdPeriphDriver/CH59x_flash.c \
../SRC/StdPeriphDriver/CH59x_gpio.c \
../SRC/StdPeriphDriver/CH59x_i2c.c \
../SRC/StdPeriphDriver/CH59x_lcd.c \
../SRC/StdPeriphDriver/CH59x_pwr.c \
../SRC/StdPeriphDriver/CH59x_sys.c \
../SRC/StdPeriphDriver/CH59x_uart1.c \
../SRC/StdPeriphDriver/CH59x_usbdev.c 

OBJS += \
./StdPeriphDriver/CH59x_adc.o \
./StdPeriphDriver/CH59x_clk.o \
./StdPeriphDriver/CH59x_flash.o \
./StdPeriphDriver/CH59x_gpio.o \
./StdPeriphDriver/CH59x_i2c.o \
./StdPeriphDriver/CH59x_lcd.o \
./StdPeriphDriver/CH59x_pwr.o \
./StdPeriphDriver/CH59x_sys.o \
./StdPeriphDriver/CH59x_uart1.o \
./StdPeriphDriver/CH59x_usbdev.o 

C_DEPS += \
./StdPeriphDriver/CH59x_adc.d \
./StdPeriphDriver/CH59x_clk.d \
./StdPeriphDriver/CH59x_flash.d \
./StdPeriphDriver/CH59x_gpio.d \
./StdPeriphDriver/CH59x_i2c.d \
./StdPeriphDriver/CH59x_lcd.d \
./StdPeriphDriver/CH59x_pwr.d \
./StdPeriphDriver/CH59x_sys.d \
./StdPeriphDriver/CH59x_uart1.d \
./StdPeriphDriver/CH59x_usbdev.d 


# Each subdirectory must supply rules for building sources it contributes
StdPeriphDriver/CH59x_adc.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_adc.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_clk.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_clk.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_flash.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_flash.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_gpio.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_gpio.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_i2c.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_i2c.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_lcd.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_lcd.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_pwr.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_pwr.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_sys.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_sys.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_uart1.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_uart1.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
StdPeriphDriver/CH59x_usbdev.o: F:/MyProjects/DevelopmentBoards/Trident-CH/Codes/Trident-CH-COM/SRC/StdPeriphDriver/CH59x_usbdev.c
	@	@	riscv-none-embed-gcc -march=rv32imac -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common  -g -DDEBUG=1 -DDCDC_ENABLE=FALSE -DBLE_TX_POWER=LL_TX_POWEER_4_DBM -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\Startup" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\APP\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Profile\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\StdPeriphDriver\inc" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\HAL\include" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\Ld" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\LIB" -I"F:\MyProjects\DevelopmentBoards\Trident-CH\Codes\Trident-CH-COM\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

