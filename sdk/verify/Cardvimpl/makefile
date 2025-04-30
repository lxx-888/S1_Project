include ../../../project/configs/current.configs
PROJ_ROOT?=$(PDIR)
export PROJ_ROOT

all:
	$(MAKE) -C live555/live555Source
	$(MAKE) -C net/cgi_1.0.0 all
	$(MAKE) -C DCF all
	$(MAKE) -C cardv_IPC all
	$(MAKE) -C cardv all
	$(MAKE) -C net/goahead-4.1.0
install:
	$(MAKE) -C net/cgi_1.0.0 install
	$(MAKE) -C DCF install
	$(MAKE) -C cardv_IPC install
	$(MAKE) -C cardv install
	$(MAKE) -C net/goahead-4.1.0 install
clean:
	$(MAKE) -C net/cgi_1.0.0 clean
	$(MAKE) -C DCF clean
	$(MAKE) -C cardv_IPC clean
	$(MAKE) -C cardv clean
	$(MAKE) -C net/goahead-4.1.0 clobber
	$(MAKE) -C live555/live555Source clean