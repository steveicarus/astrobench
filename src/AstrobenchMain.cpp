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
# include  <QDoubleSpinBox>
# include  <QFileDialog>
# include  <QGraphicsPixmapItem>
# include  <QGraphicsScene>
# include  <QString>
# include  "AstrobenchMain.h"
# include  "StackItemWidget.h"

using namespace vips;
using namespace std;

AstrobenchMain::AstrobenchMain(QWidget*parent)
: QMainWindow(parent)
{
      next_image_ = 0;

      ui.setupUi(this);

	// Get the initial stack bock count so that we know where to
	// add new stack images.
      stack_box_count_ = ui.stack_box->count();

      display_pixmap_ = 0;
      display_scene_ = new QGraphicsScene(ui.image_display_view);
      ui.image_display_view ->setScene(display_scene_);

      tone_map_lut_pixmap_ = 0;
      tone_map_lut_scene_ = new QGraphicsScene(ui.tone_map_graph);
      ui.tone_map_graph->setScene(tone_map_lut_scene_);

	// Signals from the Stack Processing tab
      connect(ui.open_next_image_button, SIGNAL(clicked()),
	      SLOT(open_next_image_button_slot_()));
      connect(ui.stack_next_image_button, SIGNAL(clicked()),
	      SLOT(stack_next_image_button_slot_()));

	// Signals from the Tone map tab
      connect(ui.tone_map_calculate, SIGNAL(clicked()),
	      SLOT(tone_map_calculate_slot_()));
      connect(ui.tone_map_apply, SIGNAL(clicked()),
	      SLOT(tone_map_apply_slot_()));
}

AstrobenchMain::~AstrobenchMain()
{
      if (next_image_) delete next_image_;

      for (list<StackItemWidget*>::const_iterator cur = stack_.begin()
		 ; cur != stack_.end() ; ++cur) {
	    delete *cur;
      }
}

void AstrobenchMain::open_next_image_button_slot_(void)
{
      QString caption = tr("Open stack image");
      QString dir;
      QString filter = tr("Images (*.tif *.tiff)");
      next_path_ = QFileDialog::getOpenFileName(this, caption, dir, filter);

	// If the user cancels, then we're done.
      if (next_path_ == "")
	    return;
	// Delete a pre-existing next_image and open this next image.
      if (next_image_)
	    delete next_image_;
      try {
	    next_image_ = new VImage(next_path_.toStdString().c_str());
      } catch (VError err) {
	    err.perror(next_path_.toStdString().c_str());
	    next_image_ = 0;
      }

      display_image(*next_image_);
}

void AstrobenchMain::stack_next_image_button_slot_()
{
      if (next_image_ == 0)
	    return;

	// Convert the next_image_ pointer to a new StackImageWidget.
      StackItemWidget*cur = new StackItemWidget(this);
      cur->set_image(next_path_, *next_image_);
      next_image_ = 0;

	// Get the previous and base images from the stack, if there
	// are any. If I'm about to push the first image, then there
	// are no prev or base images. Then push this image to the top
	// of the stack.
      StackItemWidget*prev = stack_.size() > 0? stack_.front() : 0;
      StackItemWidget*base = stack_.size() > 0? stack_.back()  : 0;
      stack_.push_front(cur);

      QString label;
      if (base == 0) {
	    label = tr("Base image");
      } else {
	    unsigned pos = stack_.size() - 1;
	    label = QString("Stack item %1").arg(pos);
      }

      ui.stack_box->insertItem(stack_box_count_, cur, label);
      cur->show();

      if (base != 0) {
	    assert(prev);
	    cur->calculate_offset_from(base);
	    cur->calculate_stack_from(prev);
      }
}

