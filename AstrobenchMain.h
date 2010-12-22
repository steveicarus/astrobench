#ifndef __AstrobenchMain_H
#define __AstrobenchMain_H
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
# include  <QGraphicsScene>
# include  <QListWidgetItem>
# include  <QTreeWidgetItem>
# include  <list>
# include  <vips/vips>
# include  "ui_astrobench.h"

class SourceImageItem;
class StackedImage;

class AstrobenchMain : public QMainWindow {

      Q_OBJECT

    public:
      AstrobenchMain(QWidget*parent =0);
      ~AstrobenchMain();

	// Display the VImage in the image display window.
      void display_image(vips::VImage&img);

	// Add this image to the image stack.
      void stack_image(SourceImageItem*img);
      void unstack_image(SourceImageItem*img);
	// Declare this image as a dark-field image.
      void dark_field_image(SourceImageItem*img);
	// Remove the image item from any roles and release it.
      void close_image(SourceImageItem*img);

    private:
	// The user interface...
      Ui::AstrobenchMainWidget ui;

	// The Image Display tab uses these members to manage the
	// display of images in the QGraphicsView widget.
      QGraphicsScene*display_scene_;
      QGraphicsPixmapItem*display_pixmap_;

	// The main items in the image stack are listed here.
      std::list<StackedImage*>stack_images_;

	// The images in the image stack are added together into this
	// accumulated sum. The type of this image depends on the
	// types of the input images.
      vips::VImage accumulated_stack_image_;

	//void closeEvent(QCloseEvent*);

    private slots:
	// File menu
      void actionOpen_Image_slot_(void);

	// Signals from the Source Data tab
      void source_item_activated_slot_(QListWidgetItem*item);
      void source_item_context_menu_slot_(const QPoint&);

	// Signals from the Stack tab
      void stack_tree_context_menu_slot_(const QPoint&);
      void stack_save_button_slot_();
      void stack_display_button_slot_();

	// Signals from the Image Display tab
      void image_zoom_slider_value_changed_slot_(int value);
};

/*
 * The SourceImageItem is the starting point for an image. When an
 * image is loaded, a SourceImageItem is created to hold the image and
 * this item is placed in the source items list. All references to the
 * input image ultimately go back to this instance.
 */
class SourceImageItem : public QListWidgetItem {

    public:
      SourceImageItem(const QString&path, vips::VImage*img);
      ~SourceImageItem();

      vips::VImage& image() { return *image_; }

      void set_stack_item(StackedImage*);
      StackedImage*get_stack_item() { return stack_item_; }

    private:
      vips::VImage*image_;

      StackedImage*stack_item_;
};

/*
 * The StackedImage instance represents images that are put into the
 * image stack. This instance refers back to the SourceImageItem that
 * is the source data for the item, and it is inserted into the stack
 * tree as a top-level item.
 */
class StackedImage : public QTreeWidgetItem {

    public:
      StackedImage(SourceImageItem*src);
      ~StackedImage();

	// This returns a reference to the internal image that is the
	// prepared and ready image for this image. Any tweaks to the
	// source image to get here are applied at this point.
      const vips::VImage&image();

    public:
	// This public member is used by the stacker as an accumulator
	// and scratch sum. This class does not use this member in any way.
      vips::VImage accumulator;

    private:
	// This is the source image item, raw.
      SourceImageItem*src_;
};

#endif
