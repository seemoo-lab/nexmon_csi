'''
Interleaved
===========

Fast and efficient methods to extract
Interleaved CSI samples in PCAP files.

~230k samples per second.

Suitable for bcm43455c0 and bcm4339 chips.

Requires Numpy.

Usage
-----

import decoders.interleaved as decoder

samples = decoder.read_pcap('path_to_pcap_file')

Bandwidth is inferred from the pcap file, but
can also be explicitly set:
samples = decoder.read_pcap('path_to_pcap_file', bandwidth=40)
'''

__all__ = [
    'read_pcap'
]

import os
import numpy as np

# Indexes of Null and Pilot OFDM subcarriers
# https://www.oreilly.com/library/view/80211ac-a-survival/9781449357702/ch02.html
nulls = {
    20: [x+32 for x in [
        -32, -31, -30, -29,
              31,  30,  29,  0
    ]],

    40: [x+64 for x in [
        -64, -63, -62, -61, -60, -59, -1, 
              63,  62,  61,  60,  59,  1,  0
    ]],

    80: [x+128 for x in [
        -128, -127, -126, -125, -124, -123, -1,
               127,  126,  125,  124,  123,  1,  0
    ]],

    160: [x+256 for x in [
        -256, -255, -254, -253, -252, -251, -129, -128, -127, -5, -4, -3, -2, -1,
               255,  254,  253,  252,  251,  129,  128,  127,  5,  4,  3,  3,  1,  0 
    ]]
}

pilots = {
    20: [x+32 for x in [
        -21, -7,
         21,  7
    ]],

    40: [x+64 for x in [
        -53, -25, -11, 
         53,  25,  11
    ]],

    80: [x+128 for x in [
        -103, -75, -39, -11,
         103,  75,  39,  11
    ]],

    160: [x+256 for x in [
        -231, -203, -167, -139, -117, -89, -53, -25,
         231,  203,  167,  139,  117,  89,  53,  25
    ]]
}

class SampleSet(object):
    '''
        A helper class to contain data read
        from pcap files.
    '''
    def __init__(self, samples, bandwidth):
        self.mac, self.seq, self.css, self.csi = samples

        self.nsamples = self.csi.shape[0]
        self.bandwidth = bandwidth

    def get_mac(self, index):
        return self.mac[index*6: (index+1)*6]

    def get_seq(self, index):
        sc = int.from_bytes( #uint16: SC
            self.seq[index*2: (index+1)*2],
            byteorder = 'little',
            signed = False
        )
        fn = sc % 16 # Fragment Number
        sc = int((sc - fn)/16) # Sequence Number

        return (sc, fn)
    
    def get_css(self, index):
        return self.css[index*2: (index+1)*2]

    def get_csi(self, index, rm_nulls=False, rm_pilots=False):
        csi = self.csi[index].copy()
        if rm_nulls:
            csi[nulls[self.bandwidth]] = 0
        if rm_pilots:
            csi[pilots[self.bandwidth]] = 0

        return csi
   
    def print(self, index):
        # Mac ID
        macid = self.get_mac(index).hex()
        macid = ':'.join([macid[i:i+2] for i in range(0, len(macid), 2)])

        # Sequence control
        sc, fn = self.get_seq(index)

        # Core and Spatial Stream
        css = self.get_css(index).hex()

        print(
            f'''
Sample #{index}
---------------
Source Mac ID: {macid}
Sequence: {sc}.{fn}
Core and Spatial Stream: 0x{css}
            '''
        )


def __find_bandwidth(incl_len):
    '''
        Determines bandwidth
        from length of packets.
        
        incl_len is the 4 bytes
        indicating the length of the
        packet in packet header
        https://wiki.wireshark.org/Development/LibpcapFileFormat/

        This function is immune to small
        changes in packet lengths.
    '''

    pkt_len = int.from_bytes(
        incl_len,
        byteorder='little',
        signed=False
    )

    # The number of bytes before we
    # have CSI data is 60. By adding
    # 128-60 to frame_len, bandwidth
    # will be calculated correctly even
    # if frame_len changes +/- 128
    # Some packets have zero padding.
    # 128 = 20 * 3.2 * 4
    nbytes_before_csi = 60
    pkt_len += (128 - nbytes_before_csi)

    bandwidth = 20 * int(
        pkt_len // (20 * 3.2 * 4)
    )

    return bandwidth



