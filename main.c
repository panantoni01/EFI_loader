#include <efi.h>
#include <efilib.h>

#define ERROR_CHECK(func, status)      \
if (EFI_ERROR(status)) {               \
   Print((func "" L": %r\n"), status); \
   return status;                      \
}

/* Hard-coded file paths - may need to be changed */
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
      
      ((CHAR8 *)Buffer)[ReadSize] = '\0';
      Print(L"%a", Buffer);
      
      FileOffset += ReadSize;
   } while (status == EFI_SUCCESS && ReadSize > 0);
   
   Print(L"\n");

   FreePool(Buffer);
   return status;
}

static VOID CpuPause () 
{
   asm("pause");
}

EFI_STATUS
EFIAPI
efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) 
{
   CHAR16 *InputChar = L"\0\0\0";
   EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
   EFI_DEVICE_PATH *DevPath;
   EFI_HANDLE GrubHandle;
   EFI_HANDLE DeviceHandle;
   SIMPLE_READ_FILE LicenseFile;
   EFI_STATUS status;
   
   InitializeLib(ImageHandle, SystemTable);
   
   /* Clear screen, print menu and get user input */
   uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
print_menu:
   Print(L"1) Boot\n");
   Print(L"2) License\n");
   Print(L"3) Reboot\n");
   Print(L"4) Halt\n");
   Input(L"Enter Your choice: ", InputChar, 2);
   Print(L"\n");
   
   status = GetLoadedImage(ImageHandle, SystemTable, &LoadedImage);
   ERROR_CHECK(L"GetLoadedImage", status);

   switch(*InputChar) {
      case '1': /* Load grub and transfer control to it */
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
         ERROR_CHECK(L"LoadImage", status);

         status = uefi_call_wrapper(SystemTable->BootServices->StartImage, 3, GrubHandle, 0, NULL);
         ERROR_CHECK(L"StartImage", status);
         
         status = uefi_call_wrapper(SystemTable->BootServices->UnloadImage, 1, GrubHandle);
         ERROR_CHECK(L"UnloadImage", status);
         break;
      
      case '2': /* Print contents of LICENSE file */
         DevPath = FileDevicePath(LoadedImage->DeviceHandle, LicensePath);

         status = OpenSimpleReadFile(FALSE, NULL, 0, &DevPath, &DeviceHandle, &LicenseFile);
         FreePool(DevPath);
         ERROR_CHECK(L"OpenSimpleReadFile", status);
         
         status = PrintFileContent(LicenseFile);
         ERROR_CHECK(L"PrintFileContent", status);

         CloseSimpleReadFile(LicenseFile);
         goto print_menu;
         break;
      
      case '3': /* Reboot */
         uefi_call_wrapper(
            SystemTable->RuntimeServices->ResetSystem,
            4,
            EfiResetWarm,
            EFI_SUCCESS,
            0,
            NULL
         );
         break;
      
      case '4': /* Halt the system. Only Ctrl+Alt+Del can be used to 
         reboot after choosing this option */
         while(1)
            CpuPause();
         break;
      
      default:
         Print(L"Invalid option number: '%c'. Please try again\n", *InputChar);
         goto print_menu;
   }

   return EFI_SUCCESS;
}
