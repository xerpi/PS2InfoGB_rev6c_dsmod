# Remove the line below, if you want to disable silent mode
#.SILENT:

all: build-ee build-iop build-example

clean:
	$(MAKE) -C ee clean
	$(MAKE) -C iop clean
	$(MAKE) -C example clean

build-iop:
	@echo Building IRX
	$(MAKE) -C iop

build-ee:
	@echo Building EE client
	$(MAKE) -C ee

build-example:
	@echo Building EE examples
	$(MAKE) -C example
