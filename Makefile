export DRAM_LIB=$(CURDIR)/DRAMSim2
#export PRAM_LIB=$(CURDIR)/PRAMSim2
#export NV_LIB=$(CURDIR)/NVDIMMSim/src

all:
	$(MAKE) libdramsim.so -C $(DRAM_LIB) 
#	$(MAKE) libpramsim.so -C $(PRAM_LIB)
#	$(MAKE) lib -C $(NV_LIB)
#	$(MAKE) lib -C $(HybridSimDir)
	$(MAKE) -C marss.dramsim
clean:
	$(MAKE) clean -C $(DRAM_LIB)
#	$(MAKE) clean -C $(PRAM_LIB)
#	$(MAKE) clean -C $(NV_LIB)
#	$(MAKE) clean -C $(HybridSimDir)
	$(MAKE) clean -C marss.dramsim
hybrid: 
	$(MAKE) libdramsim.so -C $(DRAM_LIB) 
#	$(MAKE) libpramsim.so -C $(PRAM_LIB)
#	$(MAKE) lib -C $(NV_LIB)
	$(MAKE) -C $(HybridSimDir)
