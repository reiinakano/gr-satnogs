/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, Libre Space Foundation <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "morse_debug_source_impl.h"
#include <satnogs/morse.h>

namespace gr {
  namespace satnogs {

    morse_debug_source::sptr
    morse_debug_source::make(const std::string& debug_seq)
    {
      return gnuradio::get_initial_sptr
        (new morse_debug_source_impl(debug_seq));
    }

    /*
     * The private constructor
     */
    morse_debug_source_impl::morse_debug_source_impl(std::string debug_seq)
      : gr::block("morse_debug_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
	      d_run(true),
	      d_chars { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
		      'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
		      'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7',
		      '8', '9', '0' },
	      d_symbols { ".-", "-...", "-.-.", "-..", ".", "..-.", "--.",
		      "....", "..", ".---", "-.-", ".-..", "--", "-.",
		      "---", ".--.", "--.-", ".-.", "...", "-", "..-",
		      "...-", ".--", "-..-", "-.--", "--..", ".----",
		      "..---", "...--", "....-", ".....", "-....", "--...",
		      "---..", "----.", "-----"}
    {
      message_port_register_out(pmt::mp("out"));
      d_thread = std::thread(&morse_debug_source_impl::send_debug_msg,
			     this,
			     debug_seq);
    }

    static inline size_t
    find_char_idx(const char* characters, size_t len, char c)
    {
      size_t i;
      for(i = 0; i < len; i++) {
	if(characters[i] == c){
	  return i;
	}
      }
      return len;
    }

    void
    morse_debug_source_impl::send_debug_msg (std::string sentence)
    {
      size_t i;
      size_t j;
      size_t idx;
      std::string s;
      char c;
      size_t len = sentence.length();
      pmt::pmt_t port = pmt::mp("out");

      std::this_thread::sleep_for(std::chrono::milliseconds(2000));
      while(d_run) {
	/* Not the best approach, but hey, this is only for debug */
	for(i = 0; i < len; i++){
	  c = std::toupper(sentence[i]);
	  if(c == ' '){
	    message_port_pub(port, pmt::from_long(MORSE_L_SPACE));
	  }

	  idx = find_char_idx(d_chars, sizeof(d_chars), c);
	  if(idx != sizeof(d_chars)){
	    s = d_symbols[idx];
	    for(j = 0; j < s.length(); j++) {
	      if(s[j] == '.'){
		message_port_pub(port, pmt::from_long(MORSE_DOT));
	      }
	      else{
		message_port_pub(port, pmt::from_long(MORSE_DASH));
	      }
	      std::this_thread::sleep_for(std::chrono::milliseconds(100));
	    }

	    /* Send also a character delimiter after waiting a little */
	    std::this_thread::sleep_for(std::chrono::milliseconds(200));
	    message_port_pub(port, pmt::from_long(MORSE_S_SPACE));
	  }
	}
	message_port_pub(port, pmt::from_long(MORSE_L_SPACE));
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    }

    /*
     * Our virtual destructor.
     */
    morse_debug_source_impl::~morse_debug_source_impl()
    {
      d_run = false;
      d_thread.join();
    }

  } /* namespace satnogs */
} /* namespace gr */

