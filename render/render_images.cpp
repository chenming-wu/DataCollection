#include "render_images.h"
#include <math.h>
#include <stdlib.h> // RAND_MAX

using namespace qglviewer;
using namespace std;

//#define FILTER_RECENTER

void save_center_of_mass(CGAL_Vector& p) {
	std::ofstream file_ms("mass_of_center");
	file_ms << p << std::endl;
	file_ms.close();
}

void read_center_of_mass(CGAL_Vector& p) {
	std::ifstream file_ms("mass_of_center");
	file_ms >> p;
	file_ms.close();
}

void set_color_to_constant() {
	for (int i = 0; i < ROW; ++i) {
		color[i][0] = 188;
		color[i][1] = 182;
		color[i][2] = 168;
	}
}

CGAL_Point get_center_of_mass(Polyhedron &poly)
{
	std::list<Triangle> triangles;
	Polyhedron::Facet_iterator f;
	for (f = poly.facets_begin(); f != poly.facets_end(); ++f) {
		const CGAL_Point& a = f->halfedge()->vertex()->point();
		const CGAL_Point& b = f->halfedge()->next()->vertex()->point();
		const CGAL_Point& c = f->halfedge()->prev()->vertex()->point();
		triangles.push_back(Triangle(a, b, c));
	}
	return CGAL::centroid(triangles.begin(), triangles.end());
}

void re_centered(Polyhedron* pPolyhedron)
{
	Polyhedron::Point_iterator it = pPolyhedron->points_begin();
	auto bbox = (*it).bbox();
	for (; it != pPolyhedron->points_end(); it++) {
		bbox = bbox + (*it).bbox();
	}
	double dx = bbox.xmax() - bbox.xmin();
	double dy = bbox.ymax() - bbox.ymin();
	double dz = bbox.zmax() - bbox.zmin();

	const double dRequiredDiag = 50.0;//2.0;
	double scaleFactor = dRequiredDiag / std::sqrt(dx*dx + dy*dy + dz*dz);

	CGAL_Point ptCenter = get_center_of_mass(*pPolyhedron);
	Polyhedron::Point_iterator itr = pPolyhedron->points_begin();
	for (; itr != pPolyhedron->points_end(); itr++) {
		*itr = CGAL_Point((itr->x() - ptCenter.x()) * scaleFactor, (itr->y() - ptCenter.y()) *scaleFactor, (itr->z() - ptCenter.z()) * scaleFactor);
	}
}

void recenter_and_save(Polyhedron* pPolyhedron) {
	re_centered(pPolyhedron);
	std::ofstream file_recentered("recentered.off");
	file_recentered << *pPolyhedron;
	file_recentered.close();
}

void Viewer::subexpressions_integral_terms(REALD w0, REALD w1, REALD w2, REALD &f1, REALD &f2, REALD &f3, REALD &g0, REALD &g1, REALD &g2)
{
	REALD temp0 = w0 + w1;
	REALD temp1 = w0*w0;
	REALD temp2 = temp1 + w1*temp0;
	f1 = temp0 + w2;
	f2 = temp2 + w2*f1;
	f3 = w0*temp1 + w1*temp2 + w2*f2;
	g0 = f2 + w0*(f1 + w0);
	g1 = f2 + w1*(f1 + w1);
	g2 = f2 + w2*(f1 + w2);
}

