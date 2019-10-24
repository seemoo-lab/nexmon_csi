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

#define NEX_CAP_CSI_EXTRACT (1 << 3)
int capabilities = NEX_CAP_CSI_EXTRACT;

#if NEXMON_CHIP == CHIP_VER_BCM4366c0
extern unsigned char nonmuucode_compressed_bin[];
extern unsigned int nonmuucode_compressed_bin_len;
extern unsigned char nonmuucodex_compressed_bin[];
extern unsigned int nonmuucodex_compressed_bin_len;
extern unsigned char muucode_compressed_bin[];
extern unsigned int muucode_compressed_bin_len;
extern unsigned char muucodex_compressed_bin[];
extern unsigned int muucodex_compressed_bin_len;

__attribute__((at(WLC_NONMUUCODE_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_nonmuucode_write_compressed, wlc_ucode_write_compressed_args);

__attribute__((at(NONMUUCODESTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(nonmuucodestart_ptr, nonmuucode_compressed_bin);

__attribute__((at(NONMUUCODESIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(nonmuucodesize_ptr, &nonmuucode_compressed_bin_len);


__attribute__((at(WLC_NONMUUCODEX_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_nonmuucodex_write_compressed, wlc_ucodex_write_compressed_args);

__attribute__((at(NONMUUCODEXSTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(nonmuucodexstart_ptr, nonmuucodex_compressed_bin);

__attribute__((at(NONMUUCODEXSIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(nonmuucodexsize_ptr, &nonmuucodex_compressed_bin_len);


__attribute__((at(WLC_MUUCODE_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_muucode_write_compressed, wlc_ucode_write_compressed_args);

__attribute__((at(MUUCODESTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(muucodestart_ptr, muucode_compressed_bin);

__attribute__((at(MUUCODESIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(muucodesize_ptr, &muucode_compressed_bin_len);


__attribute__((at(WLC_MUUCODEX_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_muucodex_write_compressed, wlc_ucodex_write_compressed_args);

__attribute__((at(MUUCODEXSTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(muucodexstart_ptr, muucodex_compressed_bin);

__attribute__((at(MUUCODEXSIZE_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(muucodexsize_ptr, &muucodex_compressed_bin_len);

__attribute__((at(HNDRTE_RECLAIM_UCODES_END_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(hndrte_reclaim_ucodes_end, PATCHSTART);
#else
__attribute__((at(WLC_UCODE_WRITE_BL_HOOK_ADDR, "", CHIP_VER_ALL, FW_VER_ALL)))
BLPatch(wlc_ucode_write_compressed, wlc_ucode_write_compressed);
__attribute__((at(HNDRTE_RECLAIM_0_END_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(hndrte_reclaim_0_end, PATCHSTART);
#endif

extern unsigned char templateram_bin[];
#if 1+TEMPLATERAMSTART_PTR-1 != 0
__attribute__((at(TEMPLATERAMSTART_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(templateram_bin, templateram_bin);
#endif

