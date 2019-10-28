/***************************************************************************
 *                                                                         *
 *          ###########   ###########   ##########    ##########           *
 *         ############  ############  ############  ############          *
 *         ##            ##            ##   ##   ##  ##        ##          *
 *         ##            ##            ##   ##   ##  ##        ##          *
 *         ###########   ####  ######  ##   ##   ##  ##    ######          *
 *          ###########  ####  #       ##   ##   ##  ##    #    #          *
 *                   ##  ##    ######  ##   ##   ##  ##    #    #          *
 *                   ##  ##    #       ##   ##   ##  ##    #    #          *
 *         ############  ##### ######  ##   ##   ##  ##### ######          *
 *         ###########    ###########  ##   ##   ##   ##########           *
 *                                                                         *
 *            S E C U R E   M O B I L E   N E T W O R K I N G              *
 *                                                                         *
 * Copyright (c) 2019 Matthias Schulz                                      *
 *                                                                         *
 * Permission is hereby granted, free of charge, to any person obtaining a *
 * copy of this software and associated documentation files (the           *
 * "Software"), to deal in the Software without restriction, including     *
 * without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to      *
 * permit persons to whom the Software is furnished to do so, subject to   *
 * the following conditions:                                               *
 *                                                                         *
 * 1. The above copyright notice and this permission notice shall be       *
 *    include in all copies or substantial portions of the Software.       *
 *                                                                         *
 * 2. Any use of the Software which results in an academic publication or  *
 *    other publication which includes a bibliography must include         *
 *    citations to the nexmon project a) and the paper cited under b):     *
 *                                                                         *
 *    a) "Matthias Schulz, Daniel Wegemer and Matthias Hollick. Nexmon:    *
 *        The C-based Firmware Patching Framework. https://nexmon.org"     *
 *                                                                         *
 *    b) "Francesco Gringoli, Matthias Schulz, Jakob Link, and Matthias    *
 *        Hollick. Free Your CSI: A Channel State Information Extraction   *
 *        Platform For Modern Wi-Fi Chipsets. Accepted to appear in        *
 *        Proceedings of the 13th Workshop on Wireless Network Testbeds,   *
 *        Experimental evaluation & CHaracterization (WiNTECH 2019),       *
 *        October 2019."                                                   *
 *                                                                         *
 * 3. The Software is not used by, in cooperation with, or on behalf of    *
 *    any armed forces, intelligence agencies, reconnaissance agencies,    *
 *    defense agencies, offense agencies or any supplier, contractor, or   *
 *    research associated.                                                 *
 *                                                                         *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF              *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    *
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       *
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  *
 *                                                                         *
 **************************************************************************/

#pragma NEXMON targetregion "patch"

#include <firmware_version.h>
#include <wrapper.h>
#include <structs.h>
#include <patcher.h>
#include <helper.h>
#include <capabilities.h>
#include <channels.h>

extern void prepend_ethernet_ipv4_udp_header(struct sk_buff *p);

#define WL_RSSI_ANT_MAX     4           /* max possible rx antennas */

// header of csi frame coming from ucode
struct d11csihdr {
#if NEXMON_CHIP == CHIP_VER_BCM4366c0
#define CSIDATA_PER_CHUNK   64
    uint16 NexmonCSICfg;
    uint32 csi[CSIDATA_PER_CHUNK>>2];
    uint16 csiconf;
    uint16 start;
    uint8  src[6];
    uint16 seqcnt;
    uint16 chanspec;
#else
    uint16 RxFrameSize;                 /* 0x000 Set to 0x2 for CSI frames */
    uint16 NexmonExt;                   /* 0x002 */
    uint16 NexmonCSICfg;		/* 0x004 Configuration of this CSI */
    uint16 NexmonCSILen;		/* 0x006 Number of bytes in this chunk */
    uint32 csi[];                       /* 0x008 Array of CSI data */
#endif
} __attribute__((packed));

