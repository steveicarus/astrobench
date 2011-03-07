#ifndef __AstrobenchMain_H
#define __AstrobenchMain_H
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
# include  <QMainWindow>
# include  <QString>
# include  "ui_astrobench.h"
# include  <vips/vips>
# include  <list>

class QGraphicsScene;
class QGraphicsPixmapItem;
class StackItemWidget;

class AstrobenchMain : public QMainWindow {

      Q_OBJECT;

    public:
      AstrobenchMain(QWidget*parent =0);
      ~AstrobenchMain();

      void display_image(vips::VImage&img);

    private:
	// The user interface...
      Ui::AstrobenchMainWidget ui;

	// Variables for managing the image display window.
      QGraphicsScene*display_scene_;
      QGraphicsPixmapItem*display_pixmap_;

      vips::VImage*next_image_;
      QString next_path_;
      std::list<StackItemWidget*> stack_;

    private slots:
      void open_next_image_button_slot_();
      void stack_next_image_button_slot_();
};

#endif
