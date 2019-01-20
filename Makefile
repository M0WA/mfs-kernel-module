FS_NAME := mfs
MODULE_FILENAME := $(FS_NAME)

obj-m += $(MODULE_FILENAME).o
$(MODULE_FILENAME)-objs := init.o shutdown.o module.o fs.o mount.o superblock.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test_prepare_disk:
	@mkdir -p ../$(MODULE_FILENAME)/mnt
	echo -n "" > ../$(MODULE_FILENAME)/image    

test_cleanup_disk:
	losetup -d ../$(FS_NAME)-mnt

test:
	$(MAKE)
	$(MAKE) test_prepare_disk
	insmod $(MODULE_FILENAME).ko
	mount -o loop -t $(FS_NAME) ../$(MODULE_FILENAME)/image ../$(MODULE_FILENAME)/mnt
	umount ../$(FS_NAME)-mnt
	rmmod $(MODULE_FILENAME)

.PHONY: all clean test_prepare_disk test