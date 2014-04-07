#ifndef COMMON_H
#define COMMON_H
#include <volk/volk.h>
#include <fftw3.h>
#include "gri_fft.h"
#include <uhd/utils/thread_priority.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/exception.hpp>
#include <uhd/types/metadata.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/crc.hpp>
#include <boost/multi_array.hpp>
#include <boost/tuple/tuple.hpp>
#include <iostream>
#include <fstream>
#include <csignal>
#include <sstream>
#include <complex>
#include <vector>
#include <list>
#include <algorithm>    // std::random_shuffle
#include <stdlib.h>  //rand()
#include <time.h> //time

typedef std::complex<double> com_dbl_t;
typedef std::vector<com_dbl_t> vec_com_dbl_t;

typedef std::complex<float> com_flt_t;
typedef std::vector<com_flt_t> vec_com_flt_t;
typedef std::vector<float> vec_flt_t;

typedef std::vector<int> vec_int_t;


#endif //COMMON_H
