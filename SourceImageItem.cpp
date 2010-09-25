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

# include  "AstrobenchMain.h"
# include  <vips/vips>

using namespace vips;

SourceImageItem::SourceImageItem(const QString&path, VImage*img)
: QListWidgetItem(path), image_(img)
{
      stack_item_ = 0;
}

SourceImageItem::~SourceImageItem()
{
      if (image_) delete image_;
}

void SourceImageItem::set_stack_item(StackedImage*item)
{
      assert(stack_item_ == 0);
      stack_item_ = item;
}

StackedImage::StackedImage(SourceImageItem*src)
: src_(src)
{
      setText(0, src_->text());
      src_->set_stack_item(this);
}

const vips::VImage& StackedImage::image()
{
      return src_->image();
}
