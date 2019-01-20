FS_NAME := mfs
MODULE_FILENAME := $(FS_NAME)
TESTDIR := ../../$(FS_NAME)
TESTFILESIZE := 10
TESTIMAGENAME := mfs-test-image

obj-m += $(MODULE_FILENAME).o
$(MODULE_FILENAME)-objs := init.o shutdown.o module.o fs.o mount.o superblock.o inode.o

all: clean module

module:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) install

test_create_disk_image:
	$(MKDIR) -p $(TESTDIR)/mnt
	dd if=/dev/zero count=$(TESTFILESIZE) bs=1048576 of=$(TESTDIR)/$(TESTIMAGENAME)
	losetup -f $(TESTDIR)/$(TESTIMAGENAME)
	../userspace-tools/./mkfs.$(FS_NAME) -v -d `losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | tail -n1 )`

test_destroy_disk_image:
	-( losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | xargs losetup -d )
	$(RM) $(TESTDIR)/$(TESTIMAGENAME)

test_mount_fs:
	mount -t $(FS_NAME) `losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | tail -n1)` $(TESTDIR)/mnt

test_umount_fs:
	umount $(TESTDIR)/mnt

test_insmod_fs:
	insmod $(MODULE_FILENAME).ko

test_rmmod_fs:
	rmmod $(MODULE_FILENAME)

test:
	$(MAKE)
	-$(MAKE) test_rmmod_fs
	-$(MAKE) test_destroy_disk_image 
	$(MAKE) test_create_disk_image 
	$(MAKE) test_insmod_fs 
	$(MAKE) test_mount_fs 
	$(MAKE) test_umount_fs 
	$(MAKE) test_rmmod_fs
	$(MAKE) test_destroy_disk_image

.PHONY: all module clean install test_destroy_disk_image test_create_disk_image test_insmod_fs test_mount_fs test_umount_fs test_rmmod_fs test_destroy_disk_image