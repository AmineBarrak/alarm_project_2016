.SUFFIXES : .x .o .c .s

include $(APPLIDIR)/build.mak
SRCS = alarm.c
OBJS = alarm.o
LIBS = alarm.a

all: prebuild
	$(CC) $(WEC_LDFLAGS) $(SRCS) -o $(OBJS)  -static
	@$(CC) $(SRCS) -o $(OBJS)  -static
	@$(AR) lib$(LIBS) $(OBJS)
	@$(COPY) *.o $(OUTPUT_PATH)
	@$(COPY) *.a $(OUTPUT_PATH)
	@$(CLEAN) *.a
	@$(CLEAN) *.o

prebuild:
	@if [ ! -d "$(OUTPUT_PATH)" ]; then mkdir -p $(OUTPUT_PATH); fi

clean:
	$(CLEAN) $(OUTPUT_PATH)

install:
	$(COPY) $(OUTPUT_PATH)/*.a $(APPLIDIR)/$(OUTPUT_PATH)
	$(COPY) $(OUTPUT_PATH)/*.o $(APPLIDIR)/$(OUTPUT_PATH)
