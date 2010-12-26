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

# include  <qapplication.h>
# include  "AstrobenchMain.h"
# include  <QFileDialog>
# include  <QGraphicsPixmapItem>
# include  <QListWidgetItem>
# include  <QTreeWidget>
# include  <vips/vips>

using namespace vips;

AstrobenchMain::AstrobenchMain(QWidget*parent)
: QMainWindow(parent)
{
      ui.setupUi(this);

      display_pixmap_ = 0;
      display_scene_ = new QGraphicsScene(ui.image_display_view);
      ui.image_display_view ->setScene(display_scene_);

	// File Menu
      connect(ui.actionOpen_Image,
	      SIGNAL(triggered()),
	      SLOT(actionOpen_Image_slot_()));

	// Signals from the Source Data tab
      connect(ui.source_list, SIGNAL(itemActivated(QListWidgetItem*)),
	      SLOT(source_item_activated_slot_(QListWidgetItem*)));
      connect(ui.source_list, SIGNAL(customContextMenuRequested(const QPoint&)),
	      SLOT(source_item_context_menu_slot_(const QPoint&)));

	// Signals from the Stack tab
      connect(ui.stack_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
	      SLOT(stack_tree_context_menu_slot_(const QPoint&)));
      connect(ui.stack_save_button, SIGNAL(clicked()),
	      SLOT(stack_save_button_slot_()));
      connect(ui.stack_align_button, SIGNAL(clicked()),
	      SLOT(stack_align_button_slot_()));
      connect(ui.stack_display_button, SIGNAL(clicked()),
	      SLOT(stack_display_button_slot_()));

	// Signals from the Image Display tab
      connect(ui.image_zoom_slider, SIGNAL(valueChanged(int)),
	      SLOT(image_zoom_slider_value_changed_slot_(int)));
}

AstrobenchMain::~AstrobenchMain()
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
}

/*
 * The stack_image and unstack_image methods add a source image to the
 * image stack, or remove that image from the stack.
 *
 * The image stack is an accumulated sum of images. The image stack
 * makes use of the "accumulator" member of each image to keep a
 * running sum of the current image with the accumulated previous
 * image. The stack is managed with a std::list, with the beginning of
 * the list being the top of the stack. Images are added to the top of
 * the stack, although they can be removed from anywhere in the stack.
 */
void AstrobenchMain::stack_image(SourceImageItem*img)
{
      StackedImage*item = new StackedImage(img);
      ui.stack_tree->addTopLevelItem(item);
      stack_images_.push_front(item);

      if (stack_images_.size() == 1) {
	      // If this is the only image, then its accumulated value
	      // is the image itself.
	    item->accumulator = item->image();

      } else {
#if 1
	      // Calculate the offset of this image from the reference
	      // image, which is by definition the image in the bottom
	      // of the stack.
	    StackedImage*ref = stack_images_.back();
	    item->calculate_offset(ref);
#endif
	      // There are existing images in the stack, so its
	      // accumulated value is the image itself plus the
	      // accumulated value of the rest of the stack.
	    std::list<StackedImage*>::iterator ptr = stack_images_.begin();
	    ptr ++;
	    item->accumulator = item->image() + (*ptr)->accumulator;
      }

      accumulated_stack_image_ = item->accumulator;
}