//The return values are the mass, the center of mass, and the inertia tensor relative to the
// center of mass. The code assumes that the rigid body has constant density 1. If the rigid body has constant
// density D, then you need to multiply the output mass by D and the output inertia tensor by D.
void Viewer::calculateMassCenterInertia(Polyhedron &poly, double& massReturn, CGAL_Point &cmReturn, CGAL_Vector &inertiaReturn)
{
	const REALD mult[10] = { 1. / 6., 1. / 24., 1. / 24., 1. / 24., 1. / 60., 1. / 60., 1 / 60., 1. / 120., 1. / 120., 1. / 120. };
	REALD intg[10] = { 0., 0., 0., 0., 0., 0., 0., 0., 0., 0. }; // order: 1, x, y, z, x^2, y^2, z^2, xy, yz, zx

	for (Polyhedron::Facet_iterator fit = poly.facets_begin();
	fit != poly.facets_end(); ++fit)
	{
		CGAL_Halfedge_handle h = fit->halfedge();

		REALD x0 = CGAL::to_double(h->vertex()->point().x());
		REALD y0 = CGAL::to_double(h->vertex()->point().y());
		REALD z0 = CGAL::to_double(h->vertex()->point().z());
		h = h->next();

		REALD x1 = CGAL::to_double(h->vertex()->point().x());
		REALD y1 = CGAL::to_double(h->vertex()->point().y());
		REALD z1 = CGAL::to_double(h->vertex()->point().z());
		h = h->next();

		REALD x2 = CGAL::to_double(h->vertex()->point().x());
		REALD y2 = CGAL::to_double(h->vertex()->point().y());
		REALD z2 = CGAL::to_double(h->vertex()->point().z());

		// get edges and cross product of edges
		REALD a1 = x1 - x0;
		REALD b1 = y1 - y0;
		REALD c1 = z1 - z0;
		REALD a2 = x2 - x0;
		REALD b2 = y2 - y0;
		REALD c2 = z2 - z0;
		REALD d0 = b1*c2 - b2*c1;
		REALD d1 = a2*c1 - a1*c2;
		REALD d2 = a1*b2 - a2*b1;

		// compute integral terms
		REALD f1x, f2x, f3x, g0x, g1x, g2x, f1y, f2y, f3y, g0y, g1y, g2y, f1z, f2z, f3z, g0z, g1z, g2z;
		subexpressions_integral_terms(x0, x1, x2, f1x, f2x, f3x, g0x, g1x, g2x);
		subexpressions_integral_terms(y0, y1, y2, f1y, f2y, f3y, g0y, g1y, g2y);
		subexpressions_integral_terms(z0, z1, z2, f1z, f2z, f3z, g0z, g1z, g2z);

		// update integrals
		intg[0] += d0*f1x;
		intg[1] += d0*f2x;
		intg[2] += d1*f2y;
		intg[3] += d2*f2z;
		intg[4] += d0*f3x;
		intg[5] += d1*f3y;
		intg[6] += d2*f3z;
		intg[7] += d0*(y0*g0x + y1*g1x + y2*g2x);
		intg[8] += d1*(z0*g0y + z1*g1y + z2*g2y);
		intg[9] += d2*(x0*g0z + x1*g1z + x2*g2z);

	}

	//Multiple the coefficients
	for (int i = 0; i < 10; i++)
	{
		intg[i] *= mult[i];
	}

	//Mass of the body with a constant density 1
	REALD mass = intg[0];

	//Center of mass
	REALD cm_x = intg[1] / mass;
	REALD cm_y = intg[2] / mass;
	REALD cm_z = intg[3] / mass;

	//Inertia tensor relative to center of mass
	REALD inertia_xx = intg[5] + intg[6] - mass*(cm_y*cm_y + cm_z*cm_z);
	REALD inertia_yy = intg[4] + intg[6] - mass*(cm_z*cm_z + cm_x*cm_x);
	REALD inertia_zz = intg[4] + intg[5] - mass*(cm_x*cm_x + cm_y*cm_y);
	//REALD inertia_xy = -(intg[7]-mass*cm_x*cm_y);
	//REALD inertia_yz = -(intg[8]-mass*cm_y*cm_z);
	//REALD inertia_xz = -(intg[9]-mass*cm_z*cm_x);

	//Return the references values
	massReturn = fabs(mass);
	cmReturn = CGAL_Point(cm_x, cm_y, cm_z);
	inertiaReturn = CGAL_Vector(inertia_xx, inertia_yy, inertia_zz);
}

void Viewer::init()
{
	if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, 
		"Mode selection",
		"Yes for adjusting viewport state, no for rendering",
		QMessageBox::Yes|QMessageBox::No).exec()) 
	{
		count = -1;
		cout << "Yes selected" << endl;
	} 
	// else execute item selection for rendering
	else {
		QStringList items;
		items << tr("X") << tr("Y") << tr("Z");
		bool ok;
		QString item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
			tr("Season:"), items, 0, false, &ok);
		if (ok && !item.isEmpty())
		{
			if (item == QString("X")) rotating_axis[0] = 1;
			else if (item == QString("Y")) rotating_axis[0] = 2;
			else rotating_axis[0] = 3;
		}

		item = QInputDialog::getItem(this, tr("QInputDialog::getItem()"),
			tr("Season:"), items, 0, false, &ok);
		if (ok && !item.isEmpty()) {
			if (item == QString("X")) rotating_axis[1] = 1;
			else if (item == QString("Y")) rotating_axis[1] = 2;
			else rotating_axis[1] = 3;
		}


		count = 0;
		set_color_to_constant();
		read_center_of_mass(cmOrigin);
		cout << "No selected" << endl;
	}
	vec_now = 0; 
	vec_pos = 0;
	absolute_files.push_back(std::vector<QString>());
	read_files();
	restoreStateFromFile();
    //showEntireScene();
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glClearColor(1, 1, 1, 0);
	glPointSize(3.0);
	startAnimation();
	setSnapshotFormat(QString(FIGURE_FORMAT));
}

