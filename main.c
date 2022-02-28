#include <efi.h>
#include <efilib.h>

static CHAR16 *LicensePath = L"\\EFI\\EFIshell\\LICENSE";
static CHAR16 *GrubPath = L"\\EFI\\gentoo\\grubx64.efi";


static VOID PrintBuffer(IN VOID* Buffer, IN UINTN Size) {
   for (UINTN i = 0; i < Size; i++) {
      CHAR8 *CharPtr = (CHAR8 *)(Buffer + i);
      Print(L"%c", *CharPtr);
   }
}

static EFI_STATUS GetLoadedImage(
   IN EFI_HANDLE ImageHandle, 
   IN EFI_SYSTEM_TABLE *SystemTable,
   OUT EFI_LOADED_IMAGE_PROTOCOL **LoadedImage)
{
   EFI_GUID LoadedImageGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;        
   EFI_STATUS status;

   status =  uefi_call_wrapper(
      SystemTable->BootServices->HandleProtocol, 
      3, 
      ImageHandle, 
      &LoadedImageGUID, 
      (void **) LoadedImage);
   return status;
}

static EFI_STATUS PrintFileContent(
   IN SIMPLE_READ_FILE File) 
{
   EFI_STATUS status;
   CONST UINTN BufferSize = 511;
   UINTN ReadSize;
   VOID* Buffer = AllocateZeroPool(BufferSize + 1);
   UINTN FileOffset = 0;
         
   do {
      ReadSize = BufferSize;
      status = ReadSimpleReadFile(File, FileOffset, &ReadSize, Buffer);
      if (ReadSize > 0)
         PrintBuffer(Buffer, ReadSize);
      FileOffset += ReadSize;
   } while (status == EFI_SUCCESS && ReadSize > 0);
   Print(L"\n");

   FreePool(Buffer);
   return status;
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
   EFI_INPUT_KEY Char;
   InitializeLib(ImageHandle, SystemTable);
   
   uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
   Print(L"1) Boot\n");
   Print(L"2) License\n");
   Print(L"3) Reboot\n");
   Print(L"4) Halt\n");
   Print(L"Enter Your choice:");

   uefi_call_wrapper(SystemTable->ConIn->Reset, 1, SystemTable->ConIn);
   uefi_call_wrapper(SystemTable->BootServices->WaitForEvent, 3, 1, &(SystemTable->ConIn->WaitForKey), NULL);
   uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Char);
   Print(L" %c\n", Char.UnicodeChar);
   
   switch(Char.UnicodeChar) {
      case '1':
         Print(L"Not implemented\n");
         break;
      case '2':
         EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
         EFI_DEVICE_PATH *DevPath;
         EFI_HANDLE DeviceHandle;
         SIMPLE_READ_FILE LicenseFile;
         EFI_STATUS status;

         status = GetLoadedImage(ImageHandle, SystemTable, &LoadedImage);
         if (EFI_ERROR(status)) {
            Print(L"GetLoadedImage: %r\n", status);
            return status;
         }
         
         DevPath = FileDevicePath(LoadedImage->DeviceHandle, LicensePath);
         status = OpenSimpleReadFile(FALSE, NULL, 0, &DevPath, &DeviceHandle, &LicenseFile);
         if (EFI_ERROR(status)) {
            Print(L"OpenSimpleReadFile: %r\n", status);
            FreePool(DevPath);
            return status;
         }
         
         status = PrintFileContent(LicenseFile);
         if (EFI_ERROR(status)) {
            Print(L"PrintFileContent: %r\n", status);
            FreePool(DevPath);
            return status;
         }

         FreePool(DevPath);
         CloseSimpleReadFile(LicenseFile);
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
