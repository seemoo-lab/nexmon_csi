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
#include <patcher.h>

#if ((NEXMON_CHIP == CHIP_VER_BCM43455c0) && (NEXMON_FW_VERSION == FW_VER_7_45_189))
char version[] = "7.45.189 (nexmon.org/csi: " GIT_VERSION "-" BUILD_NUMBER ")\n";
char date[] = __DATE__;
char time[] = __TIME__;
__attribute__((at(0x1A6DD8, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
GenericPatch4(date_patch, date);
__attribute__((at(0x1A6DC8, "", CHIP_VER_BCM43455c0, FW_VER_7_45_189)))
GenericPatch4(time_patch, time);
#elif ((NEXMON_CHIP == CHIP_VER_BCM4339) && (NEXMON_FW_VERSION == FW_VER_6_37_32_RC23_34_43_r639704))
char version[] = "6.37.32 (nexmon.org/csi: " GIT_VERSION "-" BUILD_NUMBER ")\n";
#elif ((NEXMON_CHIP == CHIP_VER_BCM4358) && (NEXMON_FW_VERSION == FW_VER_7_112_300_14))
char version[] = "7.112.300.14 (nexmon.org/csi: " GIT_VERSION "-" BUILD_NUMBER ")\n";
#elif ((NEXMON_CHIP == CHIP_VER_BCM4366c0) && (NEXMON_FW_VERSION == FW_VER_10_10_122_20))
char version[] = "10.10.122.20 (nexmon.org/csi: " GIT_VERSION "-" BUILD_NUMBER ")\n";
#endif
__attribute__((at(VERSION_PTR, "", CHIP_VER_ALL, FW_VER_ALL)))
GenericPatch4(version_patch, version);