void Viewer::recursive_load(QString path) {
	QDir dir(path);
	while (absolute_files[vec_pos].size() != 0) {
		absolute_files.push_back(std::vector<QString>());
		++ vec_pos;
	}

	QFileInfoList list = dir.entryInfoList();
	for (auto i = 0; i < list.size(); ++i) {
		QFileInfo mfi = list.at(i);
		if (mfi.isFile() && mfi.suffix() == QString("off"))
		{
			absolute_files[vec_pos].push_back(mfi.absoluteFilePath());
		}
		else
		{
			if (mfi.fileName() == "." || mfi.fileName() == "..")continue;
			recursive_load(mfi.absoluteFilePath());
		}
	}
}

void Viewer::read_files(){
	// Read obj files
	#ifdef READ_FROM_DIALOGUE
		QString filename = QFileDialog::getOpenFileName( 
	    this, 
	    tr("Open Files"), 
	    "Resources", 
	    tr("All files (*.*)") );
		QString fileroot = filename.left(filename.lastIndexOf('/') + 1);



		recursive_load(fileroot);
		for (auto i = 0; i < absolute_files.size(); ++i){
			std::cout << "Layer " << i << std::endl;
			for (auto j = 0; j < absolute_files[i].size(); ++j) {
				std::cout << "  " << absolute_files[i][j].toStdString() << std::endl;
			}
		}

		return;	 // !!!!!!!!!!!!!!!! return
		
		QDir dir(fileroot);
	    if(!dir.exists()) return;
	    QFileInfoList list = dir.entryInfoList();
		std::cerr << "文件中有list: " << list.size() << std::endl;
	    for(auto i = 0 ; i < list.size(); ++i){
	    	QFileInfo fileinfo = list.at(i);
			std::cerr << "list " << i << " : " << fileinfo.absoluteFilePath().toStdString() << std::endl;
            if(fileinfo.suffix() == QString("off")){
                Polyhedron* mesh = new Polyhedron();
				std::ifstream in_file(fileinfo.absoluteFilePath().toStdString().c_str());
				in_file >> *mesh;
				cout << fileinfo.absoluteFilePath().toUtf8().constData() << endl;
                cout << "Size of vertices = " << mesh->size_of_vertices() << endl;
                
				// scaling 
				for (auto it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
					it->point() = Point_3(it->point().x() / 50, it->point().y() / 50, it->point().z() / 50);
				}
				
				meshList.push_back(mesh);
				in_file.close();
            }
	    }
    #else

    #endif
	cout << "Size of meshList = " << meshList.size() << endl;

}

void Viewer::render_mesh_list(){
	for(unsigned i = 0 ; i < meshList.size(); ++i){
		double rmReturn;
		CGAL_Point cmReturn;
		CGAL_Vector intertiaReturn;
		calculateMassCenterInertia(*meshList[i], rmReturn, cmReturn, intertiaReturn);
		CGAL_Vector mass_of_center(cmReturn.x(), cmReturn.y(), cmReturn.z());
// 		std::cout << cmOrigin << std::endl;
// 		system("pause");
		mass_of_center = mass_of_center- cmOrigin;
		//double dis_cc = CGAL::sqrt(mass_of_center.squared_length());
		//mass_of_center = mass_of_center / CGAL::sqrt(mass_of_center.squared_length());
		mass_of_center = 0.18  * mass_of_center;

        glColor3ub(color[i][0],color[i][1],color[i][2]);
		for (Polyhedron::Facet_iterator fit = meshList[i]->facets_begin();
		fit != meshList[i]->facets_end(); fit++)
		{
			//Polyhedron::Traits::Vector_3 face_normal = compute_facet_normal<Polyhedron::Facet, Polyhedron::Traits>(*fit);
			//glNormal3d(face_normal.x(), face_normal.y(), face_normal.z());
			glBegin(GL_TRIANGLES);
			Polyhedron::Halfedge_handle h = fit->halfedge();
			for (int i = 0; i < 3; i++)
			{
				auto vertex_normal = compute_vertex_normal<Polyhedron::Vertex,Kernel>(*h->vertex());
				glNormal3d(vertex_normal.x(), vertex_normal.y(), vertex_normal.z());
				const Kernel::Point_3 vert = h->vertex()->point();
				glVertex3d(vert.x() + mass_of_center.x(), vert.y() + mass_of_center.y() , vert.z() + mass_of_center.z());
				h = h->next();
			}
			glEnd();
		}
		/*
		for(unsigned j = 0 ; j < meshList[i]->m_nFace; ++j){
            auto& normal = meshList[i]->m_pFace[j].m_vNormal;
            glNormal3f(normal.x, normal.y, normal.z);
            glBegin(GL_TRIANGLES);
                for(unsigned k = 0; k < 3; ++k){
                    auto& pos = meshList[i]->m_pVertex[meshList[i]->m_pFace[j].m_piVertex[k]].m_vPosition;
                    glVertex3f(pos.x, pos.y, pos.z);
                }
            glEnd();
        }
		*/
	}
}

