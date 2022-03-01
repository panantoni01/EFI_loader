#include <efi.h>
#include <efilib.h>

static CHAR16 *LicensePath = L"\\EFI\\EFIshell\\LICENSE";
static CHAR16 *GrubPath = L"\\EFI\\gentoo\\grubx64.efi";


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
      (void **) LoadedImage
   );
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
      if (ReadSize > 0) {
         ((CHAR8 *)Buffer)[ReadSize] = '\0';
         Print(L"%a", Buffer);
      }
      FileOffset += ReadSize;
   } while (status == EFI_SUCCESS && ReadSize > 0);
   Print(L"\n");

   FreePool(Buffer);
   return status;
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) 
{
   EFI_INPUT_KEY Char;
   EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
   EFI_DEVICE_PATH *DevPath;
   EFI_HANDLE GrubHandle;
   EFI_HANDLE DeviceHandle;
   SIMPLE_READ_FILE LicenseFile;
   EFI_STATUS status;
   
   InitializeLib(ImageHandle, SystemTable);
   
   uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
   Print(L"1) Boot\n");
   Print(L"2) License\n");
   Print(L"3) Reboot\n");
   Print(L"4) Halt\n");
   Print(L"Enter Your choice:");

   uefi_call_wrapper(SystemTable->ConIn->Reset, 1, SystemTable->ConIn);
   uefi_call_wrapper(
      SystemTable->BootServices->WaitForEvent, 
      3, 
      1, 
      &(SystemTable->ConIn->WaitForKey), 
      NULL
   );
   uefi_call_wrapper(SystemTable->ConIn->ReadKeyStroke, 2, SystemTable->ConIn, &Char);
   Print(L" %c\n", Char.UnicodeChar);
   
   status = GetLoadedImage(ImageHandle, SystemTable, &LoadedImage);
   if (EFI_ERROR(status)) {
      Print(L"GetLoadedImage: %r\n", status);
      return status;
   }

   switch(Char.UnicodeChar) {
      case '1':
         DevPath = FileDevicePath(LoadedImage->DeviceHandle, GrubPath);
         
         status = uefi_call_wrapper(
            SystemTable->BootServices->LoadImage, 
            6, 
            FALSE, 
            ImageHandle,
            DevPath, 
            NULL, 
            0,
            &GrubHandle
         );
         FreePool(DevPath);
         if (EFI_ERROR(status)) {
            Print(L"LoadImage: %r\n", status);
            return status;
         }

         status = uefi_call_wrapper(SystemTable->BootServices->StartImage, 3, GrubHandle, 0, NULL);
         if (EFI_ERROR(status)) {
            Print(L"StartImage: %r\n", status);
            return status;
         }
         
         status = uefi_call_wrapper(SystemTable->BootServices->UnloadImage, 1, GrubHandle);
         if (EFI_ERROR(status)) {
            Print(L"UnloadImage: %r\n", status);
            return status;
         }
         break;
      case '2':
         DevPath = FileDevicePath(LoadedImage->DeviceHandle, LicensePath);

         status = OpenSimpleReadFile(FALSE, NULL, 0, &DevPath, &DeviceHandle, &LicenseFile);
         FreePool(DevPath);
         if (EFI_ERROR(status)) {
            Print(L"OpenSimpleReadFile: %r\n", status);
            return status;
         }
         
         status = PrintFileContent(LicenseFile);
         if (EFI_ERROR(status)) {
            Print(L"PrintFileContent: %r\n", status);
            return status;
         }

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
