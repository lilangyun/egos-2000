/*
 * (C) 2026, Cornell University
 * All rights reserved.
 *
 * Description: bootloader
 * Initialize the tty device, disk device, MMU, and CPU interrupts.
 */

#include "process.h"

void tty_init();
void disk_init();
void mmu_init();
void intr_init(uint core_id);
void grass_entry(uint core_id);

struct grass* grass = (void*)GRASS_STRUCT;
struct earth* earth = (void*)EARTH_STRUCT;

/* Student's code goes here (Ethernet & TCP/IP). */

/* Define an array of 128 RX descriptors and a buffer of 128*2048 bytes. Every
 * descriptor corresponds to a 2048-byte RX buffer for receiving from Ethernet.
 */

/* Student's code ends here. */

void boot() {
    uint core_id, vendor_id;
    asm("csrr %0, mhartid" : "=r"(core_id));
    asm("csrr %0, mvendorid" : "=r"(vendor_id));
    earth->platform = (vendor_id == 666) ? HARDWARE : QEMU;

    if (booted_core_cnt++ == 0) {
        /* The first booted core needs to do some more work. */
        tty_init();
        CRITICAL("--- Booting on %s with core #%d ---",
                 earth->platform == HARDWARE ? "Hardware" : "QEMU", core_id);

        disk_init();
        SUCCESS("Finished initializing the tty and disk devices");

        mmu_init();
        intr_init(core_id);
        SUCCESS("Finished initializing the MMU, timer and interrupts");

        /* Student's code goes here (I/O Device Driver). */

        /* Initialize QEMU's standard VGA device for apps/user/video_demo.c.
         * Start with https://www.qemu.org/docs/master/specs/standard-vga.html,
         * and you could ask AI/LLMs about the Bochs Dispi (Display Interface).
         * Your driver should setup the PCI ECAM for VGA, and then set the VGA
         * screen resolution to 800*600 pixels, each using 4 bytes for its RGB
         * information. Lastly, initialize all the pixels with white color. */
        #define VGA_PCI_ECAM  0x30010000UL
        #define VGA_MMIO_BASE 0x42000000UL
        #define PCI_ECAM_ALLOW_MMIO_AND_DMA ((1 << 1) | (1 << 2))

        /* Set the PCI ECAM base address register to SDHCI_BASE. */
        REGW(VGA_PCI_ECAM, 0x4)  = PCI_ECAM_ALLOW_MMIO_AND_DMA;
        REGW(VGA_PCI_ECAM, 0x10) = VIDEO_FRAME_BASE;
        REGW(VGA_PCI_ECAM, 0x18) = VGA_MMIO_BASE;
        INFO("VGA BAR0 = 0x%x, BAR2 = 0x%x",
            REGW(VGA_PCI_ECAM, 0x10), REGW(VGA_PCI_ECAM, 0x18));

        /* Setup the screen resolution */
        #define VBE_DISPI_INDEX_XRES   (1) 
        #define VBE_DISPI_INDEX_YRES   (2)
        #define VBE_DISPI_INDEX_BPP    (3)
        #define VBE_DISPI_INDEX_ENABLE (4)
        REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_ENABLE<<1)) = 0x0;
        REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_XRES<<1))   = 800;
        REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_YRES<<1))   = 600;
        REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_BPP<<1))    = 32;
        REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_ENABLE<<1)) = 0x1;

        /* Turn the VGA video on through the legacy Attribute Controller.
         * QEMU's std VGA gates its whole graphics path on the AR index
         * register's PAS bit -- vga_update_display() does:
         *     if (!(s->ar_index & 0x20)) graphic_mode = GMODE_BLANK;
         * and GMODE_BLANK -> vga_draw_blank() never resizes the surface, so
         * the screen stays stuck on the 640x480 "Guest has not initialized
         * the display (yet)." placeholder. vbe_update_vgaregs() sets gr[6]
         * but never touches ar_index; on an x86 PC the VGA BIOS does it.
         * We have no VGA BIOS (-bios tools/egos.bin is bare firmware), so we
         * must. BAR2 maps VGA I/O ports at +0x400 (port 0x3C0), so read
         * Input Status 1 (0x3DA, at +0x41A) to reset the AR flip-flop, then
         * write 0x20 as the AR index to switch the video on. */
        (void)REGB(VGA_MMIO_BASE, 0x41A);
        REGB(VGA_MMIO_BASE, 0x400) = 0x20;

        INFO("DISPI_X = 0x%x, DISPI_Y = 0x%x, DISPI_BPP = 0x%x, DISPI_ENA = 0x%x",
            REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_XRES<<1)),
            REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_YRES<<1)),
            REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_BPP<<1)),
            REGH(VGA_MMIO_BASE, 0x500 + (VBE_DISPI_INDEX_ENABLE<<1))
        );

        /* Initialize all the pixels with white color */
        uint *fb = (uint*)VIDEO_FRAME_BASE;
        for (int i = 0; i < 800 * 600; i++) {
            fb[i] = 0x00FFFFFF;
        }

        /* Student's code ends here. */

        /* Student's code goes here (Ethernet & TCP/IP). */

        /* Use ETH_CTL_BASE as the BAR0 in Ethernet's PCIe configuration. Enable
         * the Ethernet controller's RXT0 (Receiver Timer Interrupt). Initialize
         * the MAC address register and the RX descriptor registers. Ensure that
         * every RX descriptor's address field is written with its corresponding
         * RX buffer's address. Read the descriptions for RCTL (Receive Control)
         * and decide which bits should be set to 1. */

        /* Student's code ends here. */

        grass_entry(core_id);
    } else {
        SUCCESS("--- Core #%d starts running ---", core_id);

        /* Student's code goes here (Multicore & Locks). */

        /* Refer to mmu_init() and intr_init(), and decide how to initialize
         * the CSRs for virtual memory and interrupts on this CPU core. */

        /* Reset the timer, release the boot lock, and run the wfi instruction.
         * After the next timer interrupt, this CPU core will enter the kernel,
         * and the kernel could schedule a process to run on this CPU core. */

        /* Student's code ends here. */
    }
}