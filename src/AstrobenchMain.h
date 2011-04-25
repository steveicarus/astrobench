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
# include  <QDir>
# include  <QMainWindow>
# include  <QSettings>
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

	// This is the directory where all the project files are to go.
      const QDir& project_root() const { return project_path_; }

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

	// Information about the current project, or nil if there is
	// no open project.
      QDir project_path_;
      QSettings*project_;

	// This is the initial number of pages in the stack box. It is
	// used to know where stack images can go.
      int stack_box_count_;

	// Keep a map of identifiers to the StaciItemWidget that they
	// represent. This allows us to quickly choose new identifiers
	// when we create items.
      std::map<unsigned,StackItemWidget*>ident_map_;
      unsigned choose_unique_id_(void);

	// Variables for managing the image display window.
      QGraphicsScene*display_scene_;
      QGraphicsPixmapItem*display_pixmap_;

	// Variables for managing tone mapping.
      std::vector<unsigned short>tone_map_lut_data_;
      QGraphicsScene*tone_map_lut_scene_;
      QGraphicsPixmapItem*tone_map_lut_pixmap_;

	// next_image_ is a temporary space for the next image to be
	// loaded, before it is disposed of in some way.
      vips::VImage*next_image_;
      QString next_path_;

      vips::VImage stack_top_;
      std::list<StackItemWidget*> stack_;

    private: // Image processing function implementations

      void push_stack_item_(StackItemWidget*item);

      template <class T> static void do_tone_map_(void*pin, void*pout, int wid, void*a, void*b);

    private slots:
	// menu action slots
      void menu_new_project_slot_();
      void menu_open_project_slot_();
      void menu_close_project_slot_();
      void menu_save_image_slot_();
	// Stack image tab slots
      void open_next_image_button_slot_();
      void stack_next_image_button_slot_();
	// Tone map tab slots
      void tone_map_calculate_slot_();
      void tone_map_apply_slot_();
};

#endif
