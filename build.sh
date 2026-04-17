#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

build_kernel() {
    make ${MAKE_OPTS:-} -C "$SCRIPT_DIR/kernel" kernel.elf
}

build_apps() {
    for mk in "$SCRIPT_DIR"/apps/*/Makefile
    do
        [ -f "$mk" ] || continue

        app_dir=$(dirname "$mk")
        app=$(basename "$app_dir")
        make ${MAKE_OPTS:-} -C "$app_dir" "$app"
    done
}

run_mikanos() {
    MIKANOS_DIR="$SCRIPT_DIR"
    LOADER_EFI=${LOADER_EFI:-"$MIKANOS_DIR/../edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi"}
    DEVENV_DIR="$MIKANOS_DIR/../osbook/devenv"
    DISK_IMG=${DISK_IMG:-"$MIKANOS_DIR/disk.img"}
    APPS_DIR=${APPS_DIR:-"apps"}
    RESOURCE_DIR=${RESOURCE_DIR:-}
    QEMU_OPTS=${QEMU_OPTS:-}

    KERNEL_ELF="$MIKANOS_DIR/kernel/kernel.elf"

    if [ ! -f "$LOADER_EFI" ]
    then
        echo "No such file: $LOADER_EFI"
        exit 1
    fi

    rm -f "$DISK_IMG"
    qemu-img create -f raw "$DISK_IMG" 200M
    mkfs.fat -n 'MIKAN OS' -s 2 -f 2 -R 32 -F 32 "$DISK_IMG"

    mmd -i "$DISK_IMG" ::/EFI
    mmd -i "$DISK_IMG" ::/EFI/BOOT
    mcopy -i "$DISK_IMG" "$LOADER_EFI" ::/EFI/BOOT/BOOTX64.EFI
    mcopy -i "$DISK_IMG" "$KERNEL_ELF" ::/

    if [ "$APPS_DIR" != "" ]
    then
        mmd -i "$DISK_IMG" "::/$APPS_DIR"
    fi

    for mk in "$MIKANOS_DIR"/apps/*/Makefile
    do
        [ -f "$mk" ] || continue

        app_dir=$(dirname "$mk")
        app=$(basename "$app_dir")
        app_bin="$app_dir/$app"

        if [ -f "$app_bin" ]
        then
            if [ "$APPS_DIR" != "" ]
            then
                mcopy -i "$DISK_IMG" "$app_bin" "::/$APPS_DIR"
            else
                mcopy -i "$DISK_IMG" "$app_bin" ::/
            fi
        fi
    done

    if [ "$RESOURCE_DIR" != "" ]
    then
        mcopy -i "$DISK_IMG" "$MIKANOS_DIR/$RESOURCE_DIR"/* ::/
    fi

    qemu-system-x86_64 \
        -m 1G \
        -drive if=pflash,format=raw,readonly=on,file="$DEVENV_DIR/OVMF_CODE.fd" \
        -drive if=pflash,format=raw,file="$DEVENV_DIR/OVMF_VARS.fd" \
        -drive if=ide,index=0,media=disk,format=raw,file="$DISK_IMG" \
        -device nec-usb-xhci,id=xhci \
        -device usb-mouse -device usb-kbd \
        -monitor stdio \
        $QEMU_OPTS
}

main() {
    build_kernel
    build_apps

    if [ "${1:-}" = "run" ]
    then
        run_mikanos
    fi
}

main "$@"
