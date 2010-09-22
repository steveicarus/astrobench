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

void AstrobenchMain::stack_image(SourceImageItem*img)
{
      QTreeWidgetItem*item = new QTreeWidgetItem;
      item->setText(0, img->text());
      ui.stack_tree->addTopLevelItem(item);

      img->set_stack_item(item);
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
      QAction a_stack("Stack",      0);
      QAction a_dark ("Dark Field", 0);
      QAction a_close("Close",      0);

      a_stack.setEnabled( ! item->is_stacked() );
      a_dark .setEnabled( ! item->is_stacked() );

      QList<QAction*> menu_list;
      menu_list .append(&a_show);
      menu_list .append(&a_stack);
      menu_list .append(&a_dark);
      menu_list .append(&a_close);

      QAction*hit = QMenu::exec(menu_list, mapToGlobal(pos), &a_show);

      if (hit == &a_show) {
	    display_image(item->image());

      } else if (hit == &a_stack) {
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

void AstrobenchMain::image_zoom_slider_value_changed_slot_(int value)
{
}
