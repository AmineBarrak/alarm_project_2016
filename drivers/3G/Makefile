.SUFFIXES : .x .o .c .s

include $(APPLIDIR)/build.mak
SRCS = 3g.c
OBJS = 3g.o
LIBS = 3g.a

all: prebuild build

prebuild:
	@if [ ! -d "$(OUTPUT_PATH)" ]; then mkdir -p $(OUTPUT_PATH); fi

build:
	$(CC) $(WEC_LDFLAGS) $(SRCS) -o $(OBJS)  -static
	@$(CC) $(SRCS) -o $(OBJS)  -static
	@$(AR) lib$(LIBS) $(OBJS)
	@$(COPY) *.o $(OUTPUT_PATH)
	@$(COPY) *.a $(OUTPUT_PATH)
	@$(CLEAN) *.a
	@$(CLEAN) *.o
	
clean:
	$(CLEAN) $(OUTPUT_PATH)

install:
	$(COPY) $(APPLIDIR)/drivers/3G/$(OUTPUT_PATH)/*.a $(APPLIDIR)/drivers/$(OUTPUT_PATH)
	$(COPY) $(APPLIDIR)/drivers/3G/$(OUTPUT_PATH)/*.o $(APPLIDIR)/drivers/$(OUTPUT_PATH)
