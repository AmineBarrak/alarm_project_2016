include $(APPLIDIR)/build.mak

LIBS = cred_mw.a

all :prebuild compile install

compile:

	$(MAKE) -C NOTIFICATION all
	$(MAKE) -C ALARM all
	$(MAKE) -C CONTACTID all
	$(MAKE) -C POWER all
	$(MAKE) -C NETWORK all
	$(MAKE) -C TELEPHONY all
	$(MAKE) -C MAIN_ALARM all
	$(MAKE) -C MULTIMEDIA all
	$(MAKE) -C IPSTREAMING all
	$(MAKE) -C RF_CROW all
	$(MAKE) -C VOIP all
	$(MAKE) -C EMAIL all
	$(MAKE) -C REMOTECOM all
	$(MAKE) -C SIAIP all
	$(MAKE) -C VOICE_GUIDE all
	$(MAKE) -C PUSH_NOTIF all
	
clean :
	@$(MAKE) -sC NOTIFICATION clean
	@$(MAKE) -sC ALARM clean
	@$(MAKE) -sC CONTACTID clean
	@$(MAKE) -sC POWER clean
	@$(MAKE) -sC NETWORK clean
	@$(MAKE) -sC TELEPHONY clean
	@$(MAKE) -sC MAIN_ALARM clean
	@$(MAKE) -sC MULTIMEDIA clean
	@$(MAKE) -sC IPSTREAMING clean
	@$(MAKE) -sC RF_CROW clean
	@$(MAKE) -sC VOIP clean
	@$(MAKE) -sC EMAIL clean
	@$(MAKE) -sC REMOTECOM clean
	@$(MAKE) -sC SIAIP clean
	@$(MAKE) -sC VOICE_GUIDE clean
	@$(MAKE) -sC PUSH_NOTIF clean
	@$(CLEAN) $(OUTPUT_PATH)

prebuild:
	@if [ ! -d "$(OUTPUT_PATH)" ]; then mkdir -p $(OUTPUT_PATH); fi

install:
	@$(COPY) NOTIFICATION/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) ALARM/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) CONTACTID/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) POWER/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) NETWORK/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) TELEPHONY/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) MAIN_ALARM/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) MULTIMEDIA/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) IPSTREAMING/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) RF_CROW/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) VOIP/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) EMAIL/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) REMOTECOM/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) SIAIP/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) VOICE_GUIDE/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(COPY) PUSH_NOTIF/$(OUTPUT_PATH)/*.o $(OUTPUT_PATH)
	@$(CROSS_COMPILE)ar  rcs $(OUTPUT_PATH)/lib$(LIBS) $(MWLIB)/*.o
