FS_NAME := mfs
MODULE_FILENAME := $(FS_NAME)
TESTDIR := ../../$(FS_NAME)
TESTFILESIZE := 10

obj-m += $(MODULE_FILENAME).o
$(MODULE_FILENAME)-objs := init.o shutdown.o module.o fs.o mount.o superblock.o

all: clean module

module:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) install

test_prepare_disk:
	$(MKDIR) -p $(TESTDIR)/mnt
	dd if=/dev/zero count=$(TESTFILESIZE) bs=1048576 of=$(TESTDIR)/image

test_cleanup_disk:
	#losetup -d $(TESTDIR)/mnt

test_mount:
	mount -o loop -t $(FS_NAME) $(TESTDIR)/image $(TESTDIR)/mnt
	umount $(TESTDIR)/mnt

test:
	$(MAKE)
	$(MAKE) test_prepare_disk
	insmod $(MODULE_FILENAME).ko
	-$(MAKE) test_mount
	rmmod $(MODULE_FILENAME)
	$(MAKE) test_cleanup_disk

.PHONY: all module clean install test_prepare_disk test test_cleanup_disk