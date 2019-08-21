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
#include <channels.h>
#include <patcher.h>
#include <wrapper.h>
#include <channels.h>

// Nop the following call to keep user tx power targets
//    Choose least of user and now combined regulatory/hw targets
//    ppr_compare_min(tx_pwr_target, srom_max_txpwr);
__attribute__((at(0x1C50B8, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x1D3212, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
GenericPatch4(nop_ppr_compare_min, 0);

// This allows 80 MHz channels in the 2 GHz band
int
wf_chspec_malformed_hook(unsigned short chanspec)
{
    return 0;
}

__attribute__((at(0x13778, "flashpatch", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x4F64, "flashpatch", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
BPatch(wf_chspec_malformed_hook, wf_chspec_malformed_hook);

__attribute__((at(0x1fa040, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((at(0x1E4C1C, "", CHIP_VER_BCM4358, FW_VER_7_112_300_14)))
__attribute__((aligned(1)))
unsigned char _locale_channels[] = {
#if (NEXMON_CHIP == CHIP_VER_BCM4339)
    0x00, 
    0x01, 0xFF, 
    0x01, 0xFF, 
    0x02, 0xFF, 0xFF, 
    0x04, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x02, 0xFF, 0xFF, 
    0x02, 0xFF, 0xFF, 
    0x01, 0xFF, 
    0x04, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x03, 0xFF, 0xFF, 0xFF, 
    0x04, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x04, 0xFF, 0xFF, 0xFF, 0xFF, 
    0x03, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF, 
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF, 
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF, 
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF, 
    0x01, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF, 
    0x01, 0xFF, 
    0x01, 0xFF
#elif (NEXMON_CHIP == CHIP_VER_BCM4358)
    0x00,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x02, 0xFF, 0xFF,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x04, 0xFF, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x02, 0xFF, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x01, 0xFF,
    0x03, 0xFF, 0xFF, 0xFF,
    0x02, 0xFF, 0xFF
#endif
};

__attribute__((at(0x1fa48c, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((aligned(1)))
unsigned char _valid_channel_2g_20m[] = { 0x01, 0x0f, 0x01, 0x00 };

__attribute__((at(0x1fb08e, "", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
__attribute__((aligned(1)))
unsigned char _valid_channel_5g_20m[] = { 0x22, 0x2e, 0x04, 0x24 };

unsigned short additional_valid_chanspecs[] = {
    CH80MHZ_CHSPEC(6, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(7),
    CH40MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(7),
    CH40MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_U),
    CH80MHZ_CHSPEC(7, WL_CHANSPEC_CTL_SB_U),
    CH20MHZ_CHSPEC(9),
    CH40MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(9, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(13),
    CH40MHZ_CHSPEC(13, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(13, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(106),
    CH40MHZ_CHSPEC(106, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(106, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(116),
    CH40MHZ_CHSPEC(118, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(138, WL_CHANSPEC_CTL_SB_L),
    CH80MHZ_CHSPEC(122, WL_CHANSPEC_CTL_SB_L),
    CH20MHZ_CHSPEC(120),
#if (NEXMON_CHIP == CHIP_VER_BCM43455c0)
    0xe02a, // 36/80
    0xe29b, // 157/80
#endif
};

int
wlc_valid_chanspec_ext_hook(void *wlc_cm, unsigned short chanspec, int dualband)
{
	int valid = wlc_valid_chanspec_ext(wlc_cm, chanspec, dualband);
	int i;

	if (!valid && dualband == 1)
		for (i = 0; i < sizeof(additional_valid_chanspecs)/sizeof(additional_valid_chanspecs[0]); i++)
			valid |= additional_valid_chanspecs[i] == chanspec;
		
    return valid;
}

__attribute__((at(0x58eb6, "flashpatch", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
__attribute__((at(0x5BA28, "flashpatch", CHIP_VER_BCM4339, FW_VER_6_37_32_RC23_34_43_r639704)))
BPatch(wlc_valid_chanspec_ext, wlc_valid_chanspec_ext_hook)