def __find_nsamples_max(pcap_filesize, nsub):
    '''
        Returns an estimate for the maximum possible number
        of samples in the pcap file.

        The size of the pcap file is divided by the size of
        a packet to calculate the number of samples. However,
        some packets have a padding of a few bytes, so the value
        returned is slightly higher than the actual number of
        samples in the pcap file.
    '''

    # PCAP global header is 24 bytes
    # PCAP packet header is 12 bytes
    # Ethernet + IP + UDP headers are 46 bytes
    # Nexmon metadata is 18 bytes
    # CSI is nsub*4 bytes long
    #
    # So each packet is 12 + 46 + 18 + nsub*4 bytes long
    nsamples_max = int(
        (pcap_filesize - 24) / (
            12 + 46 + 18 + (nsub*4)
        )
    )

    return nsamples_max

def read_pcap(pcap_filepath, bandwidth=0, nsamples_max=0):
    '''
        Reads CSI samples from
        a pcap file. A SampleSet
        object is returned.

        Bandwidth and maximum samples
        are inferred from the pcap file by
        default, but you can also set them explicitly.
    '''

    pcap_filesize = os.stat(pcap_filepath).st_size
    with open(pcap_filepath, 'rb') as pcapfile:
        fc = pcapfile.read()
    
    if bandwidth == 0:
        bandwidth = __find_bandwidth(
            # 32-36 is where the incl_len
            # bytes for the first frame are
            # located.
            # https://wiki.wireshark.org/Development/LibpcapFileFormat/
            fc[32:36]
        )
    # Number of OFDM sub-carriers
    nsub = int(bandwidth * 3.2)

    if nsamples_max == 0:
        nsamples_max = __find_nsamples_max(pcap_filesize, nsub)

    # Preallocating memory
    mac = bytearray(nsamples_max * 6)
    seq = bytearray(nsamples_max * 2)
    css = bytearray(nsamples_max * 2)
    csi = bytearray(nsamples_max * nsub * 4)

    # Pointer to current location in file.
    # This is faster than using file.tell()
    # =24 to skip pcap global header
    ptr = 24

    nsamples = 0
    while ptr < pcap_filesize:
        # Read frame header
        # Skip over Eth, IP, UDP
        ptr += 8
        frame_len = int.from_bytes(
            fc[ptr: ptr+4],
            byteorder='little',
            signed=False
        )
        ptr += 50

        # 4 bytes: Magic Bytes               @ 0 - 4
        # 6 bytes: Source Mac ID             @ 4 - 10
        # 2 bytes: Sequence Number           @ 10 - 12
        # 2 bytes: Core and Spatial Stream   @ 12 - 14
        # 2 bytes: ChanSpec                  @ 14 - 16
        # 2 bytes: Chip Version              @ 16 - 18
        # nsub*4 bytes: CSI Data             @ 18 - 18 + nsub*4

        mac[nsamples*6: (nsamples+1)*6] = fc[ptr+4: ptr+10]
        seq[nsamples*2: (nsamples+1)*2] = fc[ptr+10: ptr+12]
        css[nsamples*2: (nsamples+1)*2] = fc[ptr+12: ptr+14]
        csi[nsamples*(nsub*4): (nsamples+1)*(nsub*4)] = fc[ptr+18: ptr+18 + nsub*4]

        ptr += (frame_len - 42)
        nsamples += 1

    # Convert CSI bytes to numpy array
    csi_np = np.frombuffer(
        csi,
        dtype = np.int16,
        count = nsub * 2 * nsamples
    )

    # Cast numpy 1-d array to matrix
    csi_np = csi_np.reshape((nsamples, nsub * 2))

    # Convert csi into complex numbers
    csi_cmplx = np.fft.fftshift(
            csi_np[:nsamples, ::2] + 1.j * csi_np[:nsamples, 1::2], axes=(1,)
    )

    return SampleSet(
        (mac,
        seq,
        css,
        csi_cmplx),
        bandwidth
    )

if __name__ == "__main__":
    samples = read_pcap('pcap_files/output-40.pcap')
