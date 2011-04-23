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

# include  "StackItemWidget.h"
# include  "AstrobenchMain.h"
# include  <stdio.h>
# include  <cassert>

using namespace vips;

StackItemWidget::StackItemWidget(AstrobenchMain*am, QWidget*parent)
    : QWidget(parent), astromain_(am)
{
      ui.setupUi(this);

	// Display buttons
      connect(ui.stack_item_display_raw, SIGNAL(clicked()),
	      SLOT(display_raw_slot_()));
      connect(ui.stack_item_display_aligned, SIGNAL(clicked()),
	      SLOT(display_aligned_slot_()));
      connect(ui.stack_item_display_stack, SIGNAL(clicked()),
	      SLOT(display_stacked_slot_()));
}

StackItemWidget::~StackItemWidget()
{
}

void StackItemWidget::set_image(const QString&path, const vips::VImage&img)
{
      image_ = img;
      processed_ = image_;
      accumulated_ = image_;
      accumulated_stats_ = accumulated_.stats();
      ui.stack_item_path->setText(path);
      ui.stack_item_path->setToolTip(path);

      vips::VImage ref_tmp;
      if (image_.Bands() == 3)
	    ref_tmp = image_.extract_bands(1,1);
      else
	    ref_tmp = image_.extract_bands(0,1);

      image_fwfft_ = ref_tmp.fwfft();
}

void StackItemWidget::calculate_offset_from(StackItemWidget*that)
{
      assert(image_.Xsize() == that->image_.Xsize());
      assert(image_.Ysize() == that->image_.Ysize());

      vips::VImage xphase = that->image_fwfft_.cross_phase(image_fwfft_);
      vips::VImage corr = xphase.invfftr();

      double xpos, ypos;

      std::complex<double>pos = corr.maxpos();
      xpos = pos.real();
      ypos = pos.imag();
      printf("XXXX xpos/ypos = %f/%f\n", xpos, ypos);
      fflush(stdout);

      if (xpos > image_.Xsize()/2)
	    xpos = xpos - image_.Xsize();
      if (ypos > image_.Ysize()/2)
	    ypos = ypos - image_.Ysize();

      shift_x_ = -xpos;
      shift_y_ = -ypos;
      ui.stack_item_dx_entry->setValue(shift_x_);
      ui.stack_item_dy_entry->setValue(shift_y_);

	// If the end result is no shift, then we are done.
      if (shift_x_ == 0 && shift_y_ == 0) {
	    processed_ = image_;
	    accumulated_ = image_;
	    return;
      }


	// Where to trim from the soure image
      int tx = 0, ty = 0, tw, th;
	// Where to place trimmed image into padded image
      int px = 0, py = 0;

      if (shift_x_ >= 0) {
	    tw = image_.Xsize() - shift_x_;
	    tx = shift_x_;
	    px = 0;
      } else {
	    tw = image_.Xsize() + shift_x_;
	    tx = 0;
	    px = -shift_x_;
      }

      if (shift_y_ >= 0) {
	    th = image_.Ysize() - shift_y_;
	    ty = shift_y_;
	    py = 0;
      } else {
	    th = image_.Ysize() + shift_y_;
	    ty = 0;
	    py = -shift_y_;
      }

      processed_ = image_.extract_area(tx, ty, tw, th)
	              .embed(0, px, py, image_.Xsize(), image_.Ysize());
      accumulated_ = processed_;
      accumulated_stats_ = accumulated_.stats();
}

void StackItemWidget::calculate_stack_from(StackItemWidget*that)
{
      accumulated_ = processed_ + that->accumulated_;
      accumulated_stats_ = accumulated_.stats();
}

const VImage& StackItemWidget::accumulated_image(void) const
{
      return accumulated_;
}

unsigned StackItemWidget::accumulated_pixel_max()
{
      return accumulated_stats_(1,0);
}

void StackItemWidget::display_raw_slot_(void)
{
      astromain_->display_image(image_);
}

void StackItemWidget::display_aligned_slot_(void)
{
      astromain_->display_image(processed_);
}

void StackItemWidget::display_stacked_slot_(void)
{
      astromain_->display_image(accumulated_);
}
