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
# include  <vips/vips>
# include  "ui_astrobench.h"


class AstrobenchMain : public QMainWindow {

      Q_OBJECT

    public:
      AstrobenchMain(QWidget*parent =0);
      ~AstrobenchMain();

      void display_image(vips::VImage&img);

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

	// Signals from the Image Display tab
      void image_zoom_slider_value_changed_slot_(int value);
};

#endif
