FS_NAME := mfs
MODULE_FILENAME := $(FS_NAME)
TESTDIR := ./test
TESTFILESIZE := 10
TESTIMAGENAME := mfs-test-image
TESTIMAGESIZE := 1048576
MFSUSERBIN := ../mfs-userspace-tools

obj-m += $(MODULE_FILENAME).o
$(MODULE_FILENAME)-objs := init.o shutdown.o module.o bitmap.o fs.o mount.o superblock.o inode.o freemap.o dir.o file.o utils.o

all: clean module

module:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) install

test_create_disk_image:
	mkdir -p $(TESTDIR)/mnt
	dd if=/dev/zero count=$(TESTFILESIZE) bs=$(TESTIMAGESIZE) of=$(TESTDIR)/$(TESTIMAGENAME)
	losetup -f $(TESTDIR)/$(TESTIMAGENAME)
	$(MFSUSERBIN)/mkfs.$(FS_NAME) -v -d `losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | tail -n1 )`

test_destroy_disk_image:
	-( $(MFSUSERBIN)/fsck.mfs -v -d `losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | tail -n1 )` )
	-( losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | xargs losetup -d )
	$(RM) $(TESTDIR)/$(TESTIMAGENAME)

test_mkdir:
	mkdir -p $(TESTDIR)/mnt/dir $(TESTDIR)/mnt/dir2

test_touch:
	touch $(TESTDIR)/mnt/dir/file
	touch $(TESTDIR)/mnt/file

test_write:
	echo "blablabla" > $(TESTDIR)/mnt/file

test_read:
	cat $(TESTDIR)/mnt/file

test_ls:
	ls -lah $(TESTDIR)/mnt

test_mount_fs:
	mount -t $(FS_NAME) `losetup | grep $(TESTIMAGENAME) | awk '{print $$1}' | tail -n1)` $(TESTDIR)/mnt

test_umount_fs:
	umount $(TESTDIR)/mnt

test_insmod_fs:
	insmod $(MODULE_FILENAME).ko

test_rmmod_fs:
	rmmod $(MODULE_FILENAME)

test_cleanup_force:
	-$(MAKE) test_umount_fs 
	-$(MAKE) test_rmmod_fs
	-$(MAKE) test_destroy_disk_image

test_cleanup:
	-$(MAKE) test_umount_fs 
	-$(MAKE) test_rmmod_fs
	-$(MAKE) test_destroy_disk_image

test:
	$(MAKE)
	$(MAKE) test_cleanup_force
	$(MAKE) test_create_disk_image 
	$(MAKE) test_insmod_fs 
	$(MAKE) test_mount_fs 
	$(MAKE) test_mkdir
	$(MAKE) test_touch
	$(MAKE) test_ls
	$(MAKE) test_write
	$(MAKE) test_ls
	$(MAKE) test_read
	$(MAKE) test_umount_fs
	$(MAKE) test_cleanup
	@echo kernel log output is
	@(dmesg | tail -n20)

.PHONY: all module clean install test test_destroy_disk_image test_create_disk_image test_insmod_fs test_mount_fs test_umount_fs test_rmmod_fs test_destroy_disk_image test_cleanup_force test_cleanup
