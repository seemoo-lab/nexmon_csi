# Basic Configuration
# ===================

# Chip - Uncomment *one* of the lines below
# ---------------------------------------

# chip = 'bcm4339'    # Nexus 5
chip = 'bcm43455c0' # Raspberry Pi 3B+ and 4B
# chip = 'bcm4358'    # Nexus 6P
# chip = 'bcm4366c0'  # Asus RT-AC86U


# Fileroot - Path to the directory with .pcap files
# -------------------------------------------------

pcap_fileroot = 'pcapfiles'


# Miscellaneous
# -------------

print_samples = True  # Set this to False to stop printing to terminal
plot_samples = True   # Set this to False to stop plotting
plot_animation_delay_s = 0.005 # Delay between csi plots.

# Setting this option to True removes Null Subcarriers.
# Null subcarriers have arbitrary values, and are used to
# help WiFi co-exist with other wireless technologies.
# https://www.oreilly.com/library/view/80211ac-a-survival/9781449357702/ch02.html
remove_null_subcarriers = True

# Pilot subcarriers are used to control the WiFi link,
# while other subcarriers carry data. I found pilot
# subcarriers sometimes have inconsistent CSI compared
# to the rest, and so I remove them. You may not necessarily
# face such issues.
remove_pilot_subcarriers = False




# Advanced configuration. You don't need to change this
# =====================================================

# Decoder
# -------

if chip in ['bcm4339', 'bcm43455c0']:
    # The Real and Imaginary values of CSI
    # are interleaved for these chips
    decoder = 'interleaved'
elif chip in ['bcm4358', 'bcm4366c0']:
    # Right now, there is no support for
    # CSI encoded as floating point values.
    # If this is important to you, please raise
    # an issue.
    decoder = 'floatingpoint'
else:
    decoder = chip


help_str = f'''
CSI Reader
==========

A simple Python utility to
explore nexmon_csi CSI samples.

Change the config.py to match your
WiFi chip and bandwidth. Current chip
is {chip}.

To explore a sample, type it's
index from the pcap file. Indexes
start from 0.

To plot a range of samples as animation,
type their indexes separated by '-'.

Type 'help' to see this message again.

Type 'exit' to stop this program.
'''