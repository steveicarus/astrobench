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
# include  <QDoubleSpinBox>
# include  <QFileDialog>
# include  <QGraphicsPixmapItem>
# include  <QGraphicsScene>
# include  <QMessageBox>
# include  <QString>
# include  "AstrobenchMain.h"
# include  "StackItemWidget.h"
# include  <cassert>

using namespace vips;
using namespace std;

/*
 * These are methods for managing project files/directories. These
 * include the slots for opening/closing projects, and other helper
 * methods for keeping projects up to date.
 */

  // An astrobench project contains a .ini file to hold settings.
const QString ASTRO_PROJECT_INI ("astro_project.ini");

/*
 * Create a new project. Use a dialog box to select a path for the new
 * project, and use that to create the project directory structure.
 */
void AstrobenchMain::menu_new_project_slot_(void)
{
	// Project already open
      if (project_ != 0) {
	    QMessageBox::information(this, tr("Busy"),
				     tr("Close the existing project first."));
	    return;
      }

      project_path_ = QString();

	// Request a project path from the user. If the user cancels,
	// then simply quit.
      QString name = QFileDialog::getSaveFileName(this,
			      tr("Enter a new project name"));
      if (name.isEmpty())
	    return;

      project_path_ = name;

	// Temporarily pull the basename out of the project path so
	// that I can make the project directory and cd back into it.
      QString basename = project_path_.dirName();
      assert(!basename.isEmpty());

      project_path_.cdUp();
      assert(project_path_.exists());

	// Make sure the basename has a .iab suffix. Add one if needed.
      if (! basename .endsWith(".iab", Qt::CaseInsensitive))
	    basename .append(".iab");

	// FIXME: For now, don't know how to replace an existing file
	// or project. Quit if the issue comes up.
      if (project_path_.exists(basename)) {
	    QMessageBox::information(this, tr("File Exists"),
				     tr("I don't yet know how to replace files."));
	    project_path_ = QString();
	    return;
      }

	// Create the project directory, and change the project_path_
	// into it. Now we are ready to create all the stuff that goes
	// into a project directory.
      bool rc = project_path_.mkdir(basename);
      assert(rc);

      rc = project_path_.cd(basename);
      assert(rc);

	// Create an initial project setup.
      QString settings_path = project_path_.filePath(ASTRO_PROJECT_INI);

      project_ = new QSettings(settings_path, QSettings::IniFormat);
      project_->setValue("version", 0.0);
}

/*
 * Open an existing project. Pop up a dialog box to help the user
 * search for a directory with the correct suffix. After selecting the
 * directory, check it for sanity, then open the project within.
 */
void AstrobenchMain::menu_open_project_slot_(void)
{
	// Project already open
      if (project_ != 0) {
	    QMessageBox::information(this, tr("Busy"),
				     tr("Close the existing project first."));
	    return;
      }

      QFileDialog selector(this, tr("Select an existing project"));
      selector.setFilter(QDir::Dirs);
      selector.setNameFilter("Icarus AstroBench project (*.iab)");

      int rc = selector.exec();
      if (rc == 0)
	    return;

	// It should only be possible to select a single file. But if
	// the user somehow selected none, pretend the operation is
	// cancelled.
      QStringList files = selector.selectedFiles();
      if (files.size() == 0)
	    return;

	// Check that the directory exists, and that it contains
	// sensible project files. Then open the project.
      project_path_ = files[0];
      assert(project_path_.exists());
      if (!project_path_.exists(ASTRO_PROJECT_INI)) {
	    QMessageBox::information(this, tr("Error"),
				     tr("Project missing settings subfile."));
	    project_path_ = QString();
	    return;
      }

      QString settings_path = project_path_.filePath(ASTRO_PROJECT_INI);
      project_ = new QSettings(settings_path, QSettings::IniFormat);

	// Get the list of known items as a collection of keys under
	// the "items" group.
      project_->beginGroup("items");
      QStringList items = project_->childKeys();
      project_->endGroup();

	// Create all the StackItemWidgets for all the listed items.
      for (int idx = 0 ; idx < items.size() ; idx += 1) {
	    QString item_str = items[idx];
	    bool ok_flag = true;
	    unsigned item_id = item_str.toUInt(&ok_flag, 10);
	    assert(ok_flag);

	    StackItemWidget*cur = new StackItemWidget(this, item_id);
	    ident_map_[cur->ident()] = cur;

	    printf("XXXX recover image %u\n", cur->ident());
	    cur->recover_data();
      }

	// First recover the base image and push it to the stack.
      unsigned base_id = project_->value("base_image").toUInt();
      printf("XXXX Use image %u as the base image\n", base_id);
      StackItemWidget*base = ident_map_[base_id];
      assert(base);
      push_stack_item_(base);

	// Now push all the remaining images to the stack.
      for (map<unsigned,StackItemWidget*>::iterator cur = ident_map_.begin()
		 ; cur != ident_map_.end() ; ++cur) {
	    if (cur->second == base)
		  continue;

	    printf("XXXX Push image %u\n", cur->first);
	    push_stack_item_(cur->second);
      }
}

void AstrobenchMain::menu_close_project_slot_(void)
{
      if (project_ == 0)
	    return;

      project_->sync();
      delete project_;
      project_ = 0;
      project_path_ = QString();

	// Empth the object stack.
      stack_.clear();

	// Clear the items from the toolbox
      while (ui.stack_box->count() > stack_box_count_) {
	    ui.stack_box->removeItem(stack_box_count_);
      }

	// Clear the ident_map_ map.
      for (map<unsigned,StackItemWidget*>::iterator cur = ident_map_.begin()
		 ; cur != ident_map_.end() ; ++cur) {
	    delete cur->second;
      }
      ident_map_.clear();
}
