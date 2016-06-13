/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.6.3.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#include <iostream>
#include <fstream>

#include <QGLViewer/qglviewer.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QInputDialog>

#include <vector>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/centroid.h>
#include "compute_normal.h"

typedef CGAL::Simple_cartesian<double>               Kernel;
typedef Kernel::Point_3                              Point_3;
typedef CGAL::Polyhedron_3<Kernel>                   Polyhedron;
typedef Polyhedron::Facet_iterator                   Facet_iterator;
typedef Polyhedron::Halfedge_around_facet_circulator Halfedge_facet_circulator;
typedef double REALD;
typedef Kernel::Triangle_3 Triangle;
typedef std::list<Triangle> TrianglesList;
typedef Polyhedron::Halfedge_handle CGAL_Halfedge_handle;
typedef Point_3 CGAL_Point;
typedef Kernel::Vector_3 CGAL_Vector;
#define ROW 16
#define COL 3
#define FIGURE_FORMAT "JPEG"
static int color[16][3] = { 255,230,90, 193,246,128, 255,187,90, 75,182,247, 96,249,228, 247,75,75, 160,74,75, 75,75,250, 255,185,132, 125,210,210, 127,210,126, 250,120,75, 0, 120,0, 139, 129, 76, 238,238,209, 255, 182, 193};

#define READ_FROM_DIALOGUE

class Viewer : public QGLViewer
{


protected:
	virtual void draw();
	virtual void init();
	virtual void animate();
	virtual QString helpString() const;

	void read_files();
	void recursive_load(QString path);
	void render_mesh_list();
	void calculateMassCenterInertia(Polyhedron &poly, double& massReturn, CGAL_Point &cmReturn, CGAL_Vector &inertiaReturn);
	void subexpressions_integral_terms(REALD w0, REALD w1, REALD w2, REALD &f1, REALD &f2, REALD &f3, REALD &g0, REALD &g1, REALD &g2);
private:
	int nbPart;
	std::vector<Polyhedron*> meshList;
	std::vector<CGAL_Vector> cmList;
	CGAL_Vector cmOrigin;
	int count;
	std::vector<std::vector<QString> > absolute_files;
	int vec_now;
	unsigned vec_pos;
	int rotating_axis[2];
};

