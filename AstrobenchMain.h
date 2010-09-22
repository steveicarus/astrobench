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
# include  <vips/vips>
# include  "ui_astrobench.h"

class SourceImageItem;

class AstrobenchMain : public QMainWindow {

      Q_OBJECT

    public:
      AstrobenchMain(QWidget*parent =0);
      ~AstrobenchMain();

	// Display the VImage in the image display window.
      void display_image(vips::VImage&img);

	// Add this image to the image stack.
      void stack_image(SourceImageItem*img);
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

	//void closeEvent(QCloseEvent*);

    private slots:
	// File menu
      void actionOpen_Image_slot_(void);

	// Signals from the Source Data tab
      void source_item_activated_slot_(QListWidgetItem*item);
      void source_item_context_menu_slot_(const QPoint&);

	// Signals from the Stack tab
      void stack_tree_context_menu_slot_(const QPoint&);

	// Signals from the Image Display tab
      void image_zoom_slider_value_changed_slot_(int value);
};

class SourceImageItem : public QListWidgetItem {

    public:
      SourceImageItem(const QString&path, vips::VImage*img);
      ~SourceImageItem();

      vips::VImage& image() { return *image_; }

      void set_stack_item(QTreeWidgetItem*);
      inline bool is_stacked (void) const { return stack_item_ != 0; }

    private:
      vips::VImage*image_;

      QTreeWidgetItem*stack_item_;
};

#endif
