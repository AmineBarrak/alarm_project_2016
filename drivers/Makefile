include $(APPLIDIR)/build.mak

LIBS = cred_drivers.a

all :prebuild compile install

compile:

	$(MAKE) -C ADC all
	$(MAKE) -C AES all
	$(MAKE) -C AUDIO all
	$(MAKE) -C CAMERA all
	$(MAKE) -C DTMF all
	$(MAKE) -C DTMF_RT all
	$(MAKE) -C ETHERNET all
	$(MAKE) -C EVENTS all
	$(MAKE) -C GPIO all
	$(MAKE) -C H264DECODER all
	$(MAKE) -C H264ENCODER all
	$(MAKE) -C I2C all
	$(MAKE) -C IOEXPANDER all
	$(MAKE) -C VIDEO_HW all
#	$(MAKE) -C KEYPAD all
	$(MAKE) -C OSD all
	$(MAKE) -C PWM all
	$(MAKE) -C TIMER all
	$(MAKE) -C RTC all
	$(MAKE) -C SDCARD all
	$(MAKE) -C SPI all
	$(MAKE) -C TOOLS all
	$(MAKE) -C UART all
	$(MAKE) -C USB all
	$(MAKE) -C VIDEO all
	$(MAKE) -C 3G all
	
clean :
	@$(MAKE) -sC ADC clean
	@$(MAKE) -sC AES clean
	@$(MAKE) -sC AUDIO clean
	@$(MAKE) -sC CAMERA clean
	@$(MAKE) -sC DTMF clean
	@$(MAKE) -sC DTMF_RT clean
	@$(MAKE) -sC ETHERNET clean
	@$(MAKE) -sC EVENTS clean
	@$(MAKE) -sC GPIO clean
	@$(MAKE) -sC H264DECODER clean
	@$(MAKE) -sC H264ENCODER clean
	@$(MAKE) -sC I2C clean
	@$(MAKE) -sC IOEXPANDER clean
	@$(MAKE) -sC VIDEO_HW clean
#	@$(MAKE) -sC KEYPAD clean
	@$(MAKE) -sC OSD clean
	@$(MAKE) -sC PWM clean
	@$(MAKE) -sC TIMER clean
	@$(MAKE) -sC RTC clean
	@$(MAKE) -sC SDCARD clean
	@$(MAKE) -sC SPI clean
	@$(MAKE) -sC TOOLS clean
	@$(MAKE) -sC UART clean
	@$(MAKE) -sC USB clean
	@$(MAKE) -sC VIDEO clean
	@$(MAKE) -sC 3G clean
	@$(CLEAN) $(OUTPUT_PATH)

prebuild:
	@if [ ! -d "$(OUTPUT_PATH)" ]; then mkdir -p $(OUTPUT_PATH); fi

install:
	$(MAKE) -C ADC install
	$(MAKE) -C AUDIO install
	$(MAKE) -C AES install
	$(MAKE) -C CAMERA install
	$(MAKE) -C DTMF install
	$(MAKE) -C DTMF_RT install
	$(MAKE) -C ETHERNET install
	$(MAKE) -C EVENTS install
	$(MAKE) -C GPIO install
	$(MAKE) -C H264DECODER install
	$(MAKE) -C H264ENCODER install
	$(MAKE) -C I2C install
	$(MAKE) -C IOEXPANDER install
	$(MAKE) -C VIDEO_HW install
#	$(MAKE) -C KEYPAD install
	$(MAKE) -C OSD install
	$(MAKE) -C PWM install
	$(MAKE) -C TIMER install
	$(MAKE) -C RTC install
	$(MAKE) -C SDCARD install
	$(MAKE) -C SPI install
	$(MAKE) -C TOOLS install
	$(MAKE) -C UART install
	$(MAKE) -C USB install
	$(MAKE) -C VIDEO install
	$(MAKE) -C 3G install
	@$(CROSS_COMPILE)ar  rcs $(OUTPUT_PATH)/lib$(LIBS) $(DRVLIB)/*.o
	
