#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
/*
#include <sys/types.h>
#include <sys/socket.h>
#include <net/ethernet.h>
*/
#include "bcmwifi_channels.h"

void st16le (uint16_t value, uint16_t *addr)
{
    uint8_t *_addr = (uint8_t *) addr;
    _addr[0] = value & 0xff;
    _addr[1] = (value >> 8) & 0xff;
}

void st16be (uint16_t value, uint16_t *addr)
{
    uint8_t *_addr = (uint8_t *) addr;
    _addr[1] = value & 0xff;
    _addr[0] = (value >> 8) & 0xff;
}

#define VALID_CORE_MASK 0xf
#define VALID_NSS_MASK 0xf
#define MAX_MAC_ADDRESS 4
#define	DEFAULT_DELAY_US 50

int countbit (uint32_t val)
{
    int counter = 0;
    while (val != 0) {
        if (val & 1) counter ++;
        val = val >> 1;
    }
    return counter;
}

struct csi_params {
        uint16_t chanspec;            // chanspec to tune to
        uint8_t  csi_collect;         // trigger csi collect (1: on, 0: off)
        uint8_t  core_nss_mask;       // coremask and spatialstreammask./iperf -u -c 192.168.2.59 -i 1 -b 10M
        uint8_t  use_pkt_filter;      // trigger first packet byte filter (1: on, 0: off)
        uint8_t  first_pkt_byte;      // first packet byte to filter for
        uint16_t n_mac_addr;          // number of mac addresses to filter for (0: off, 1-4: on,use src_mac_0-3)
        uint16_t cmp_src_mac_0_0;     // filter src mac 0
        uint16_t cmp_src_mac_0_1;
        uint16_t cmp_src_mac_0_2;
        uint16_t cmp_src_mac_1_0;     // filter src mac 1
        uint16_t cmp_src_mac_1_1;
        uint16_t cmp_src_mac_1_2;
        uint16_t cmp_src_mac_2_0;     // filter src mac 2
        uint16_t cmp_src_mac_2_1;
        uint16_t cmp_src_mac_2_2;
        uint16_t cmp_src_mac_3_0;     // filter src mac 3
        uint16_t cmp_src_mac_3_1;
        uint16_t cmp_src_mac_3_2;
        uint16_t delay;
};

void usage () 
{
    char *usage_str =
        "Usage: makecsiparams [OPTION...]\n"
        "\n"
        "   -h           print this message\n"
        "   -e on/off    enable/disable CSI collection (0 = disable, default is 1)\n"
        "   -c chanspec  Channel specification <channel>/<bandwidth>\n"
        "   -C coremask  bitmask with cores where to activate capture\n"
        "                (e.g., 0x5 = 0b0101 set core 0 and 2)\n"
        "   -N nssmask   bitmask with spatial streams to capture\n"
        "                (e.g., 0x7 = 0b0111 capture first 3 ss)\n"
        "   -m addr      filter on this source mac address (up to four, comma separated)\n"
        "   -b byte      filter frames starting with byte\n"
        "   -d delay     delay in us after each CSI operation\n"
        "                (really needed for 3x4, 4x3 and 4x4 configurations,\n"
        "                without it is enforced automatically)\n"
        "   -r           generate raw output (no base64)\n"
        "";
    fprintf (stdout, "%s\n", usage_str);
}

char base64(uint32_t val)
{
    char ch = '/';
    if (val < 26) 
        ch = 'A' + val;
    else if (val < 52)
        ch = 'a' + (val - 26);
    else if (val < 62)
        ch = '0' + (val - 52);
    else if (val == 62)
        ch = '+';
    return ch;
}

