#include <efi.h>
#include <efilib.h>

static CHAR16 *LicensePath = L"\\EFI\\EFIshell\\LICENSE";
static CHAR16 *GrubPath = L"\\EFI\\gentoo\\grubx64.efi";

// static EFI_STATUS PrintFileContent(IN CHAR16 *FileName) {
//    EFI_DEVICE_PATH *path;
// }


static VOID PrintBuffer(IN VOID* Buffer, IN UINTN Size) {
   for (UINTN i = 0; i < Size; i++) {
      CHAR8 *CharPtr = (CHAR8 *)(Buffer + i);
      Print(L"%c", *CharPtr);
   }
}

static EFI_STATUS PrintFileContent(IN EFI_HANDLE FileDevice, IN CONST CHAR16* FileName) {
   EFI_STATUS status;
   EFI_DEVICE_PATH *DevPath;
   EFI_HANDLE DeviceHandle;
   SIMPLE_READ_FILE LicenseFile;
   CONST UINTN BufferSize = 510;
   UINTN ReadSize;
   VOID* Buffer;
   UINTN FileOffset = 0;

   DevPath = FileDevicePath(FileDevice, LicensePath);
   status = OpenSimpleReadFile(FALSE, NULL, 0, &DevPath, &DeviceHandle, &LicenseFile);
   if (EFI_ERROR(status)) 
      return status;

   Buffer = AllocateZeroPool(BufferSize + 2);
         
   do {
      ReadSize = BufferSize;
      status = ReadSimpleReadFile(LicenseFile, FileOffset, &ReadSize, Buffer);
      PrintBuffer(Buffer, ReadSize);
      FileOffset += ReadSize;
   } while (status == EFI_SUCCESS && ReadSize > 0);
   Print(L"\n");

   FreePool(Buffer);
   CloseSimpleReadFile(LicenseFile);
   return status;
}

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
         Print(L"Not implemented\n");
         break;
      case '2':
         EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
         EFI_GUID LoadedImageGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;        
         EFI_STATUS status;
         
         status = uefi_call_wrapper(BootServices->HandleProtocol, 3, 
                                    ImageHandle, &LoadedImageGUID, (void **) &LoadedImage);
         if (EFI_ERROR(status)) {
            Print(L"HandleProtocol: %r\n", status);
            return EFI_SUCCESS;
         }
         
         status = PrintFileContent(LoadedImage->DeviceHandle, LicensePath);
         if (EFI_ERROR(status)) {
            Print(L"PrintFileContent: %r\n", status);
            return EFI_SUCCESS;
         }
         break;
      case '3':
         Print(L"Not implemented\n");
         break;
      case '4':
         Print(L"Not implemented\n");
         break;
      default:
         Print(L"Invalid option number, quitting...\n");
   }

   return EFI_SUCCESS;
}
