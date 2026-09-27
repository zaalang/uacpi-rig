#include <string.h>
#include <uacpi/kernel_api.h>
#include <uacpi/context.h>
#include <uacpi/tables.h>
#include <uacpi/uacpi.h>
#include <uacpi/acpi.h>
#include "uefi.h"

uacpi_phys_addr rsdp = 0;

EFI_GUID ACPI_20_TABLE_GUID = { 0x8868e871, 0xe4f1, 0x11d3, { 0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }};

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
  return (void*)(addr);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *text) {
  switch (level)
  {
    case UACPI_LOG_DEBUG:
      puts("[DEBUG] "); puts(text);;
      break;

    case UACPI_LOG_TRACE:
      puts("[TRACE] "); puts(text);
      break;

    case UACPI_LOG_INFO:
      puts("[INFO] "); puts(text);
      break;

    case UACPI_LOG_WARN:
      puts("[WARN] "); puts(text);
      break;

    default:
      puts("[ERROR] "); puts(text);
      break;
  }
}

uacpi_status uacpi_kernel_pci_device_open(uacpi_pci_address address, uacpi_handle *out_handle) {
  uacpi_table mcfg;
  uacpi_status status;

  status = uacpi_table_find_by_signature(ACPI_MCFG_SIGNATURE, &mcfg);
  if (uacpi_unlikely_error(status))
    return status;

  size_t mcfg_entry_count = (mcfg.hdr->length - sizeof(struct acpi_mcfg)) / sizeof(struct acpi_mcfg_allocation);

  for (size_t i = 0; i < mcfg_entry_count; ++i) {
    struct acpi_mcfg_allocation *entry = ((struct acpi_mcfg *)mcfg.hdr)->entries + i;

    if (entry->segment == address.segment && entry->start_bus <= address.bus && address.bus <= entry->end_bus) {
      uacpi_u64 config_address = entry->address;
      config_address += (uacpi_u64)(address.bus) << 20;
      config_address += (uacpi_u64)(address.device) << 15;
      config_address += (uacpi_u64)(address.function) << 12;

      *out_handle = malloc(sizeof(uacpi_u64));
      *(uacpi_u64*)(*out_handle) = config_address;

      return UACPI_STATUS_OK;
    }
  }

  return UACPI_STATUS_NOT_FOUND;
}

void uacpi_kernel_pci_device_close(uacpi_handle handle) {
  free(handle);
}

