#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Top Block
# Generated: Mon Apr  7 19:13:40 2014
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import numpy
import time
import wx

class top_block(grc_wxgui.top_block_gui):

	def __init__(self):
		grc_wxgui.top_block_gui.__init__(self, title="Top Block")
		_icon_path = "/usr/share/icons/hicolor/32x32/apps/gnuradio-grc.png"
		self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

		##################################################
		# Variables
		##################################################
		self.symbol_rate = symbol_rate = 1000000
		self.samp_rate = samp_rate = 500000

		##################################################
		# Blocks
		##################################################
		self.uhd_usrp_sink_0 = uhd.usrp_sink(
			device_addr="",
			stream_args=uhd.stream_args(
				cpu_format="fc32",
				channels=range(1),
			),
		)
		self.uhd_usrp_sink_0.set_samp_rate(symbol_rate)
		self.uhd_usrp_sink_0.set_center_freq(915000000, 0)
		self.uhd_usrp_sink_0.set_gain(20, 0)
		self.random_source_x_0 = gr.vector_source_s(map(int, numpy.random.randint(0, 2, 2000000)), True)
		self.digital_qam_mod_0 = digital.qam.qam_mod(
		  constellation_points=4,
		  mod_code="gray",
		  differential=True,
		  samples_per_symbol=2,
		  excess_bw=0.01,
		  verbose=False,
		  log=False,
		  )
		self.blocks_throttle_0 = blocks.throttle(gr.sizeof_float*1, symbol_rate)
		self.blocks_short_to_float_0 = blocks.short_to_float(1, 1)
		self.blocks_multiply_xx_0 = blocks.multiply_vcc(1)
		self.blocks_float_to_uchar_0 = blocks.float_to_uchar()
		self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate, analog.GR_COS_WAVE, 915000000, 1, 0)

		##################################################
		# Connections
		##################################################
		self.connect((self.random_source_x_0, 0), (self.blocks_short_to_float_0, 0))
		self.connect((self.blocks_short_to_float_0, 0), (self.blocks_throttle_0, 0))
		self.connect((self.blocks_throttle_0, 0), (self.blocks_float_to_uchar_0, 0))
		self.connect((self.blocks_float_to_uchar_0, 0), (self.digital_qam_mod_0, 0))
		self.connect((self.analog_sig_source_x_0, 0), (self.blocks_multiply_xx_0, 1))
		self.connect((self.blocks_multiply_xx_0, 0), (self.uhd_usrp_sink_0, 0))
		self.connect((self.digital_qam_mod_0, 0), (self.blocks_multiply_xx_0, 0))


	def get_symbol_rate(self):
		return self.symbol_rate

	def set_symbol_rate(self, symbol_rate):
		self.symbol_rate = symbol_rate
		self.blocks_throttle_0.set_sample_rate(self.symbol_rate)
		self.uhd_usrp_sink_0.set_samp_rate(self.symbol_rate)

	def get_samp_rate(self):
		return self.samp_rate

	def set_samp_rate(self, samp_rate):
		self.samp_rate = samp_rate
		self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate)

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	(options, args) = parser.parse_args()
	tb = top_block()
	tb.Run(True)

