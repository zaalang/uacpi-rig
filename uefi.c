//
// uefi runtime
//

#include "uefi.h"

uintptr_t _efi_call0(void *);
uintptr_t _efi_call1(void *, uintptr_t);
uintptr_t _efi_call2(void *, uintptr_t, uintptr_t);
uintptr_t _efi_call3(void *, uintptr_t, uintptr_t, uintptr_t);
uintptr_t _efi_call4(void *, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
uintptr_t _efi_call5(void *, uintptr_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

int32_t main(EFI_HANDLE Image, EFI_SYSTEM_TABLE *SystemTable);

EFI_SIMPLE_INPUT_PROTOCOL *ConIn;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
EFI_BOOT_SERVICES *BootServices;
EFI_HANDLE Instance;

uint64_t tsc_frequency = 0;
uint64_t tsc_cyc2ns = 0;

void reloc(uintptr_t base, uintptr_t dynamic, uintptr_t offset) {
  #define DT_RELA 7
  #define DT_RELASZ 8
  #define DT_RELAENT 9
  #define R_X86_64_RELATIVE 8

  struct Elf64_Dyn
  {
    uint64_t tag;
    uintptr_t value;
  };

  struct Elf64_Rela
  {
    uintptr_t offset;
    uint64_t info;
    int64_t addend;
  };

  uintptr_t rela = 0;
  uintptr_t relasz = 0;
  uintptr_t relaent = 0;

  for (struct Elf64_Dyn *entry = (struct Elf64_Dyn*)(dynamic); entry->tag != 0; ++entry)
  {
    switch (entry->tag)
    {
      case DT_RELA:
        rela = base + entry->value;
        break;

      case DT_RELASZ:
        relasz = entry->value;
        break;

      case DT_RELAENT:
        relaent = entry->value;
        break;
    }
  }

  while (relasz != 0)
  {
    struct Elf64_Rela *rel = (struct Elf64_Rela*)(rela);

    if ((rel->info & 0xffffffff) == R_X86_64_RELATIVE)
      *(uintptr_t *)(base + rel->offset) = offset + rel->addend;

    rela += relaent;
    relasz -= relaent;
  }
}

uint64_t rdtsc(void) {
  uint32_t edx, eax;
  asm volatile ("rdtsc" : "=a" (eax), "=d" (edx) :: "memory");
  return ((uint64_t)(edx) << 32) | eax;
}

void init_tsc() {
  uint32_t eax, ebx, ecx, edx;

  asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0), "c"(0));

  // calibrate tsc

  if (eax >= 0x15) {
    asm volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0x15), "c"(0));
    if (eax != 0 && ebx != 0 && ecx != 0)
      tsc_frequency = (uint64_t)(ecx) * ebx / eax;
  }

  if (tsc_frequency == 0)
  {
    uint64_t s = rdtsc();
    stall(1000);
    uint64_t e = rdtsc();

    tsc_frequency = (e - s) * 1000;
  }

  tsc_cyc2ns = ((UINT64_C(1000000000) << 32) / tsc_frequency) << 32;
}

int puts(const char *str) {
  int i = 0;
  uint16_t buffer[1024];

  for (const char *ch = str; *ch != 0; ++ch) {
    if (*ch == '\n')
      buffer[i++] = '\r';

    buffer[i++] = *ch;
  }

  buffer[i++] = 0;

  _efi_call2(ConOut->OutputString, (uintptr_t)ConOut, (uintptr_t)buffer);

  return i;
}

void *malloc(size_t size) {
  uintptr_t memory = 0;
  if (_efi_call3(BootServices->AllocatePool, 2, size, (uintptr_t)&memory) != 0)
    return NULL;

  return (void*)(memory);
}

void free(void *addr) {
  _efi_call1(BootServices->FreePool, (uintptr_t)addr);
}

uint64_t clock() {
  uint64_t tsc = rdtsc();

  return (((tsc & 0xffffffff) * (tsc_cyc2ns >> 32)) >> 32) + (tsc >> 32) * (tsc_cyc2ns >> 32);
}

void stall(int32_t usec) {
  _efi_call1(BootServices->Stall, usec);
}

void exit(int32_t exitcode) {
  for (;;) {
    _efi_call4(BootServices->Exit, Instance, exitcode, 0, 0);
  }
}

void efi_main(EFI_HANDLE Image, EFI_SYSTEM_TABLE *SystemTable) {
  ConIn = SystemTable->ConIn;
  ConOut = SystemTable->ConOut;
  BootServices = SystemTable->BootServices;
  Instance = Image;

  init_tsc();

  exit(main(Image, SystemTable));
}
