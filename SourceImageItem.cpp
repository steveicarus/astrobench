/*
 * Copyright (c) 2010 Stephen Williams (steve@icarus.com)
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
# include  <vips/vips>
# include  <vips/vips.h>

using namespace vips;

SourceImageItem::SourceImageItem(const QString&path, VImage*img)
: QListWidgetItem(path), image_(img)
{
      stack_item_ = 0;
}

SourceImageItem::~SourceImageItem()
{
      if (image_) delete image_;
}

void SourceImageItem::set_stack_item(StackedImage*item)
{
      assert(stack_item_ == 0 || item == 0);
      stack_item_ = item;
}

StackedImage::StackedImage(SourceImageItem*src)
: src_(src)
{
      shift_x_ = 0;
      shift_y_ = 0;
      setText(0, src_->text());
      src_->set_stack_item(this);
}

StackedImage::~StackedImage()
{
      src_->set_stack_item(0);
}

static int progress_callback(void*arg1, void*arg2)
{
      IMAGE*img = (IMAGE*)arg1;
      const char*heading = (const char*)arg2;

      printf("%s: %d%%...\n", heading, img->time->percent);
      fflush(stdout);
      return 0;
}

void StackedImage::calculate_offset(const StackedImage*ref)
{
      vips::VImage ref_tmp;
      if (ref->src_->image().Bands() == 3) {
	    ref_tmp = ref->src_->image().extract_bands(1,1);
      } else {
	    ref_tmp = ref->src_->image().extract_bands(0,1);
      }

      vips::VImage this_tmp;
      if (src_->image().Bands() == 3) {
	    this_tmp = src_->image().extract_bands(1,1);
      } else {
	    this_tmp = src_->image().extract_bands(0,1);
      }

      vips::VImage corr = ref_tmp.phasecor_fft(this_tmp);
#if 1
      im_add_eval_callback(corr.image(), progress_callback, corr.image(), (void*)"corr_tmp");
#endif

      double xpos, ypos;

      std::complex<double>pos = corr.maxpos();
      xpos = pos.real();
      ypos = pos.imag();
      printf("XXXX xpos/ypos = %f/%f\n", xpos, ypos);
      fflush(stdout);

      if (xpos > src_->image().Xsize()/2)
	    xpos = xpos - src_->image().Xsize();
      if (ypos > src_->image().Ysize()/2)
	    ypos = ypos - src_->image().Ysize();

      shift_x_ = -xpos;
      shift_y_ = -ypos;
      printf("XXXX shift_x_/shift_y_ = %d/%d\n", shift_x_, shift_y_);
      fflush(stdout);
}

const vips::VImage& StackedImage::image()
{
      if (shift_x_ == 0 && shift_y_ == 0)
	    return src_->image();

	// Where to trim from the soure image
      int tx = 0, ty = 0, tw, th;
	// Where to place trimmed image into padded image
      int px = 0, py = 0;

      vips::VImage&ref = src_->image();

      if (shift_x_ >= 0) {
	    tw = ref.Xsize() - shift_x_;
	    tx = shift_x_;
	    px = 0;
      } else {
	    tw = ref.Xsize() + shift_x_;
	    tx = 0;
	    px = -shift_x_;
      }

      if (shift_y_ >= 0) {
	    th = ref.Ysize() - shift_y_;
	    ty = shift_y_;
	    py = 0;
      } else {
	    th = ref.Ysize() + shift_y_;
	    ty = 0;
	    py = -shift_y_;
      }

      processed_ = ref.extract_area(tx, ty, tw, th)
	              .embed(0, px, py, ref.Xsize(), ref.Ysize());
      return processed_;
}

#if 0
const vips::VImage& StackedImage::fwfft(void)
{
      fwfft_ = src_->image().grey().fwfft();
      return fwfft_;
}
#endif
