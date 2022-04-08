# EFI loader
Simple EFI application created for Linux course at UWr. Task:
```
Zapoznaj się i przygotuj krótkie omówienie pakietu GNU EFI. Napisz prostą
aplikację EFI o nazwie loader.efi, która po uruchomieniu wyświetla menu:
1. Boot (przekazuje sterowanie do prawdziwego bootloadera),
2. License (wypisuje na ekranie plik zawierający treść licencji MIT),
3. Reboot (powoduje ponowne uruchomienie płyty głównej),
4. Halt (zatrzymuje system).
Ścieżki do prawdziwego bootloadera i pliku tekstowego mogą być „zaszyte” w aplikacji. Warto przeczy-
tać: www.rodsbooks.com/efi-programming.
```

# Build and install

1. Make sure, that the paths in ```main.c``` to LICENSE and grub files are ok:
    * identify mountpoint of Your EFI partition (e.g. using lsblk) - probably ```/boot/efi```
    * copy the LICENSE file (e.g. ```cp LICENSE /boot/efi/EFI/YourDestDir/```)
    * find grub efi binary - should be at ```/boot/efi/EFI/YourOSname/grubx64.efi```
    * adjust the *LicensePath* and *GrubPath* strings
2. You may need to change some variables in Makefile:
    * EFIINC - path to the ```efi.h``` and ```efilib.h``` header files
    * LIB, EFILIB - paths to the static libraries - ```libefi.a```,```libgnuefi.a``` and ```crt0-efi-ARCH.o```, ```elf_ARCH_efi.lds``` files. On gentoo these are the same (*/usr/lib64*), however they may be different on other Linux distributions.
3. Run ```make```. EFI binary file should appear in the current directory.
4. Copy this binary to the EFI partition (e.g. ```cp loader.efi /boot/efi/EFI/YourDestDir/```).

# How to run
Note: I don't take responsibility for any possible damage - make sure You know what You're doing. It is much safer to run the application in virtual machine, e.g. qemu, or at least run it from EFI shell. Always be careful, when messing with EFI system partition!

1. Use ```efibootmgr``` to list the boot options and order.
2. Create new boot option (e.g ```efibootmgr -c -L "YourLabel" -l 'EFI\YourDestDir\loader.efi'```).
3. Change boot order with ```efibootmgr --bootorder```
4. If You want to delete the created boot option, run ```efibootmgr -B -b n```, where *n* is the number of the option