// original hardware header
struct d11rxhdr {
    uint16 RxFrameSize;                 /* 0x000 Set to 0x2 for CSI frames */
    uint16 NexmonExt;                   /* 0x002 */
    uint16 PhyRxStatus_0;               /* 0x004 PhyRxStatus 15:0 */
    uint16 PhyRxStatus_1;               /* 0x006 PhyRxStatus 31:16 */
    uint16 PhyRxStatus_2;               /* 0x008 PhyRxStatus 47:32 */
    uint16 PhyRxStatus_3;               /* 0x00a PhyRxStatus 63:48 */
    uint16 PhyRxStatus_4;               /* 0x00c PhyRxStatus 79:64 */
    uint16 PhyRxStatus_5;               /* 0x00e PhyRxStatus 95:80 */
    uint16 RxStatus1;                   /* 0x010 MAC Rx Status */
    uint16 RxStatus2;                   /* 0x012 extended MAC Rx status */
    uint16 RxTSFTime;                   /* 0x014 RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY */
    uint16 RxChan;                      /* 0x016 gain code, channel radio code, and phy type */
#if NEXMON_CHIP == CHIP_VER_BCM4366c0
    uint16 RxFameSize_0;                /* size of rx-frame in fifo-0 in case frame is copied to fifo-1 */
    uint16 HdrConvSt;                   /* hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
    uint16 AvbRxTimeL;                  /* AVB RX timestamp low16 */
    uint16 AvbRxTimeH;                  /* AVB RX timestamp high16 */
    uint16 MuRate;                      /* MU rate info (bit3:0 MCS, bit6:4 NSTS) */
    uint16 Pad;
#endif
} __attribute__((packed));

// header or regular frame coming from ucode
struct nexmon_d11rxhdr {
    struct d11rxhdr rxhdr;              /* 0x000 d11rxhdr */
    uint8 SrcMac[6];                    /* 0x018 source mac address */
} __attribute__((packed));

// header after process_frame_hook
struct wlc_d11rxhdr {
    struct d11rxhdr rxhdr;              /* 0x000 d11rxhdr */
    uint32  tsf_l;                      /* 0x018 TSF_L reading */
    int8    rssi;                       /* 0x01c computed instantaneous rssi in BMAC */
    int8    rxpwr0;                     /* 0x01d obsoleted, place holder for legacy ROM code. use rxpwr[] */
    int8    rxpwr1;                     /* 0x01e obsoleted, place holder for legacy ROM code. use rxpwr[] */
    int8    do_rssi_ma;                 /* 0x01f do per-pkt sampling for per-antenna ma in HIGH */
    int8    rxpwr[WL_RSSI_ANT_MAX];     /* 0x020 rssi for supported antennas */
    int8    rssi_qdb;                   /* 0x024 qdB portion of the computed rssi */
    uint8   PAD[2];                     /* 0x025 extra padding to fill up RXE_RXHDR_EXTRA */
} __attribute__((packed));

struct csi_udp_frame {
    struct ethernet_ip_udp_header hdrs;
    uint32 kk1;
    uint8 SrcMac[6];
    uint16 seqCnt;
    uint16 csiconf;
    uint16 chanspec;
    uint16 chip;
    uint32 csi_values[];
} __attribute__((packed));

struct int14 {signed int val:14;} __attribute__((packed));

uint16 missing_csi_frames = 0;
uint16 inserted_csi_values = 0;
struct sk_buff *p_csi = 0;

void
create_new_csi_frame(struct wl_info *wl, uint16 csiconf, int length)
{
    struct osl_info *osh = wl->wlc->osh;
    // create new csi udp frame
    p_csi = pkt_buf_get_skb(osh, sizeof(struct csi_udp_frame) + length);
    // fill header
    struct csi_udp_frame *udpfrm = (struct csi_udp_frame *) p_csi->data;
    // add magic bytes, csi config and chanspec to new udp frame
    udpfrm->kk1 = 0x11111111;
    udpfrm->seqCnt = 0;
    udpfrm->csiconf = csiconf;
    udpfrm->chanspec = get_chanspec(wl->wlc);
    udpfrm->chip = NEXMON_CHIP;
}

