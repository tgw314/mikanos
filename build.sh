#!/bin/sh -eu

make ${MAKE_OPTS:-} -C kernel kernel.elf

for MK in $(ls apps/*/Makefile)
do
    APP_DIR=$(dirname $MK)
    APP=$(basename $APP_DIR)
    make ${MAKE_OPTS:-} -C $APP_DIR $APP
done

if [ "${1:-}" = "run" ]
then
    export MIKANOS_DIR=$(dirname "$0")
    export LOADER_EFI="$MIKANOS_DIR/../edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi"
    $MIKANOS_DIR/../osbook/devenv/run_mikanos.sh
fi
