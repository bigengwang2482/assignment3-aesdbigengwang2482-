#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u
BUILD_PATH=$(pwd -P)
OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
	make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper # deep clean
	make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # do the configure
	make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all # build the kernel image for booting with QEMU
	# make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules # build any kernel modules, skip according to instruction
	make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs # build the device tree
	# DONE!
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
# DONE!
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox	
	make distclean
	make defconfig
	# DONE!
else
    cd busybox
fi

# TODO: Make and install busybox
# Now we are in the busybox folder
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
d ${OUTDIR}/rootfs # go to rootfs to check the dependencies use the installed busybox
# DONE!
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
sysroot_path=$(${CROSS_COMPILE}gcc -print-sysroot)
echo ${sysroot_path}
cp ${sysroot_path}/lib/ld-linux-aarch64.so.1 lib64/
cp ${sysroot_path}/lib64/libm.so.6 lib64/
cp ${sysroot_path}/lib64/libresolv.so.2 lib64/
cp ${sysroot_path}/lib64/libc.so.6 lib64/
# DONE!
# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1
# TODO: Clean and build the writer utility
# Use the Makefile in this folder for writer utility
cd ${BUILD_PATH} # get back to the compiling folder
pwd -P
make clean # clean
make all CROSS_COMPILE=${CROSS_COMPILE} # use CROSS_COMPILE variable and make target all
# DONE!
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp finder.sh ${OUTDIR}/rootfs/home/
cp -r ../conf ${OUTDIR}/rootfs/home/
cp finder-test.sh ${OUTDIR}/rootfs/home/
cp autorun-qemu.sh ${OUTDIR}/rootfs/home/

# TODO: Chown the root directory
sudo chown -R root:root ${OUTDIR}/rootfs
# TODO: Create initramfs.cpio.gz
cd "${OUTDIR}/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