void
process_frame_hook(struct sk_buff *p, struct wlc_d11rxhdr *wlc_rxhdr, struct wlc_hw_info *wlc_hw, int tsf_l)
{
    struct osl_info *osh = wlc_hw->wlc->osh;
    struct wl_info *wl = wlc_hw->wlc->wl;
#if NEXMON_CHIP == CHIP_VER_BCM4366c0
    if (wlc_rxhdr->rxhdr.Pad) {
        struct d11csihdr *ucodecsifrm = (struct d11csihdr *) &(wlc_rxhdr->rxhdr.Pad);
        int missing = ucodecsifrm->NexmonCSICfg & 0xff;
        int tones = CSIDATA_PER_CHUNK>>2;
        uint16 csiconf = ucodecsifrm->csiconf;
#define NEWCSI	0x8000
        // check this is a new frame
        if (ucodecsifrm->start & NEWCSI) {
#else
    if (wlc_rxhdr->rxhdr.RxFrameSize == 2) {
        struct d11csihdr *ucodecsifrm = (struct d11csihdr *) &(wlc_rxhdr->rxhdr);
        int missing = ucodecsifrm->NexmonCSICfg & 0xff;
        int tones = ucodecsifrm->NexmonCSILen;
        uint16 csiconf = (ucodecsifrm->NexmonCSICfg >> 8)&0x3f;
#define CSIDATA_PER_CHUNK   56
#define NEWCSI	0x4000
        // check this is a new frame
        if (ucodecsifrm->NexmonCSICfg & NEWCSI) {
#endif
            if (p_csi != 0) {
                printf("unexpected new csi, clearing old\n");
                pkt_buf_free_skb(osh, p_csi, 0);
            }
            create_new_csi_frame(wl, csiconf, missing * CSIDATA_PER_CHUNK);
            missing_csi_frames = missing;
            inserted_csi_values = 0;
        }
        else if (p_csi == 0) {
            printf("unexpected csi data\n");
            pkt_buf_free_skb(osh, p, 0);
            return;
        }
        else if (missing != missing_csi_frames) {
            printf("number of missing frames mismatch\n");
            pkt_buf_free_skb(osh, p, 0);
            pkt_buf_free_skb(osh, p_csi, 0);
            p_csi = 0;
            return;
        }

        struct csi_udp_frame *udpfrm = (struct csi_udp_frame *) p_csi->data;

        int i;
        for (i = 0; i < tones; i ++) {
#if ((NEXMON_CHIP == CHIP_VER_BCM4339) || (NEXMON_CHIP == CHIP_VER_BCM43455c0))
            // csi format is 4bit null, int14 real, int14 imag
            // convert to int16 real, int16 imag
            struct int14 sint14;
            sint14.val = (ucodecsifrm->csi[i] >> 14) & 0x3fff;
            udpfrm->csi_values[inserted_csi_values] = (uint32)((int16)(sint14.val)<<16);
            sint14.val = ucodecsifrm->csi[i] & 0x3fff;
            udpfrm->csi_values[inserted_csi_values] |= ((uint32)((int16)(sint14.val)))&0xffff;
#elif ((NEXMON_CHIP == CHIP_VER_BCM4358) || (NEXMON_CHIP == CHIP_VER_BCM4366c0))
            // csi format
            // for bcm4358:
            // sign(1bit) real(9bit) sign(1bit) imag(9bit) exp(5bit)
            // for bcm4366c0:
            // sign(1bit) real(12bit) sign(1bit) imag(12bit) exp(6bit)
            // forward as uint32 and unpack in user application
            udpfrm->csi_values[inserted_csi_values] = ucodecsifrm->csi[i];
#endif
            inserted_csi_values++;
        }

        missing_csi_frames --;

        // send csi udp to host
        if (missing_csi_frames == 0) {
#if NEXMON_CHIP == CHIP_VER_BCM4366c0
            memcpy(udpfrm->SrcMac, &(ucodecsifrm->src), sizeof(udpfrm->SrcMac));
            udpfrm->seqCnt = ucodecsifrm->seqcnt;
#else
            memcpy(udpfrm->SrcMac, &(ucodecsifrm->csi[tones]), sizeof(udpfrm->SrcMac)); // last csifrm also contains SrcMac
            udpfrm->seqCnt = *((uint16*)(&(ucodecsifrm->csi[tones]))+(sizeof(udpfrm->SrcMac)>>1)); // last csifrm also contains seqN
#endif
            p_csi->len = sizeof(struct csi_udp_frame) + inserted_csi_values * sizeof(uint32);
            skb_pull(p_csi, sizeof(struct ethernet_ip_udp_header));
            prepend_ethernet_ipv4_udp_header(p_csi);
            wl->dev->chained->funcs->xmit(wl->dev, wl->dev->chained, p_csi);
            p_csi = 0;
        }
        pkt_buf_free_skb(osh, p, 0);
        return;
    }

    wlc_rxhdr->tsf_l = tsf_l;
    wlc_phy_rssi_compute(wlc_hw->band->pi, wlc_rxhdr);
    wlc_recv(wlc_hw->wlc, p);
}

__attribute__((at(0x1AAFCC, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((naked))
void
process_frame_prehook_off0xC(void)
{
    asm(
        "mov r2, r4\n"              // 2 bytes: move wlc_hw pointer to r2
        "ldr r0, [sp,0xC]\n"        // 2 bytes: load reference to p into r0
        "bl process_frame_hook\n"   // 4 bytes
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
    );
}
__attribute__((at(0x1B2012, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((at(0x1C1C3E, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x2456E8, "", CHIP_VER_BCM4366c0, FW_VER_10_10_122_20)))
__attribute__((naked))
void
process_frame_prehook_off0x8(void)
{
    asm(
        "mov r2, r4\n"              // 2 bytes: move wlc_hw pointer to r2
        "ldr r0, [sp,0x8]\n"        // 2 bytes: load reference to p into r0
        "bl process_frame_hook\n"   // 4 bytes
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
        "nop\n"                     // 2 bytes: to overwrite existing instruction
    );
}


// Increase d11rxhdr size in initvals (found: below 0c 04 02 00)
__attribute__((at(0x1D4370, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x1F5768, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x1DF03C, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((at(0x28395C, "", CHIP_VER_BCM4366c0, FW_VER_10_10_122_20)))
GenericPatch1(initvals_rxhdr_len0, (RXE_RXHDR_LEN * 2));

__attribute__((at(0x1D4388, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x1F5778, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x1DF054, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((at(0x28396C, "", CHIP_VER_BCM4366c0, FW_VER_10_10_122_20)))
GenericPatch1(initvals_rxhdr_len1, (RXE_RXHDR_LEN * 2));

// Increase d11rxhdr size in wlc->hwrxhdr variable in wlc_bmac_attach
__attribute__((at(0x1F580C, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x210F56, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x1F67F8, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((at(0x2F4328, "", CHIP_VER_BCM4366c0, FW_VER_10_10_122_20)))
GenericPatch1(hwrxoff, (RXE_RXHDR_LEN * 2) + RXE_RXHDR_EXTRA);

// Increase d11rxhdr size in wlc->hwrxoff_pktget variable in wlc_bmac_attach
__attribute__((at(0x1F5812, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x210F60, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x1F6802, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((at(0x2F4332, "", CHIP_VER_BCM4366c0, FW_VER_10_10_122_20)))
GenericPatch1(hwrxoff_pktget, (RXE_RXHDR_LEN * 2) + RXE_RXHDR_EXTRA + 2);
