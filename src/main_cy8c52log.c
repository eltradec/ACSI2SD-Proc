//	Copyright (C) 2014 Michael McMaster <michael@codesrc.com>
//	Modified 2018 (or timestamp yymmdd) Robert Matyschok <rm@eltradec.eu>, modifications commented with "RM" prefix
//	This file is part of SCSI2SD.
//
//	SCSI2SD is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	SCSI2SD is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with SCSI2SD.  If not, see <http://www.gnu.org/licenses/>.

#include "device.h"
#include "scsi.h"
#include "scsiPhy.h"
#include "config.h"
#include "disk.h"
#include "led.h"
#include "SOUND_ISR.h"
#include "time.h"
#include "trace.h"
#include "VDAC8_1.h"

const char* Notice = "Copyright (C) 2015-2018 Michael McMaster <michael@codesrc.com>";
    
void soundInit()
{  
    // Opamp_1_Start(); // RM Opamp not present in CY8C52
    // VDAC8_1_Start(); // RM start VDAC
    // LPF_1_Start(); // RM start LPF
    // SOUND_ISR_Start(); // RM start sound
}

void logInit()
{  
       /* Defines for ACSI_LOG_DMA */
#define ACSI_LOG_DMA_BYTES_PER_BURST 1
#define ACSI_LOG_DMA_REQUEST_PER_BURST 1
#define ACSI_LOG_DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define ACSI_LOG_DMA_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for ACSI_LOG_DMA */
/* Move these variable declarations to the top of the function */
uint8 ACSI_LOG_DMA_Chan;
uint8 ACSI_LOG_DMA_TD[1];
const uint32 ACSI_LOG_BUF = 0x20005800;
const uint16 ACSI_LOG_LEN = 0x0800;

/* DMA Configuration for ACSI_LOG_DMA */
ACSI_LOG_DMA_Chan = ACSI_LOG_DMA_DmaInitialize(ACSI_LOG_DMA_BYTES_PER_BURST, ACSI_LOG_DMA_REQUEST_PER_BURST, 
    HI16(ACSI_LOG_DMA_SRC_BASE), HI16(ACSI_LOG_DMA_DST_BASE));
ACSI_LOG_DMA_TD[0] = CyDmaTdAllocate();
CyDmaTdSetConfiguration(ACSI_LOG_DMA_TD[0], ACSI_LOG_LEN, CY_DMA_DISABLE_TD, CY_DMA_TD_INC_DST_ADR);
CyDmaTdSetAddress(ACSI_LOG_DMA_TD[0], LO16((uint32)ACSI_In_Status_PTR), LO16((uint32)ACSI_LOG_BUF));
CyDmaChSetInitialTd(ACSI_LOG_DMA_Chan, ACSI_LOG_DMA_TD[0]);
CyDmaChEnable(ACSI_LOG_DMA_Chan, 1);

/* Defines for SCSI_LOG_DMA */
#define SCSI_LOG_DMA_BYTES_PER_BURST 1
#define SCSI_LOG_DMA_REQUEST_PER_BURST 1
#define SCSI_LOG_DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define SCSI_LOG_DMA_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for SCSI_LOG_DMA */
/* Move these variable declarations to the top of the function */
uint8 SCSI_LOG_DMA_Chan;
uint8 SCSI_LOG_DMA_TD[1];
const uint32 SCSI_LOG_BUF = 0x20006000;
const uint16 SCSI_LOG_LEN = 0x0800;

/* DMA Configuration for SCSI_LOG_DMA */
SCSI_LOG_DMA_Chan = SCSI_LOG_DMA_DmaInitialize(SCSI_LOG_DMA_BYTES_PER_BURST, SCSI_LOG_DMA_REQUEST_PER_BURST, 
    HI16(SCSI_LOG_DMA_SRC_BASE), HI16(SCSI_LOG_BUF));
SCSI_LOG_DMA_TD[0] = CyDmaTdAllocate();
CyDmaTdSetConfiguration(SCSI_LOG_DMA_TD[0], SCSI_LOG_LEN, CY_DMA_DISABLE_TD, CY_DMA_TD_INC_DST_ADR);
CyDmaTdSetAddress(SCSI_LOG_DMA_TD[0], LO16((uint32)SCSI_In_Status_PTR), LO16((uint32)SCSI_LOG_BUF));
CyDmaChSetInitialTd(SCSI_LOG_DMA_Chan, SCSI_LOG_DMA_TD[0]);
CyDmaChEnable(SCSI_LOG_DMA_Chan, 1);


/* Defines for INQ_OUT_DMA */
#define INQ_OUT_DMA_BYTES_PER_BURST 1
#define INQ_OUT_DMA_REQUEST_PER_BURST 1
#define INQ_OUT_DMA_SRC_BASE (CYDEV_PERIPH_BASE)
#define INQ_OUT_DMA_DST_BASE (CYDEV_SRAM_BASE)

/* Variable declarations for INQ_OUT_DMA */
/* Move these variable declarations to the top of the function */
uint8 INQ_OUT_DMA_Chan;
uint8 INQ_OUT_DMA_TD[1];
const uint32 INQ_OUT_BUF = 0x20006800;
const uint16 INQ_OUT_LEN = 0x0800;

/* DMA Configuration for INQ_OUT_DMA */
INQ_OUT_DMA_Chan = INQ_OUT_DMA_DmaInitialize(INQ_OUT_DMA_BYTES_PER_BURST, INQ_OUT_DMA_REQUEST_PER_BURST, 
    HI16(INQ_OUT_DMA_SRC_BASE), HI16(INQ_OUT_BUF));
INQ_OUT_DMA_TD[0] = CyDmaTdAllocate();
CyDmaTdSetConfiguration(INQ_OUT_DMA_TD[0], INQ_OUT_LEN, CY_DMA_DISABLE_TD, CY_DMA_TD_INC_DST_ADR);
CyDmaTdSetAddress(INQ_OUT_DMA_TD[0], LO16((uint32)ACSI_In_Status_PTR), LO16((uint32)INQ_OUT_BUF));
CyDmaChSetInitialTd(INQ_OUT_DMA_Chan, INQ_OUT_DMA_TD[0]);
CyDmaChEnable(INQ_OUT_DMA_Chan, 1);
}

