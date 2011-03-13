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
# include  <vector>

class QGraphicsScene;
class QGraphicsPixmapItem;
class StackItemWidget;

class AstrobenchMain : public QMainWindow {

      Q_OBJECT;

    public:
      AstrobenchMain(QWidget*parent =0);
      ~AstrobenchMain();

      void display_image(vips::VImage&img);

    public: // Image processing functions

	// This method applies the tone map to an image. It uses the
	// tone_map_lut_data_ member below to perform the actual
	// transformation. The input is unsigned integral, the output
	// is USHORT.
      vips::VImage tone_map(vips::VImage) throw (vips::VError);

    private:
	// The user interface...
      Ui::AstrobenchMainWidget ui;

	// This is the initial number of pages in the stack box. It is
	// used to know where stack images can go.
      int stack_box_count_;

	// Variables for managing the image display window.
      QGraphicsScene*display_scene_;
      QGraphicsPixmapItem*display_pixmap_;

	// Variables for managing tone mapping.
      std::vector<unsigned short>tone_map_lut_data_;
      QGraphicsScene*tone_map_lut_scene_;
      QGraphicsPixmapItem*tone_map_lut_pixmap_;

      vips::VImage*next_image_;
      QString next_path_;
      std::list<StackItemWidget*> stack_;

    private: // Image processing function implementations

      template <class T> static void do_tone_map_(void*pin, void*pout, int wid, void*a, void*b);

    private slots:
      void open_next_image_button_slot_();
      void stack_next_image_button_slot_();

      void tone_map_calculate_slot_();
      void tone_map_apply_slot_();
};

#endif