void AstrobenchMain::unstack_image(SourceImageItem*img)
{
      StackedImage*item = img->get_stack_item();
      assert(item);

      std::list<StackedImage*>::iterator ptr = stack_images_.begin();
      while (item != *ptr) {
	    assert(ptr != stack_images_.end());
	    ptr ++;
      }

      if (stack_images_.size() == 1) {
	    stack_images_.erase(ptr);
	    accumulated_stack_image_ = vips::VImage();

      } else if (ptr == stack_images_.begin()) {
	    stack_images_.erase(ptr);
	    ptr = stack_images_.begin();
	    accumulated_stack_image_ = (*ptr)->accumulator;

      } else {
	    std::list<StackedImage*>::iterator prev = ptr;
	    prev --;
	    stack_images_.erase(ptr);

	    std::list<StackedImage*>::iterator next = prev;
	    next ++;
	      // Restack the images before the image that has been
	      // removed. Step all the way to the beginning of the list.
	    for ( ;; ) {
		  if (next == stack_images_.end())
			(*prev)->accumulator = (*prev)->image();
		  else
			(*prev)->accumulator = (*prev)->image() + (*next)->accumulator;

		  if (prev == stack_images_.begin())
			break;

		  prev --;
		  next --;
	    }

	    accumulated_stack_image_ = (*prev)->accumulator;
      }

      delete item;
}

void AstrobenchMain::dark_field_image(SourceImageItem*img)
{
}

void AstrobenchMain::close_image(SourceImageItem*img)
{
}

void AstrobenchMain::actionOpen_Image_slot_(void)
{
      QStringList paths = QFileDialog::getOpenFileNames(this,
			      tr("Select image files to open."),
			      QString(),
			      "Images (*.png *.jpg *.tif *.tiff)");

      for (QStringList::iterator idx = paths.begin()
		 ; idx != paths.end() ; idx ++) {
	    QString path = *idx;
	    VImage*img = 0;

	    try {
		  img = new VImage(path.toStdString().c_str());
	    } catch (VError err) {
		  err.perror(path.toStdString().c_str());
		  img = 0;
	    }

	    if (img == 0)
		  continue;

	    SourceImageItem*item = new SourceImageItem(path, img);
	    ui.source_list->addItem(item);
      }
}

void AstrobenchMain::source_item_activated_slot_(QListWidgetItem*raw_item)
{
      SourceImageItem*item = dynamic_cast<SourceImageItem*> (raw_item);

      if (item == 0)
	    return;

      display_image(item->image());
}

void AstrobenchMain::source_item_context_menu_slot_(const QPoint&pos)
{
      QListWidgetItem*raw_item = ui.source_list->itemAt(pos);
      if (raw_item == 0) return;

      SourceImageItem*item = dynamic_cast<SourceImageItem*> (raw_item);
      if (item == 0) return;

      QAction a_show ("Show",       0);
      QAction a_stack(item->get_stack_item()? "UN-Stack" : "Stack", 0);
      QAction a_dark ("Dark Field", 0);
      QAction a_close("Close",      0);

      a_dark .setEnabled( ! item->get_stack_item() );

      QList<QAction*> menu_list;
      menu_list .append(&a_show);
      menu_list .append(&a_stack);
      menu_list .append(&a_dark);
      menu_list .append(&a_close);

      QAction*hit = QMenu::exec(menu_list, mapToGlobal(pos), &a_show);

      if (hit == &a_show) {
	    display_image(item->image());

      } else if (hit == &a_stack) {
	    if (item->get_stack_item())
		  unstack_image(item);
	    else
		  stack_image(item);

      } else if (hit == &a_dark) {
	    dark_field_image(item);

      } else if (hit == &a_close) {
	    close_image(item);
      }
}

void AstrobenchMain::stack_tree_context_menu_slot_(const QPoint&pos)
{
      QTreeWidgetItem*raw_item = ui.stack_tree->itemAt(pos);
      if (raw_item == 0) return;
}

void AstrobenchMain::stack_save_button_slot_(void)
{
}

void AstrobenchMain::stack_align_button_slot_(void)
{
	// If there are no image to align to a reference image, then
	// do nothing.
      if (stack_images_.size() < 2)
	    return;

	// XXXX NOT IMPLEMENTED
      return;
}

void AstrobenchMain::stack_display_button_slot_(void)
{
      display_image(accumulated_stack_image_);
}

void AstrobenchMain::image_zoom_slider_value_changed_slot_(int value)
{
}
