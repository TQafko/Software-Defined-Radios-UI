/*
Version: 1
By: Tedi Qafko
Date: 12/19/2023

Description: 

(Threaded) A Software defined radio is configured to output four files from binary data into 
four of its transmit channels. The signals are meant to be generated through other software
like MATLAB, python, etc.

Examples editted from: https://github.com/EttusResearch/uhd

*/

#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <complex>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>

namespace po = boost::program_options;

static bool stop_signal_called = false;
static bool repeat = false;
static double delay = 0;
void sig_int_handler(int)
{
    stop_signal_called = true;
}

template <typename samp_type>
void send_from_file(uhd::tx_streamer::sptr tx_stream, const std::string &file, size_t samps_per_buff)
{
    while(not stop_signal_called){
        uhd::tx_metadata_t md;
        md.start_of_burst = false;
        md.end_of_burst   = false;
        std::vector<samp_type> buff(samps_per_buff);
        std::ifstream infile(file.c_str(), std::ifstream::binary);

        // loop until the entire file has been read

        while (not md.end_of_burst and not stop_signal_called) {
            infile.read((char*)&buff.front(), buff.size() * sizeof(samp_type));
            size_t num_tx_samps = size_t(infile.gcount() / sizeof(samp_type));

            md.end_of_burst = infile.eof();

            const size_t samples_sent = tx_stream->send(&buff.front(), num_tx_samps, md);
            if (samples_sent != num_tx_samps) {
                UHD_LOG_ERROR("TX-STREAM",
                    "The tx_stream timed out sending " << num_tx_samps << " samples ("
                                                    << samples_sent << " sent).");
                return;
            }
        }

        infile.close();
        if (repeat and delay > 0.0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(int64_t(delay * 1000)));
        }
    } 
}

int UHD_SAFE_MAIN(int argc, char* argv[])
{
    // variables to be set by po
    std::string args, file1, file2, file3, file4, type, ant, subdev, ref, wirefmt;
    size_t spb;
    double rate, freq1, freq2, freq3, freq4, gain, bw, lo_offset;
    std::vector<unsigned long> channel; channel.push_back(0); channel.push_back(1);channel.push_back(2);channel.push_back(3);
    
    // setup the program options
    po::options_description desc("Allowed options");
    // clang-format off
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value("addr0=192.168.10.2,addr1=192.168.10.4"), "multi uhd device address args")
        ("file1", po::value<std::string>(&file1)->default_value("usrp_samples.dat"), "name of the file to read binary samples from")
        ("file2", po::value<std::string>(&file2)->default_value("usrp_samples.dat"), "name of the file to read binary samples from")
        ("file3", po::value<std::string>(&file3)->default_value("usrp_samples.dat"), "name of the file to read binary samples from")
        ("file4", po::value<std::string>(&file4)->default_value("usrp_samples.dat"), "name of the file to read binary samples from")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type: double, float, or short")
        ("spb", po::value<size_t>(&spb)->default_value(30000), "samples per buffer")
        ("rate", po::value<double>(&rate), "rate of outgoing samples")
        ("freq1", po::value<double>(&freq1)->default_value(1000e6), "RF center frequency in Hz")
        ("freq2", po::value<double>(&freq2)->default_value(1100e6), "RF center frequency in Hz")
        ("freq3", po::value<double>(&freq3)->default_value(1200e6), "RF center frequency in Hz")
        ("freq4", po::value<double>(&freq4)->default_value(1300e6), "RF center frequency in Hz")
        ("lo-offset", po::value<double>(&lo_offset)->default_value(0.0),
            "Offset for frontend LO in Hz (optional)")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        ("ant", po::value<std::string>(&ant), "antenna selection")
        ("subdev", po::value<std::string>(&subdev), "subdevice specification")
        ("bw", po::value<double>(&bw), "analog frontend filter bandwidth in Hz")
        ("ref", po::value<std::string>(&ref), "clock reference (internal, external, mimo, gpsdo)")
        ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8 or sc16)")
        ("delay", po::value<double>(&delay)->default_value(0.0), "specify a delay between repeated transmission of file (in seconds)")
        // ("channel", po::value<std::string>(&channel)->default_value("0"), "which channel to use")
        ("repeat", "repeatedly transmit file")
        ("int-n", "tune USRP with integer-n tuning")
    ;
    // clang-format on
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // print the help message
    if (vm.count("help")) {
        std::cout << boost::format("UHD TX samples from file %s") % desc << std::endl;
        return ~0;
    }

    repeat = vm.count("repeat") > 0;

    // create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args
              << std::endl;
    uhd::usrp::multi_usrp::sptr usrp = uhd::usrp::multi_usrp::make(args);

    // Lock mboard clocks
    if (vm.count("ref")) {
        usrp->set_clock_source(ref);
    }

    // always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev"))
        usrp->set_tx_subdev_spec(subdev);

    std::cout << boost::format("Using Device: %s") % usrp->get_pp_string() << std::endl;

    // set the sample rate
    if (not vm.count("rate")) {
        std::cerr << "Please specify the sample rate with --rate" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate / 1e6) << std::endl;
    usrp->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (usrp->get_tx_rate() / 1e6)
              << std::endl
              << std::endl;

    std::vector<double> freqs;  freqs.push_back(freq1); freqs.push_back(freq2);freqs.push_back(freq3);freqs.push_back(freq4);
    for (size_t ch = 0; ch < usrp->get_tx_num_channels(); ch++) {
        // Sets freq and gain correct in both x310 channels
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
   }
    // allow for some setup time:
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // set sigint if user wants to receive
    if (repeat) {
        std::signal(SIGINT, &sig_int_handler);
        std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
    }

    // create a transmit streamer
    std::string cpu_format;
    std::vector<size_t> channel_nums;
    if (type == "double")
        cpu_format = "fc64";
    else if (type == "float")
        cpu_format = "fc32";
    else if (type == "short")
        cpu_format = "sc16";
    uhd::stream_args_t stream_args1(cpu_format, wirefmt);
    stream_args1.channels = {0};
    uhd::tx_streamer::sptr tx_stream1 = usrp->get_tx_stream(stream_args1);

    uhd::stream_args_t stream_args2(cpu_format, wirefmt);
    stream_args2.channels = {1};
    uhd::tx_streamer::sptr tx_stream2 = usrp->get_tx_stream(stream_args2);

    uhd::stream_args_t stream_args3(cpu_format, wirefmt);
    stream_args3.channels = {2};
    uhd::tx_streamer::sptr tx_stream3 = usrp->get_tx_stream(stream_args3);

    uhd::stream_args_t stream_args4(cpu_format, wirefmt);
    stream_args4.channels = {3};
    uhd::tx_streamer::sptr tx_stream4 = usrp->get_tx_stream(stream_args4);

    std::thread th1(send_from_file<std::complex<short>>, tx_stream1, file1, spb);
    std::thread th2(send_from_file<std::complex<short>>, tx_stream2, file2, spb);
    std::thread th3(send_from_file<std::complex<short>>, tx_stream3, file3, spb);
    std::thread th4(send_from_file<std::complex<short>>, tx_stream4, file4, spb);
    
    // send_from_file<std::complex<double>>(tx_stream1, tx_stream2, file1, file2, spb);
    
    // th1.detach();
    // th2.detach();
    // th3.detach();
    // th4.detach();
    
    do {
        // send from file
        std::this_thread::yield();
        // std::cout << "Work \n";
        
    } while (not stop_signal_called);

    // finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
