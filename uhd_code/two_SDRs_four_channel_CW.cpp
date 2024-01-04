/* 
Version: 1
By: Tedi Qafko
Date: 12/19/2023 

Description: 

Two software defined radios are connected in the network and accesses
via UHD whose channels are set to transmit continuous sine waves.

Examples editted from: https://github.com/EttusResearch/uhd

*/
#include "wavetable.hpp"
#include <uhd/exception.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/utils/thread.hpp>
#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

namespace po = boost::program_options;

/***********************************************************************
 * Signal handlers
 **********************************************************************/
static bool stop_signal_called = false;
void sig_int_handler(int)
{
    stop_signal_called = true;
}

int UHD_SAFE_MAIN(int argc, char *argv[]) {
    
    // variables to be set by po
    std::string args, wave_type, ant, subdev, ref, pps, otw, channel_list;
    uint64_t total_num_samps;
    size_t spb;
    double rate, freq1, freq2, freq3, freq4, gain, power, wave_freq, bw, lo_offset;
    float ampl;
    std::vector<unsigned long> channel; channel.push_back(0); channel.push_back(1); channel.push_back(2); channel.push_back(3); 

    // setup the program options
    po::options_description desc("Allowed options");
    
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value("addr0=192.168.10.2,addr1=192.168.10.4"), "single uhd device address args")
        ("spb", po::value<size_t>(&spb)->default_value(0), "samples per buffer, 0 for default")
        ("nsamps", po::value<uint64_t>(&total_num_samps)->default_value(0), "total number of samples to transmit")
        ("rate", po::value<double>(&rate)->default_value(1e6), "rate of outgoing samples")
        ("freq1", po::value<double>(&freq1)->default_value(1000e6), "RF center frequency in Hz")
        ("freq2", po::value<double>(&freq2)->default_value(1100e6), "RF center frequency in Hz")
        ("freq3", po::value<double>(&freq3)->default_value(1200e6), "RF center frequency in Hz")
        ("freq4", po::value<double>(&freq4)->default_value(1300e6), "RF center frequency in Hz")
        ("lo-offset", po::value<double>(&lo_offset)->default_value(0.0),
            "Offset for frontend LO in Hz (optional)")
        ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of the waveform [0 to 0.7]")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        ("power", po::value<double>(&power), "Transmit power (if USRP supports it)")
        ("ant", po::value<std::string>(&ant), "antenna selection")
        ("bw", po::value<double>(&bw), "analog frontend filter bandwidth in Hz")
        ("wave-type", po::value<std::string>(&wave_type)->default_value("CONST"), "waveform type (CONST, SQUARE, RAMP, SINE)")
        ("wave-freq", po::value<double>(&wave_freq)->default_value(0), "waveform frequency in Hz")
        ("ref", po::value<std::string>(&ref), "clock reference (internal, external, mimo, gpsdo)")
        ("pps", po::value<std::string>(&pps), "PPS source (internal, external, mimo, gpsdo)")
        ("otw", po::value<std::string>(&otw)->default_value("sc16"), "specify the over-the-wire sample mode")
        ("channels", po::value<std::string>(&channel_list)->default_value("0"), "which channels to use (specify \"0\", \"1\", \"0,1\", etc)")
        ("int-n", "tune USRP with integer-N tuning")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    // set sample rate
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate / 1e6) << std::endl;
    usrp->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate() / 1e6) << std::endl << std::endl;

    const wave_table_class wave_table("CONST", float(0.3));
    const size_t step = std::lround(wave_freq / usrp->get_tx_rate() * wave_table_len);
    size_t index      = 0;
    // std::this_thread::sleep_for(std::chrono::seconds(1)); // allow for some setup time
    
    // for the const wave, set the wave freq for small samples per period
    if (wave_freq == 0) {
        if (wave_type == "CONST") {
            wave_freq = usrp->get_tx_rate() / 2;
        } else {
            throw std::runtime_error(
                "wave freq cannot be 0 with wave type other than CONST");
        }
    }
    // Adds frequencies to a vector
    std::vector<double> freqs;  
    freqs.push_back(freq1); 
    freqs.push_back(freq2);
    freqs.push_back(freq3); 
    freqs.push_back(freq4);

    // Sets frequencies of channels via the vector of the frequencies entered from user.
    for (size_t ch = 0; ch < usrp->get_tx_num_channels(); ch++) {
        // Sets freq and gain correct in both x310 channels
        // set freq
        std::cout << boost::format("Setting TX Freq: %f MHz...") % (freqs[ch] / 1e6) << std::endl;
        uhd::tune_request_t tune_request(freqs[ch], lo_offset);
        usrp->set_tx_freq(tune_request, channel[ch]);
        std::cout << boost::format("Actual TX Freq: %f MHz...") % (usrp->get_tx_freq(channel[ch]) / 1e6)
                  << std::endl
                  << std::endl;
        // set the rf gain
        std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
        usrp->set_tx_gain(gain, channel[ch]);
        std::cout << boost::format("Actual TX Gain: %f dB...")% usrp->get_tx_gain(channel[ch])
                      << std::endl
                      << std::endl;
        // std::this_thread::sleep_for(std::chrono::seconds(1)); // allow for some setup time
    }
    // error when the waveform is not possible to generate
    if (std::abs(wave_freq) > usrp->get_tx_rate() / 2) {
        throw std::runtime_error("wave freq out of Nyquist zone");
    }
    if (usrp->get_tx_rate() / std::abs(wave_freq) > wave_table_len / 2) {
        throw std::runtime_error("wave freq too small for table");
    }

    uhd::stream_args_t stream_args("fc32", otw);
    stream_args.channels = channel;
    uhd::tx_streamer::sptr tx_stream = usrp->get_tx_stream(stream_args);

    if (spb == 0) {
        spb = tx_stream->get_max_num_samps() * 10;
    }
    std::vector<std::complex<float>> buff(spb);
    std::vector<std::complex<float>*> buffs(usrp->get_tx_num_channels(), &buff.front());

    // pre-fill the buffer with the waveform
    for (size_t n = 0; n < buff.size(); n++) {
        buff[n] = wave_table(index += step);
    }

    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
    usrp->set_time_now(0.0);
    std::signal(SIGINT, &sig_int_handler);
    std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
    
    uhd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst   = false;
    md.has_time_spec  = true;
    md.time_spec      = usrp->get_time_now() + uhd::time_spec_t(0.1);
    
    // send data until the signal handler gets called
    // or if we accumulate the number of samples specified (unless it's 0)
    uint64_t num_acc_samps = 0;
    while (true) {
        // Break on the end of duration or CTRL-C
        if (stop_signal_called) {
            break;
        }
        
        // Break when we've received nsamps
        if (total_num_samps > 0 and num_acc_samps >= total_num_samps) {
            break;
        }
        
        // send the entire contents of the buffer
        num_acc_samps += tx_stream->send(buffs, buff.size(), md);
        
        // fill the buffer with the waveform
        for (size_t n = 0; n < buff.size(); n++) {
            buff[n] = wave_table(index += step);
        }
        
        md.start_of_burst = false;
        md.has_time_spec  = false;
    }
    
    // send a mini EOB packet
    md.end_of_burst = true;
    tx_stream->send("", 0, md);

    // finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;
    return EXIT_SUCCESS;
}
