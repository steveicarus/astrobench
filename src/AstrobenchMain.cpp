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

      display_pixmap_ = 0;
      display_scene_ = new QGraphicsScene(ui.image_display_view);
      ui.image_display_view ->setScene(display_scene_);

	// Signals from the Stack Processing tab
      connect(ui.open_next_image_button, SIGNAL(clicked()),
	      SLOT(open_next_image_button_slot_()));
      connect(ui.stack_next_image_button, SIGNAL(clicked()),
	      SLOT(stack_next_image_button_slot_()));
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

      ui.stack_box->insertItem(1, cur, label);
      cur->show();

      if (base != 0) {
	    assert(prev);
	    cur->calculate_offset_from(base);
	    cur->calculate_stack_from(prev);
      }
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
      vips::VImage display_image = img.scale();
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
