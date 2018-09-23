.SUFFIXES : .x .o .c .s

include $(APPLIDIR)/build.mak
SRCS = app_push.c
TARGET = app_push_unitary_tests
LIBS = -lcred_mw -lcred_drivers -lm -lcurl -lssl -lcrypto -lssl -lcrypto -lz -ldl -lcred_drivers -lnaacdec -lnaacenc -lm -lcred_mw -lcred_drivers -lnaacdec -lnaacenc -lm  -lstdc++ -lrt

all: prebuild compile install 

compile:

	$(LINK) $(WEC_LDFLAGS)  $(SRCS)  -o  $(TARGET) $(LIBS) -static -pthread -Wl,--gc-sections
	$(STRIP) $(TARGET)
	

prebuild:
	@if [ ! -d "$(OUTPUT_PATH)" ]; then mkdir -p $(OUTPUT_PATH); fi

clean:
	$(CLEAN) $(OUTPUT_PATH)

install:
	$(COPY) $(TARGET) $(APPLIDIR)/tests/$(OUTPUT_PATH)

