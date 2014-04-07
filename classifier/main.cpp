#include "common.h"
#include <uhd/utils/safe_main.hpp>
#include <uhd/exception.hpp>
#include <boost/program_options.hpp>
#include <gruel/realtime.h>
#include <sys/resource.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
namespace po = boost::program_options;
namespace bacc=boost::accumulators;
bool stop_signal_called = false;
bool useServer = true;
void sig_int_handler(int) {
    stop_signal_called = true;
}

size_t usrp_receive(uhd::rx_streamer::sptr rx_stream, vec_com_flt_t *buff) //gets raw data from a source
{
    uhd::rx_metadata_t md;
	size_t num_rx_samps=0;
    //listening to the uhd usrps
	num_rx_samps = rx_stream->recv(&(buff->front()), buff->size(), md);
	return num_rx_samps;
}

int UHD_SAFE_MAIN(int argc, char *argv[]) {
    //spawning a child to set the time for us
    uhd::set_thread_priority_safe();
    //variables to be set by po
    std::string  fbargs, args, file, type, ant, fbsubdev, subdev, ref, wirefmt, server, filename;
    size_t spb, pktsize, payloadsize, tail;
    size_t code;
    double fbampl, rate, freq, fbgain, gain, fbbw, bw, setup_time;
    int fbrate;
    bool fb;
    bool autokill;
    float threshold, fbjunkampl;
    unsigned short port, rx_port;
    bool RADIO_ENABLE;
    std::string mode, log;
    std::string COMP = "COMP";

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "help message")
    ("args", po::value<std::string>(&args)->default_value(""), "RX uhd device address args")
    ("rate", po::value<double>(&rate)->default_value(5e6), "rate of incoming RX samples")
    ("freq,f", po::value<double>(&freq)->default_value(915e6), "RF center frequency in Hz")
    ("gain", po::value<double>(&gain)->default_value(3), "gain for the RX chain")
    ("subdev", po::value<std::string>(&subdev), "RX daughterboard subdevice specification")
    ("fbsubdev", po::value<std::string>(&fbsubdev), "FB daughterboard subdevice specification")
    ("bw", po::value<double>(&bw), "RX IF filter bandwidth in Hz")
    ("ref", po::value<std::string>(&ref)->default_value("internal"), "waveform type (internal, external, mimo)")
    ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8 or sc16)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    uhd::usrp::multi_usrp::sptr usrp;
    std::vector<std::string> sensor_names;
    uhd::stream_args_t stream_args("fc32",wirefmt);
    uhd::rx_streamer::sptr rx_stream;

    //create the RX usrp device
    
    std::cout << boost::format("Creating RX usrp device with: %s...") % args << std::endl;
    usrp = uhd::usrp::multi_usrp::make(args);

    //Lock mboard clocks
    usrp->set_clock_source(ref);

    //always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev")) usrp->set_rx_subdev_spec(subdev);

    std::cout << boost::format("RX using Device: %s") % usrp->get_pp_string() ;

    //set RX sample rate
    std::cout << boost::format("Setting RX Rate: %f Msps...") % (rate/1e6) << std::endl;
    usrp->set_rx_rate(rate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (usrp->get_rx_rate()/1e6) << std::endl;

    //set RX center frequency
    std::cout << boost::format("Setting RX Freq: %f MHz...") % (freq/1e6) << std::endl;
    usrp->set_rx_freq(freq);
    std::cout << boost::format("Actual RX Freq: %f MHz...") % (usrp->get_rx_freq()/1e6) << std::endl;

    //set RX gain
    std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
    usrp->set_rx_gain(gain);
    std::cout << boost::format("Actual RX Gain: %f dB...") % usrp->get_rx_gain() << std::endl;

    //set RX IF filter bandwidth
    if (vm.count("bw")) {
        std::cout << boost::format("Setting RX Bandwidth: %f MHz...") % (bw/1e6) << std::endl;
        usrp->set_rx_bandwidth(bw);
    }
    std::cout << boost::format("Actual RX Bandwidth: %f MHz...") % (usrp->get_rx_bandwidth()/1e6) << std::endl;

    //set RX antenna
    usrp->set_rx_antenna("RX2");
    std::cout << boost::format("RX Antenna: %s ...") % usrp->get_rx_antenna() << std::endl;

    //Check RX Ref and LO Lock detect
    sensor_names = usrp->get_rx_sensor_names(0);
    if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end()) {
        uhd::sensor_value_t lo_locked = usrp->get_rx_sensor("lo_locked",0);
        std::cout << boost::format("Checking RX: %s ...") % lo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(lo_locked.to_bool());
    }
    sensor_names = usrp->get_mboard_sensor_names(0);
    if ((ref == "mimo") and (std::find(sensor_names.begin(), sensor_names.end(), "mimo_locked") != sensor_names.end())) {
        uhd::sensor_value_t mimo_locked = usrp->get_mboard_sensor("mimo_locked",0);
        std::cout << boost::format("Checking RX: %s ...") % mimo_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(mimo_locked.to_bool());
    }
    if ((ref == "external") and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end())) {
        uhd::sensor_value_t ref_locked = usrp->get_mboard_sensor("ref_locked",0);
        std::cout << boost::format("Checking RX: %s ...") % ref_locked.to_pp_string() << std::endl;
        UHD_ASSERT_THROW(ref_locked.to_bool());
    }

    //setup RX streaming
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    stream_cmd.num_samps = 0;
    stream_cmd.stream_now = true;
    stream_cmd.time_spec = uhd::time_spec_t();
    rx_stream = usrp->get_rx_stream(stream_args);
    rx_stream->issue_stream_cmd(stream_cmd);

    //Enable "real-time" scheduling
    setpriority(PRIO_PROCESS, 0, -20);
    gruel::enable_realtime_scheduling();

    std::signal(SIGINT, &sig_int_handler);
    std::signal(SIGKILL, &sig_int_handler);
    std::signal(SIGTERM, &sig_int_handler);

    vec_com_flt_t *rx_buff = new vec_com_flt_t(1440,0);

    bacc::accumulator_set< float,
                            bacc::stats< bacc::tag::mean >
                          > acc;

    while(not stop_signal_called)
    {
        //main loop
        usrp_receive(rx_stream, rx_buff);
        for(size_t i=0; i<rx_buff->size(); ++i){
            acc(std::abs(rx_buff->at(i)));
        }
        std::cout << "Mean::  " << boost::accumulators::mean(acc) << std::endl;
        
    }

    //finished, stop radio
    rx_stream->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);

    return EXIT_SUCCESS;
}