uacpi_status uacpi_kernel_pci_read8(uacpi_handle device, uacpi_size offset, uacpi_u8 *out_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *out_value = *(volatile const uint8_t *)(base + offset);
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read16(uacpi_handle device, uacpi_size offset, uacpi_u16 *out_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *out_value = *(volatile const uint16_t *)(base + offset);
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_read32(uacpi_handle device, uacpi_size offset, uacpi_u32 *out_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *out_value = *(volatile const uint32_t *)(base + offset);
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write8(uacpi_handle device, uacpi_size offset, uacpi_u8 in_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *(volatile uint8_t *)(base + offset) = in_value;
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write16(uacpi_handle device, uacpi_size offset, uacpi_u16 in_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *(volatile uint16_t *)(base + offset) = in_value;
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_pci_write32(uacpi_handle device, uacpi_size offset, uacpi_u32 in_value) {
  uacpi_u64 base = *(uacpi_u64*)(device);
  *(volatile uint32_t *)(base + offset) = in_value;
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_map(uacpi_io_addr base, uacpi_size len, uacpi_handle *out_handle) {
  *out_handle = malloc(sizeof(uacpi_io_addr));
  *(uacpi_io_addr*)(*out_handle) = base;
  return UACPI_STATUS_OK;
}

void uacpi_kernel_io_unmap(uacpi_handle handle) {
}

uacpi_status uacpi_kernel_io_read8(uacpi_handle handle, uacpi_size offset, uacpi_u8 *out_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("inb %w1,%0" : "=a" (*out_value) : "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read16(uacpi_handle handle, uacpi_size offset, uacpi_u16 *out_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("inw %w1,%0" : "=a" (*out_value) : "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_read32(uacpi_handle handle, uacpi_size offset, uacpi_u32 *out_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("inl %w1,%0" : "=a" (*out_value) : "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write8(uacpi_handle handle, uacpi_size offset, uacpi_u8 in_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("outb %0, %w1" : : "a" (in_value), "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write16(uacpi_handle handle, uacpi_size offset, uacpi_u16 in_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("outw %0, %w1" : : "a" (in_value), "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_io_write32(uacpi_handle handle, uacpi_size offset, uacpi_u32 in_value) {
  uacpi_io_addr base = *(uacpi_io_addr*)(handle);
  asm volatile("outl %0, %w1" : : "a" (in_value), "d" (base + offset) : "memory");
  return UACPI_STATUS_OK;
}

void *uacpi_kernel_alloc(uacpi_size size) {
  return malloc(size);
}

void uacpi_kernel_free(void *addr) {
  free(addr);
}

uacpi_u64 uacpi_kernel_get_nanoseconds_since_boot(void) {
  return clock();
}

void uacpi_kernel_stall(uacpi_u8 usec) {
  stall(usec);
}

void uacpi_kernel_sleep(uacpi_u64 msec) {
  stall(msec * 1000);
}

uacpi_handle uacpi_kernel_create_mutex(void) {
  return malloc(sizeof(int));
}

void uacpi_kernel_free_mutex(uacpi_handle handle) {
  free(handle);
}

uacpi_handle uacpi_kernel_create_event(void) {
  return malloc(sizeof(int));
}

void uacpi_kernel_free_event(uacpi_handle handle) {
  free(handle);
}

uacpi_thread_id uacpi_kernel_get_thread_id(void) {
  return (uacpi_thread_id)(1);
}

uacpi_interrupt_state uacpi_kernel_disable_interrupts(void) {
  puts("todo: uacpi_kernel_disable_interrupts\n");
  return UACPI_STATUS_UNIMPLEMENTED;
}

void uacpi_kernel_restore_interrupts(uacpi_interrupt_state state) {
  puts("todo: uacpi_kernel_restore_interrupts\n");
}

uacpi_status uacpi_kernel_acquire_mutex(uacpi_handle handle, uacpi_u16 timeout) {
  return UACPI_STATUS_OK;
}

void uacpi_kernel_release_mutex(uacpi_handle handle) {
}

uacpi_bool uacpi_kernel_wait_for_event(uacpi_handle handle, uacpi_u16 timeout) {
  puts("todo: uacpi_kernel_wait_for_event\n");
  return UACPI_TRUE;
}

void uacpi_kernel_signal_event(uacpi_handle handle) {
  puts("todo: uacpi_kernel_signal_event\n");
}

void uacpi_kernel_reset_event(uacpi_handle handle) {
  puts("todo: uacpi_kernel_reset_event\n");
}

uacpi_status uacpi_kernel_handle_firmware_request(uacpi_firmware_request *request) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_install_interrupt_handler(uacpi_u32 irq, uacpi_interrupt_handler interrupt_handler, uacpi_handle ctx, uacpi_handle *out_irq_handle) {
  return UACPI_STATUS_OK;
}

uacpi_status uacpi_kernel_uninstall_interrupt_handler(uacpi_interrupt_handler interrupt_handler, uacpi_handle irq_handle) {
  return UACPI_STATUS_OK;
}

uacpi_handle uacpi_kernel_create_spinlock(void) {
  return malloc(sizeof(int));
}

void uacpi_kernel_free_spinlock(uacpi_handle handle) {
  free(handle);
}

uacpi_cpu_flags uacpi_kernel_lock_spinlock(uacpi_handle handle) {
  return 0;
}

void uacpi_kernel_unlock_spinlock(uacpi_handle handle, uacpi_cpu_flags flags) {
}

uacpi_status uacpi_kernel_schedule_work(uacpi_work_type work_type, uacpi_work_handler work_handler, uacpi_handle ctx) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_wait_for_work_completion(void) {
  return UACPI_STATUS_UNIMPLEMENTED;
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
  *out_rsdp_address = rsdp;
  return UACPI_STATUS_OK;
}

int32_t main(EFI_HANDLE Image, EFI_SYSTEM_TABLE *SystemTable)
{
  puts("Rig\n");

  // find rsdp

  for (size_t i = 0; i < SystemTable->NumberOfTableEntries; ++i)
  {
    EFI_CONFIGURATION_TABLE *table = SystemTable->ConfigurationTable + i;

    if (memcmp(&table->VendorGuid, &ACPI_20_TABLE_GUID, sizeof(EFI_GUID)) == 0)
      rsdp = table->VendorTable;
  }

  // init uacpi

  uacpi_status status;

  //uacpi_context_set_log_level(UACPI_LOG_ERROR);

  status = uacpi_initialize(0);
  if (uacpi_unlikely_error(status)) {
      puts("uacpi_initialize error: ");
      puts(uacpi_status_to_string(status));
      puts("\n");
      return -6;
  }

  status = uacpi_namespace_load();
  if (uacpi_unlikely_error(status)) {
    puts("uacpi_namespace_load error: ");
    puts(uacpi_status_to_string(status));
    puts("\n");
    return -6;
  }

  uacpi_state_reset();

  return 0;
}