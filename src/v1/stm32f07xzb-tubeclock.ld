/*
 * Custom linker script for TubeClock project
 * Based on STM32F07xzB, 128k flash, 16k RAM
 *
 * This script reserves the last 1KB of flash for settings storage,
 * ensuring the program binary cannot overwrite the settings area.
 *
 * Settings struct size: ~118 bytes (25×uint16_t + 8×DateTime + 1×uint32_t)
 * Reserved size: 1KB provides ample room for future expansion
 */

/* Define memory regions */
MEMORY
{
	/* Main flash for program code: 127KB (0x1FC00 bytes) */
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 127K

	/* Reserved settings flash area: 1KB at end of flash */
	settings (r) : ORIGIN = 0x0801FC00, LENGTH = 1K

	/* RAM */
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
}

/* Export the settings flash address as a linker symbol */
PROVIDE(__settings_flash_start = ORIGIN(settings));
PROVIDE(__settings_flash_end = ORIGIN(settings) + LENGTH(settings));

/* Include the common ld script */
INCLUDE cortex-m-generic.ld
