#ifndef __StackItemWidget_H
#define __StackItemWidget_H
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

# include  <qapplication.h>
# include  <QWidget>
# include  "ui_stackitem.h"
# include  <vips/vips>

class AstrobenchMain;

class StackItemWidget  : public QWidget {

      Q_OBJECT;

    public:
      StackItemWidget(AstrobenchMain*am, unsigned ident, QWidget*parent =0);
      ~StackItemWidget();

      unsigned ident() const { return ident_; }

	// Set the raw image for this item.
      void set_image(const QString&use_label, vips::VImage img);

	// If opening an existing project, this method is used in
	// place of the set_image method to recover the image from the
	// image file.
      void recover_data(const QString&use_label);

	// Calculate the offset of this image from the image argument,
	// which is expected to the be base image.
      void calculate_offset_from(StackItemWidget*that);
	// Calculate the stack sum of this image from the image
	// argument, which is expected to the the previous top of the stack.
      void calculate_stack_from(StackItemWidget*that);

	// Statistics about the accumulated image.
      const vips::VImage& accumulated_image() const;
      unsigned accumulated_pixel_max();

    private slots:
      void display_raw_slot_(void);
      void display_aligned_slot_(void);
      void display_stacked_slot_(void);

    private:
      void initialize_from_image_(const QString&path);

    private:
      Ui::StackItem ui;

	// This is a pointer to the main window. Use it for global actions.
      AstrobenchMain*astromain_;

	// Unique identifier for this item.
      unsigned ident_;

	// This is the original image, passed in from the creator.
      vips::VImage image_;

	// This is a fwFFT version of the image, that can be used for
	// phase correlation. We may need it multiple times, so this
	// acts as a cache.
      vips::VImage image_fwfft_;

	// Amount of shift to line up with the base image. These are
	// calculated by calculate_offset_from().
      int shift_x_, shift_y_;

	// The processed_ image is a version of the image shifted and
	// otherwise processed by local (to this instance) variables.
      vips::VImage processed_;

	// The accumulated is the processed_ image added to the
	// previously accumulated images. It is this point in the stack.
      vips::VImage accumulated_;
      vips::VDMask accumulated_stats_;
};

#endif
