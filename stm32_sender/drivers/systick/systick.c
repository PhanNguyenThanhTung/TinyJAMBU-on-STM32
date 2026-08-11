#include "systick.h"
#include "systick_register.h"
#include "project_config.h"

static volatile uint32_t g_systick_counter = 0;

status_t systick_init(void) {
    /* 
      Gia tri Reload cho 1ms
      F_sys = 36MHz => 1ms = 36K tick
      Systick dem nguoc tu N ve 0 => 36K - 1 = 35,999K tick 
    */
    uint32_t reload_val = (SYSTEM_CLOCK_HZ / 1000UL) - 1UL;
    /* Thanh ghi LOAD cua SysTick chi co toi da 24 bit*/
    if(reload_val > 0x00FFFFFFUL) {
        return STATUS_INVALID_PARAMETER;
    }
    /* Reset gia tri dem ve 0 */
    SYSTICK->VAL = 0UL;
    /* Nap gia tri dem 35,999 tick vao LOAD */
    SYSTICK->LOAD = reload_val & 0x00FFFFFFUL;
    /*
      Bit 2 (CLKSOURCE) = 1: Chọn Processor Core Clock (36MHz AHB)
      Bit 1 (TICKINT)   = 1: Bật ngắt SysTick Exception
      Bit 0 (ENABLE)    = 1: Kích hoạt bộ đếm bắt đầu chạy
    */
    SYSTICK->CTRL = (7UL << 0);
    return STATUS_OK;
}

uint32_t systick_get_tick(void) {
  return g_systick_counter;
}

void systick_delay_ms(uint32_t delay_ms) {
  uint32_t start_tick = systick_get_tick();
  
  while ((systick_get_tick() - start_tick) < delay_ms) {
    __asm volatile("nop"); /* Lệnh Assembly No Operation */
  }
}

void SysTick_Handler(void) { /* hàm ngắt */
  g_systick_counter++;
}