void Viewer::draw()
{
	switch(count){
		case 0:
			if (vec_now < absolute_files.size()) {
				for (auto i = 0; i < meshList.size(); ++i) {
					delete meshList[i];
				}
				meshList.clear();
				if (!absolute_files[vec_now].empty()) {
					QString basefile = absolute_files[vec_now][0];
					int slash_end = basefile.lastIndexOf('/'), slash_begin;
					
					for (auto slash = slash_end - 1; slash >= 0; --slash) {
						if (basefile.at(slash) == '/') {
							slash_begin = slash;
							break;
						}
					}
					QString fileroot = basefile.mid(slash_begin + 1, slash_end - slash_begin - 1);
					//qDebug() << fileroot;
					setSnapshotFileName(fileroot);
					setSnapshotCounter(0);
					for (auto i = 0; i < absolute_files[vec_now].size(); ++i) {
						Polyhedron* mesh = new Polyhedron();
						std::ifstream in_file(absolute_files[vec_now][i].toStdString().c_str());
						in_file >> *mesh;

						// scaling 
						for (auto it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
							it->point() = Point_3(it->point().x() / 50, it->point().y() / 50, it->point().z() / 50);
						}

						meshList.push_back(mesh);
						in_file.close();
					}
					++vec_now;
					++count;
				}
				else {
					++vec_now;
					count = 0;
				}
			}
			else {
				render_mesh_list();
			}
			break;
		case 1:
			render_mesh_list();
			saveSnapshot(true);
			++count;
			break;
		case 2:
			glPushMatrix();
			if(rotating_axis[0] == 1) glRotatef(90.0, 1, 0, 0);
			else if(rotating_axis[0] == 2) glRotatef(90.0, 0, 1, 0);
			else glRotatef(90.0, 0, 0, 1);
		 	render_mesh_list();
			glPopMatrix();
			saveSnapshot(true);
			++count;
			break;
		case 3:
			glPushMatrix();
			if (rotating_axis[0] == 1) glRotatef(180.0, 1, 0, 0);
			else if (rotating_axis[0] == 2) glRotatef(180.0, 0, 1, 0);
			else glRotatef(180.0, 0, 0, 1);
		 	render_mesh_list();
			glPopMatrix();
			saveSnapshot(true);
			++count;
			break;	
		case 4:
			glPushMatrix();
			if (rotating_axis[0] == 1) glRotatef(270.0, 1, 0, 0);
			else if (rotating_axis[0] == 2) glRotatef(270.0, 0, 1, 0);
			else glRotatef(270.0, 0, 0, 1);
			render_mesh_list();
			glPopMatrix();
			saveSnapshot(true);
			++count;
			break;
		case 5:
			glPushMatrix();
			if (rotating_axis[1] == 1) glRotatef(90.0, 1, 0, 0);
			else if (rotating_axis[1] == 2) glRotatef(90.0, 0, 1, 0);
			else glRotatef(90.0, 0, 0, 1);
			render_mesh_list();
			glPopMatrix();
			saveSnapshot(true);
			++count;
			break;
		case 6:
			glPushMatrix();
			if (rotating_axis[1] == 1) glRotatef(270.0, 1, 0, 0);
			else if (rotating_axis[1] == 2) glRotatef(270.0, 0, 1, 0);
			else glRotatef(270.0, 0, 0, 1);
			render_mesh_list();
			glPopMatrix();
			saveSnapshot(true);
			count = 0;
			break;
		case -1:
			for (auto i = 0; i < absolute_files[vec_now].size(); ++i) {
				Polyhedron* mesh = new Polyhedron();
				std::ifstream in_file(absolute_files[vec_now][i].toStdString().c_str());
				in_file >> *mesh;

#ifdef FILTER_RECENTER
				recenter_and_save(mesh);
				exit(-1);
#endif
				// scaling 
				for (auto it = mesh->vertices_begin(); it != mesh->vertices_end(); ++it) {
					it->point() = Point_3(it->point().x() / 50, it->point().y() / 50, it->point().z() / 50);
				}

				double rmReturn;
				CGAL_Point cmReturn;
				CGAL_Vector intertiaReturn;
				calculateMassCenterInertia(*mesh, rmReturn, cmReturn, intertiaReturn);
				CGAL_Vector mass_of_center(cmReturn.x(), cmReturn.y(), cmReturn.z());
				save_center_of_mass(mass_of_center);

				meshList.push_back(mesh);
				in_file.close();
			}
			count = -2;
			break;
		case -2:
			render_mesh_list();
			break;
		default:
			break;
	}
}

void Viewer::animate()
{

}

QString Viewer::helpString() const
{
  QString text("<h2>R e n d e r</h2>");
  text += "Use the <i>render_images()</i> function to render partitioned models to images. ";
  return text;
}