void AstrobenchMain::tone_map_calculate_slot_()
{
	/* If the image stack is empty, then clear some of the tone
	   map calculations and give up. */
      if (stack_.size() == 0) {
	    ui.tone_map_input_max->setText(QString("N/A"));
	    return;
      }

      StackItemWidget*item = stack_.front();

      QString text;

      unsigned long pixel_max = item->accum_pixel_max();
      text = QString("%1").arg(pixel_max);
      ui.tone_map_input_max->setText(text);

      double gamma = ui.tone_map_gamma->value();
      if (gamma == 0.0)
	    gamma = 1.0;
      else
	    gamma = 1.0/gamma;

	// Create a LUT that is big enough to map the range of the
	// accumulated image, to the 16bit target range.
      unsigned short*lut_data = new unsigned short[3 * pixel_max];

      for (unsigned idx = 0 ; idx < pixel_max ; idx += 1) {
	    unsigned short*lut_ptr = lut_data + 3*idx;

	    unsigned long tmp = pow((double)idx / (double)pixel_max, gamma) * 0x10000;
	    if (tmp > 0xffff) tmp = 0xffff;
	    lut_ptr[0] = tmp;
	    lut_ptr[1] = tmp;
	    lut_ptr[2] = tmp;
      }

	// Put the lut into a VImage in a way that it can be garbate
	// collected. Then drop the malually allocated lut buffer.
      tone_map_lut_ = VImage(lut_data, pixel_max, 1, 3, VImage::FMTUSHORT);

      QSize lut_image_size = ui.tone_map_graph->size();
      int use_width = lut_image_size.width() - 2;
      int use_height = lut_image_size.height() - 2;
      QImage lut_image (use_width, use_height, QImage::Format_RGB32);

      for (int idx = 0 ; idx < use_width ; idx += 1) {
	    int ptr = idx * pixel_max / use_width;
	    if (ptr >= pixel_max) ptr = pixel_max-1;
	    ptr *= 3;

	    int valr = lut_data[ptr+0];
	    int valg = lut_data[ptr+1];
	    int valb = lut_data[ptr+2];

	    valr = valr * use_height / 0x10000;
	    valg = valg * use_height / 0x10000;
	    valb = valb * use_height / 0x10000;

	    if (valr >= use_height) valr = use_height-1;
 	    if (valg >= use_height) valg = use_height-1;
 	    if (valb >= use_height) valb = use_height-1;

	    valr = use_height - 1 - valr;
	    valg = use_height - 1 - valg;
	    valb = use_height - 1 - valb;

	    QRgb tmp;
	    tmp = lut_image.pixel(idx, valr);
	    tmp = qRgb(255, 0, 0);
	    lut_image.setPixel(idx, valr, tmp);

	    tmp = lut_image.pixel(idx, valg);
	    tmp = qRgb(qRed(tmp), 255, 0);
	    lut_image.setPixel(idx, valg, tmp);

	    tmp = lut_image.pixel(idx, valb);
	    tmp = qRgb(qRed(tmp), qGreen(tmp), 255);
	    lut_image.setPixel(idx, valb, tmp);
      }

      if (tone_map_lut_pixmap_)
	    delete tone_map_lut_pixmap_;

      tone_map_lut_pixmap_ = tone_map_lut_scene_->addPixmap(QPixmap::fromImage(lut_image));
      tone_map_lut_pixmap_->show();
      delete[]lut_data;
}

void AstrobenchMain::tone_map_apply_slot_()
{
}

static void draw_from_rgb(QImage&dst, vips::VImage&img)
{
      const unsigned char*data = (const unsigned char*)img.data();

      for (int ydx = 0 ; ydx < img.Ysize() ; ydx += 1) {
	    const unsigned char*src = data + ydx*(3*img.Xsize());
	    for (int xdx = 0 ; xdx < img.Xsize() ; xdx += 1) {
		  dst.setPixel(xdx, ydx, qRgb(src[0], src[1], src[2]));
		  src += 3;
	    }
      }
}

static void draw_from_gray(QImage&dst, vips::VImage&img)
{
      const unsigned char*data = (const unsigned char*)img.data();

      for (int ydx = 0 ; ydx < img.Ysize() ; ydx += 1) {
	    const unsigned char*src = data + ydx*(img.Xsize());
	    for (int xdx = 0 ; xdx < img.Xsize() ; xdx += 1) {
		  dst.setPixel(xdx, ydx, qRgb(src[0], src[0], src[0]));
		  src += 1;
	    }
      }
}

void AstrobenchMain::display_image(vips::VImage&img)
{
      double display_gamma = 1 / ui.tools_gamma->value();
      vips::VImage display_image = img.scale().gammacorrect(display_gamma);
      QImage tmp (display_image.Xsize(), display_image.Ysize(), QImage::Format_RGB32);

      int planes = display_image.Bands();
      assert(planes==1 || planes==3);

      switch (planes) {
	  case 1:
	    draw_from_gray(tmp, display_image);
	    break;
	  case 3:
	    draw_from_rgb(tmp, display_image);
	    break;
	  default:
	    assert(0);
	    break;
      }

      if (display_pixmap_)
	    delete display_pixmap_;

      display_pixmap_ = display_scene_->addPixmap(QPixmap::fromImage(tmp));
      display_pixmap_->show();

      VDMask stats = img.stats();

      QString text = QString("%1 x %2 (x%3)")
	    .arg(img.Xsize())
	    .arg(img.Ysize())
	    .arg(img.Bands());
      ui.stats_dimensions->setText(text);

      long val = stats(0,0);
      text = QString("%1 (0x%2)").arg(val,0,10).arg(val,0,16);
      ui.stats_min->setText(text);

      val = stats(1,0);
      text = QString("%1 (0x%2)").arg(val,0,10).arg(val,0,16);
      ui.stats_max->setText(text);
}