int main()
{
	timeInit();
	ledInit();
    // soundInit(); // RM
	traceInit();
    // logInit(); // RM 220122

	// Enable global interrupts.
	// Needed for RST and ATN interrupt handlers.
	CyGlobalIntEnable;

	// Set interrupt handlers.
	scsiPhyInit();

	configInit(&scsiDev.boardCfg);
	debugInit();
    scsiPhyConfig();
	scsiInit();
	scsiDiskInit();

	// Optional bootup delay
	int delaySeconds = 0;
	while (delaySeconds < scsiDev.boardCfg.startupDelay) {
		// Keep the USB connection working, otherwise it's very hard to revert
		// silly extra-long startup delay settings.
		int i;
		for (i = 0; i < 200; i++) {
			CyDelay(5);
			scsiDev.watchdogTick++;
			configPoll();
		}
		++delaySeconds;
	}

	uint32_t lastSDPoll = getTime_ms();
	sdCheckPresent();

	while (1)
	{
		scsiDev.watchdogTick++;

		scsiPoll();
		scsiDiskPoll();
		configPoll();
		sdPoll();

		if (unlikely(scsiDev.phase == BUS_FREE))
		{
			if (unlikely(elapsedTime_ms(lastSDPoll) > 200))
			{
				lastSDPoll = getTime_ms();
				sdCheckPresent();
			}
			else
			{
				// Wait for our 1ms timer to save some power.
				// There's an interrupt on the SEL signal to ensure we respond
				// quickly to any SCSI commands. The selection abort time is
				// only 250us, and new SCSI-3 controllers time-out very
				// not long after that, so we need to ensure we wake up quickly.
				uint8_t interruptState = CyEnterCriticalSection();
				if (!SCSI_ReadFilt(SCSI_Filt_SEL))
				{
					__WFI(); // Will wake on interrupt, regardless of mask
				}
				CyExitCriticalSection(interruptState);
			}
		}
		else if ((scsiDev.phase >= 0) && (blockDev.state & DISK_PRESENT))
		{
			// don't waste time scanning SD cards while we're doing disk IO
			lastSDPoll = getTime_ms();
		}
	}
	return 0;
}