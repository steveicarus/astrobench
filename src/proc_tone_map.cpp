
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "AstrobenchMain.h"
# include  <vips/vips.h>

using namespace vips;
using namespace std;

template <class T> void AstrobenchMain::do_tone_map_(void*pin, void*pout, int width, void*a, void*b)
{
      IMAGE*in = reinterpret_cast<IMAGE*>(a);
      AstrobenchMain*obj = reinterpret_cast<AstrobenchMain*>(b);
      unsigned short*dst = reinterpret_cast<unsigned short*> (pout);
      T*src = reinterpret_cast<T*>(pin);
      int planes = in->Bands;

      for (int idx = 0 ; idx < width*planes ; idx += 1) {
	    if (src[idx] >= obj->tone_map_lut_data_.size())
		  dst[idx] = obj->tone_map_lut_data_.back();
	    else
		  dst[idx] = obj->tone_map_lut_data_[src[idx]];
      }
}

VImage AstrobenchMain::tone_map(VImage in) throw (VError)
{
      VImage out;

      IMAGE*use_in = in.image();
      IMAGE*use_out = out.image();

      if (im_piocheck(use_in, use_out))
	    throw VError(__FILE__ ":" __STRING(__LINE__));

      if (im_cp_desc(use_out, use_in))
	    throw VError();

      use_out->BandFmt = IM_BANDFMT_USHORT;

      switch (use_in->BandFmt) {
	  case IM_BANDFMT_UINT:
	    if (im_wrapone(use_in, use_out, &do_tone_map_<unsigned>, use_in, this))
		  throw VError(__FILE__ ":" __STRING(__LINE__));
	    break;
	  case IM_BANDFMT_USHORT:
	    if (im_wrapone(use_in, use_out, &do_tone_map_<unsigned short>, use_in, this))
		  throw VError(__FILE__ ":" __STRING(__LINE__));
	    break;
	  case IM_BANDFMT_UCHAR:
	    if (im_wrapone(use_in, use_out, &do_tone_map_<unsigned char>, use_in, this))
		  throw VError(__FILE__ ":" __STRING(__LINE__));
	    break;
	  default:
	    throw VError(__FILE__ ":" __STRING(__LINE__));
      }

      return out;
}
