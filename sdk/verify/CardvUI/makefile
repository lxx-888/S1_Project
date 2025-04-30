include ../../../project/configs/current.configs
PROJ_ROOT?=$(PDIR)
export PROJ_ROOT

all:
	$(MAKE) -C app all
	$(MAKE) -C CardvUI_Fly/jni all
install:
	$(MAKE) -C CardvUI_Fly/jni install
clean:
	$(MAKE) -C CardvUI_Fly/jni clean
	$(MAKE) -C app clean