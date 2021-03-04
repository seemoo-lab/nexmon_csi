import time
import importlib
import config
from plotters.AmpPhaPlotter import Plotter # Amplitude and Phase plotter
decoder = importlib.import_module(f'decoders.{config.decoder}') # This is also an import

def string_is_int(s):
    '''
    Check if a string is an integer
    '''
    try:
        int(s)
        return True
    except ValueError:
        return False


if __name__ == "__main__":
    pcap_filename = input('Pcap file name: ')

    if '.pcap' not in pcap_filename:
        pcap_filename += '.pcap'
    pcap_filepath = '/'.join([config.pcap_fileroot, pcap_filename])

    try:
        samples = decoder.read_pcap(pcap_filepath)
    except FileNotFoundError:
        print(f'File {pcap_filepath} not found.')
        exit(-1)

    if config.plot_samples:
        plotter = Plotter(samples.bandwidth)

    while True:
        command = input('> ')

        if 'help' in command:
            print(config.help_str)
        
        elif 'exit' in command:
            break

        elif ('-' in command) and \
            string_is_int(command.split('-')[0]) and \
            string_is_int(command.split('-')[1]):

            start = int(command.split('-')[0])
            end = int(command.split('-')[1])

            for index in range(start, end+1):
                if config.print_samples:
                    samples.print(index)
                if config.plot_samples:
                    csi = samples.get_csi(
                        index,
                        config.remove_null_subcarriers,
                        config.remove_pilot_subcarriers
                    )
                    plotter.update(csi)

                time.sleep(config.plot_animation_delay_s)

        elif string_is_int(command):
            index = int(command)

            if config.print_samples:
                samples.print(index)
            if config.plot_samples:
                    csi = samples.get_csi(
                        index,
                        config.remove_null_subcarriers,
                        config.remove_pilot_subcarriers
                    )
                    plotter.update(csi)
        else:
            print('Unknown command. Type help.')