int main (int argc, char *argv[]) {
    struct csi_params p;
    memset (&p, 0, sizeof(p));
    int c;
    int retval = 0;

    int enable = 1;
    int coremask = 0;
    int nssmask = 0;
    int force_delay = 0;
    int delay;
    int nmac = 0;
    int byte = 0;
    //int chanspec = 0;
    uint16_t chanspec = 0;
    char *endptr = NULL;
    int doraw = 0;

    if(argc < 2) {
        usage ();
        goto finish;
    }

    while ((c = getopt(argc, argv, "hre:m:b:c:C:N:d:")) != EOF) {
        switch (c) {
            case 'h':
                usage ();
                goto finish;

            case 'r':
                doraw = 1;
                break;

            case 'e':
                enable = (int) strtol (optarg, &endptr, 0);
                if (*endptr != 0) {
                    fprintf (stderr, "Invalid enable value\n");
                    goto finish_error;
                }
                break;

            case 'm':
                {
                    char *split;
                    split = strtok(optarg, ",");
                    struct ether_addr *ea;
                    while (split != NULL) {
                        if (nmac >= MAX_MAC_ADDRESS) {
                            fprintf (stderr, "Only %d mac addresses can be given\n", MAX_MAC_ADDRESS);
                            goto finish_error;
                        }

                        ea = ether_aton (split);
                        if (ea == NULL) {
                            fprintf (stderr, "Invalid mac address\n");
                            goto finish_error; 
                        }

                        // assuming mac addresses are packed
                        memcpy (((void *) &p.cmp_src_mac_0_0) + nmac * 6,
                                ea->ether_addr_octet /* ea->octet */, 6);
                        nmac ++;
                        st16le(nmac, &p.n_mac_addr);
                        split = strtok(NULL, ",");
                    }
                    break;
                }

            case 'b':
                byte = strtol (optarg, &endptr, 0);
                if (*endptr != 0 || byte < 0 || byte > 255) {
                    fprintf (stderr, "Invalid byte filter\n");
                    goto finish_error;
                }
                p.use_pkt_filter = 0x01;
                p.first_pkt_byte = (uint8_t) byte;
                break;

            case 'c':
                //chanspec = strtol (optarg, &endptr, 0);
                //if (*endptr != 0) {
                //    fprintf (stderr, "Invalid chanspec string\n");
                //    goto finish_error;
                //}
                //
                //st16le ((uint16_t) chanspec, &p.chanspec);
                chanspec = wf_chspec_aton(optarg);
                if (chanspec == 0) {
                    fprintf (stderr, "Invalid chanspec\n");
                    goto finish_error;
                }
                p.chanspec = chanspec;
                break;

            case 'C':
                coremask = (int) strtol (optarg, &endptr, 0);
                if (*endptr != 0 || (coremask & ~VALID_CORE_MASK)) {
                    fprintf (stderr, "Invalid core mask\n");
                    goto finish_error;
                }
                p.core_nss_mask = (p.core_nss_mask & 0xf0) | coremask;
                //fprintf (stdout, "mask core: %04x\n", p.core_nss_mask);
                break;

            case 'N':
                nssmask = (int) strtol (optarg, &endptr, 0);
                if (*endptr != 0 || (nssmask & ~VALID_NSS_MASK)) {
                    fprintf (stderr, "Invalid nss mask\n");
                    goto finish_error;
                }
                p.core_nss_mask = (p.core_nss_mask & 0x0f) | (nssmask << 4);
                //fprintf (stdout, "mask nss: %04x\n", p.core_nss_mask);
                break;

            case 'd':
                delay = (int) strtol (optarg, &endptr, 0);
                if (*endptr != 0 || delay < 0) {
                    fprintf (stderr, "Invalid delay\n");
                    goto finish_error;
                }
                force_delay = 1;
                st16le(delay, &p.delay);
                break;

            default:
                fprintf (stderr, "Invalid option\n"); 
                goto finish_error;
        }
    }

    p.csi_collect = enable;

    if (enable != 0) {
        if (chanspec == 0) {
            fprintf (stderr, "No channel given\n");
            goto finish_error;
        }
    
        if (nssmask == 0 || coremask == 0) {
            fprintf (stderr, "No nssmask and/or coremask given\n");
            goto finish_error;
        }
    }

    if (force_delay == 0) {
        int csi_to_capture = countbit (nssmask) * countbit (coremask);
        if (csi_to_capture >= 12) {
            delay = DEFAULT_DELAY_US;
            st16le(delay, &p.delay);
        }
    }

    uint8_t *base64p = (uint8_t *) &p;

    if (doraw) {
        fwrite (base64p, sizeof(p), 1, stdout);
        goto finish;
    }

    int jj, kk;
    int len = sizeof(p);
    for (jj = 0; jj < len; jj += 3) {
        uint32_t val = 0;
        int parts = (len - jj);
        if (parts >= 3) {
            parts = 3;
            val = base64p[jj + 0] << 16 | base64p[jj + 1] << 8 | base64p[jj + 2];
        }
        else {
            for (kk = 0; kk < parts; kk ++) {
                val |= (base64p[jj + kk] << (8 * (2 - kk)));
            }
        }
        int bits = parts * 8;
        for (kk = 0; kk < 24; kk += 6) {
            uint32_t ch = (val >> 18) & 0x3f;
            if (kk > bits) fprintf (stdout, "=");
            else fprintf (stdout, "%c", base64(ch));
            val = val << 6;
        }
    }
    fprintf (stdout, "\n");
    goto finish;

  finish_error:
    retval = 1;

  finish:
    return 0;
}
// printf ${string} | xxd -r -p  | base64
