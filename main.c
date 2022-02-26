#include <efi.h>
#include <efilib.h>

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   InitializeLib(ImageHandle, SystemTable);
   
   SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut = SystemTable->ConOut;
   SIMPLE_INPUT_INTERFACE *ConIn = SystemTable->ConIn;
   EFI_BOOT_SERVICES *BootServices = SystemTable->BootServices;
   EFI_INPUT_KEY Char;
   
   uefi_call_wrapper(ConOut->ClearScreen, 1, ConOut);
   Print(L"1) Boot\n");
   Print(L"2) License\n");
   Print(L"3) Reboot\n");
   Print(L"4) Halt\n");
   Print(L"Enter Your choice:");

   uefi_call_wrapper(ConIn->Reset, 1, ConIn);
   uefi_call_wrapper(BootServices->WaitForEvent, 3, 1, &ConIn->WaitForKey, NULL);
   uefi_call_wrapper(ConIn->ReadKeyStroke, 2, ConIn, &Char);
   Print(L" %c\n", Char.UnicodeChar);
   
   switch(Char.UnicodeChar) {
      case '1':
         // TODO
         break;
      case '2':
         // TODO
         break;
      case '3':
         // TODO
         break;
      case '4':
         // TODO
         break;
      default:
         Print(L"Invalid option number, quitting...\n");
   }

   return EFI_SUCCESS;
}
