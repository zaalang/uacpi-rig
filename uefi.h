//
// uefi runtime
//

#include <stdint.h>
#include <stddef.h>

typedef uintptr_t EFI_HANDLE;
typedef size_t UINTN;

typedef struct {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} EFI_GUID;

typedef struct {
  void *Reset;
  void *ReadKeyStroke;
  void *WaitForKey;
} EFI_SIMPLE_INPUT_PROTOCOL;

typedef struct {
  void *Reset;
  void *OutputString;
  void *TestString;
  void *QueryMode;
  void *SetMode;
  void *SetAttribute;
  void *ClearScreen;
  void *SetCursorPosition;
  void *EnableCursor;
  void *Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
  uint64_t Signature;
  uint32_t Revision;
  uint32_t HeaderSize;
  uint32_t CRC32;
  uint32_t Reserved;
} EFI_TABLE_HEADER;

typedef struct  {
  EFI_TABLE_HEADER Hdr;

  void *GetTime;
  void *SetTime;
  void *GetWakeupTime;
  void *SetWakeupTime;
  void *SetVirtualAddressMap;
  void *ConvertPointer;
  void *GetVariable;
  void *GetNextVariableName;
  void *SetVariable;
  void *GetNextHighMonotonicCount;
  void *ResetSystem;
  void *UpdateCapsule;
  void *QueryCapsuleCapabilities;
  void *QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

typedef struct {
  EFI_TABLE_HEADER Hdr;

  void *RaiseTPL;
  void *RestoreTPL;
  void *AllocatePages;
  void *FreePages;
  void *GetMemoryMap;
  void *AllocatePool;
  void *FreePool;
  void *CreateEvent;
  void *SetTimer;
  void *WaitForEvent;
  void *SignalEvent;
  void *CloseEvent;
  void *CheckEvent;
  void *InstallProtocolInterface;
  void *ReinstallProtocolInterface;
  void *UninstallProtocolInterface;
  void *HandleProtocol;
  void *PCHandleProtocol;
  void *RegisterProtocolNotify;
  void *LocateHandle;
  void *LocateDevicePath;
  void *InstallConfigurationTable;
  void *LoadImage;
  void *StartImage;
  void *Exit;
  void *UnloadImage;
  void *ExitBootServices;
  void *GetNextMonotonicCount;
  void *Stall;
  void *SetWatchdogTimer;
  void *ConnectController;
  void *DisconnectController;
  void *OpenProtocol;
  void *CloseProtocol;
  void *OpenProtocolInformation;
  void *ProtocolsPerHandle;
  void *LocateHandleBuffer;
  void *LocateProtocol;
  void *InstallMultipleProtocolInterfaces;
  void *UninstallMultipleProtocolInterfaces;
  void *CalculateCrc32;
  void *CopyMem;
  void *SetMem;
  void *CreateEventEx;
} EFI_BOOT_SERVICES;

typedef struct
{
  EFI_GUID VendorGuid;
  uintptr_t VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct {
  EFI_TABLE_HEADER hdr;

  uint16_t *FirmwareVendor;
  uint32_t FirmwareRevision;

  EFI_HANDLE ConsoleInHandle;
  EFI_SIMPLE_INPUT_PROTOCOL *ConIn;

  EFI_HANDLE ConsoleOutHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;

  EFI_HANDLE StandardErrorHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;

  EFI_RUNTIME_SERVICES *RuntimeServices;
  EFI_BOOT_SERVICES *BootServices;

  UINTN NumberOfTableEntries;
  EFI_CONFIGURATION_TABLE *ConfigurationTable;
} EFI_SYSTEM_TABLE;

int puts(const char *str);

void *malloc(size_t size);
void free(void *addr);

uint64_t clock();
void stall(int usec);

void exit(int32_t exitcode);
