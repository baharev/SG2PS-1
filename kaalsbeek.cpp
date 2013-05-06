// Copyright (C) 2012, 2013 Ágoston Sasvári
// All rights reserved.
// This code is published under the GNU Lesser General Public License.

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <vector>

#include "common.h"
#include "kaalsbeek.hpp"
#include "structs.h"

using namespace std;

vector <VCTR> generate_arc (size_t SEG_CNT, size_t ARC_CNT) {

	VCTR buf;
	vector <VCTR> arc;

	DIPDIR_DIP DD;

	DD.DIPDIR = (SEG_CNT * 60.0) + 30.0 ;

	if (DD.DIPDIR > 360.0) DD.DIPDIR = DD.DIPDIR - 360.0;
	else {}

	DD.DIP = ARC_CNT * 9.0;

	size_t cnt_max = 10 - ARC_CNT;

	double step = 60.0 / cnt_max;

	for (size_t p_cnt = 0; p_cnt < cnt_max; p_cnt++) {

		buf = DXDYDZ_from_dipdir_dip(DD);

		arc.push_back(buf);

		DD.DIPDIR = DD.DIPDIR + step;
	}

	return arc;
}

vector <vector <VCTR> > generate_segment (size_t SEG_CNT) {

	vector <VCTR > buf;
	vector <vector <VCTR> > out;

	for (size_t arc_cnt = 0; arc_cnt < 10; arc_cnt++) {

		buf = generate_arc(SEG_CNT, arc_cnt);

		out.push_back(buf);
	}

	return out;
}

vector <TRIANGLE>  generate_net (vector <GDB> inGDB, INPSET inset) {

	vector <vector <VCTR> > buf;
	vector <vector <vector <VCTR> > > net;

	for (size_t seg_cnt = 0; seg_cnt < 6; seg_cnt++) {

		buf = generate_segment(seg_cnt);

		net.push_back(buf);
	}

	vector <TRIANGLE> TRI = generate_triangle (net, inset);

	TRI = return_count_in_net (inGDB, TRI);

	//dbg_cout_kaalsbeek_ps(TRI);

	return TRI;
}

vector <GRID_CENTER> generate_triangle_center (vector <TRIANGLE> net) {

	vector <GRID_CENTER> out;

	for (size_t i = 0; i < net.size(); i++) {

		GRID_CENTER buf;

		buf.CENTER.X = (net.at(i).A.X + net.at(i).B.X + net.at(i).C.X) / 3.0;
		buf.CENTER.Y = (net.at(i).A.Y + net.at(i).B.Y + net.at(i).C.Y) / 3.0;
		buf.CENTER.Z = (net.at(i).A.Z + net.at(i).B.Z + net.at(i).C.Z) / 3.0;

		buf.COUNT = net.at(i).COUNT;

		double X = buf.CENTER.X;
		double Y = buf.CENTER.Y;

		if (sqrt((X * X) + (Y * Y)) <= 1.0) out.push_back(buf);
		else {}
	}

	return out;
}

vector <GRID_CENTER> reduce_triangle_center (vector <GRID_CENTER> in) {

	vector <GRID_CENTER> out;

	for (size_t i = 0; i < in.size(); i++) {

		if (in.at(i).COUNT > 0) out.push_back(in.at(i));
		else {}
	}

	return out;
}

/*double round(double d)
{
  return floor(d + 0.5);
}
*/

vector <TRIANGLE> merge_triangle (vector <TRIANGLE> target, vector <TRIANGLE> record) {

	TRIANGLE buf;

	for (size_t i = 0; i < record.size(); i++) {

		buf = record.at(i);

		target.push_back(buf);
	}

	return target;
}

VCTR create_offnet_point (VCTR A, VCTR B) {

	VCTR out;

	out.X = ((A.X + B.X) / 2.0) * 1.083;
	out.Y = ((A.Y + B.Y) / 2.0) * 1.083;
	out.Z = 999.99;

	return out;
}

//6
vector <TRIANGLE> generate_triangle_offnet(vector <vector <vector <VCTR> > > net, size_t SEG_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	for (size_t p_cnt = 0; p_cnt < 9; p_cnt++) {

		buf.A = net.at(SEG_CNT).at(0).at(p_cnt + 0);
		buf.B = net.at(SEG_CNT).at(0).at(p_cnt + 1);
		buf.C = create_offnet_point (buf.A, buf.B);

		buf.GROUP = 6;
		buf.COUNT = 0;

		//dbg_cout_triangle ("6 - OFFNET", buf.A, buf.B, buf.C, SEG_CNT, 0, p_cnt + 0, SEG_CNT, 0, p_cnt + 1, 33, 33, 33);

		out.push_back(buf);
	}

	return out; // OK
}

//7
vector <TRIANGLE> generate_triangle_offnet_between_segments (vector <vector <vector <VCTR> > > net, size_t SEG_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	size_t SEG_L = 0;
	size_t SEG_U = 0;

	if (SEG_CNT == 5) {

		SEG_L = SEG_CNT;
		SEG_U = 0;
	}
	else {

		SEG_L = SEG_CNT;
		SEG_U = SEG_CNT + 1;
	}

	buf.A = net.at(SEG_L).at(0).at(9);
	buf.B = net.at(SEG_U).at(0).at(0);
	buf.C = create_offnet_point (buf.A, buf.B);

	buf.GROUP = 7;
	buf.COUNT = 0;

	//dbg_cout_triangle ("7 - OFF, BTW SEG", buf.A, buf.B, buf.C, SEG_L, 0, 9, SEG_U, 0, 0, 55, 55, 55);

	out.push_back(buf);

	return out; // OK
}

vector <TRIANGLE> generate_triangle_in_arc (vector <vector <vector <VCTR> > > net, size_t SEG_CNT, size_t ARC_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	size_t cnt_max = 10 - ARC_CNT - 1;

	for (size_t p_cnt = 0; p_cnt < cnt_max; p_cnt++) {

		buf.A = net.at(SEG_CNT).at(ARC_CNT + 0).at(p_cnt + 1);
		buf.B = net.at(SEG_CNT).at(ARC_CNT + 0).at(p_cnt + 0);
		buf.C = net.at(SEG_CNT).at(ARC_CNT + 1).at(p_cnt + 0);

		buf.GROUP = 1;
		buf.COUNT = 0;


		//dbg_cout_triangle ("1 - IN_ARC_I", buf.A, buf.B, buf.C, SEG_CNT, ARC_CNT + 0, p_cnt + 1, SEG_CNT, ARC_CNT + 0, p_cnt + 0, SEG_CNT, ARC_CNT + 1, p_cnt + 0);

		out.push_back(buf);
	}

	return out; //OK
}

vector <TRIANGLE> generate_triangle_in_arc_II (vector <vector <vector <VCTR> > > net, size_t SEG_CNT, size_t ARC_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	size_t cnt_max = 10 - ARC_CNT - 2;

	for (size_t p_cnt = 0; p_cnt < cnt_max; p_cnt++) {

		buf.A = net.at(SEG_CNT).at(ARC_CNT + 0).at(p_cnt + 1);
		buf.B = net.at(SEG_CNT).at(ARC_CNT + 1).at(p_cnt + 0);
		buf.C = net.at(SEG_CNT).at(ARC_CNT + 1).at(p_cnt + 1);

		buf.GROUP = 2;
		buf.COUNT = 0;

		//dbg_cout_triangle ("2 - IN_ARC_II",	buf.A, buf.B, buf.C, SEG_CNT, ARC_CNT + 0, p_cnt + 1, SEG_CNT, ARC_CNT + 1, p_cnt + 0, SEG_CNT, ARC_CNT + 1, p_cnt + 1);

		out.push_back(buf);
	}

	return out; // OK
}

vector <TRIANGLE> generate_triangle_between_arcs (vector <vector <vector <VCTR> > > net, size_t SEG_CNT, size_t ARC_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	size_t p_max = net.at(SEG_CNT).at(ARC_CNT).size() - 1;

	size_t SEG_L = 0;
	size_t SEG_U = 0;

	if (SEG_CNT == 5) {

		SEG_L = SEG_CNT;
		SEG_U = 0;
	}
	else {

		SEG_L = SEG_CNT;
		SEG_U = SEG_CNT + 1;
	}

	buf.A = net.at(SEG_U).at(ARC_CNT + 0).at(0);
	buf.B = net.at(SEG_L).at(ARC_CNT + 0).at(p_max);
	buf.C = net.at(SEG_U).at(ARC_CNT + 1).at(0);

	buf.GROUP = 3;
	buf.COUNT = 0;

	//dbg_cout_triangle ("3 - BTW_ARC_I",	buf.A, buf.B, buf.C, SEG_U, ARC_CNT + 0, 0, SEG_L, ARC_CNT + 0, p_max, SEG_U, ARC_CNT + 1, 0);

	out.push_back(buf);

	buf.A = net.at(SEG_U).at(ARC_CNT + 1).at(0);
	buf.B = net.at(SEG_L).at(ARC_CNT + 0).at(p_max);
	buf.C = net.at(SEG_L).at(ARC_CNT + 1).at(p_max - 1);

	buf.GROUP = 4;
	buf.COUNT = 0;

	//dbg_cout_triangle ("4 - BTW_ARC_II", buf.A, buf.B, buf.C, SEG_U, ARC_CNT + 1, 0, SEG_L, ARC_CNT + 0, p_max, SEG_L, ARC_CNT + 1, p_max - 1);

	out.push_back(buf);

	return out; // OK
}

vector <TRIANGLE> generate_central_triangles (vector <vector <vector <VCTR> > > net, size_t SEG_CNT) {

	TRIANGLE buf;
	vector <TRIANGLE> out;

	size_t SEG_L = 0;
	size_t SEG_U = 0;

	if (SEG_CNT == 5) {

		SEG_L = SEG_CNT;
		SEG_U = 0;
	}
	else {

		SEG_L = SEG_CNT;
		SEG_U = SEG_CNT + 1;
	}

	buf.A = net.at(SEG_U).at(9).at(0);
	buf.B = net.at(SEG_L).at(9).at(0);
	buf.C = declare_vector (0.0, 0.0, 1.0);

	buf.GROUP = 5;
	buf.COUNT = 0;

	//dbg_cout_triangle ("5 - CENTRAL", buf.A, buf.B, buf.C, SEG_U, 9, 0, SEG_L, 9, 0, 11, 11, 11);

	out.push_back(buf);

	return out; // OK
}

vector <TRIANGLE> generate_triangle_in_segment (vector <vector <vector <VCTR> > > net, size_t SEG_CNT) {

	vector <TRIANGLE> buf;
	vector <TRIANGLE> out;

	for (size_t arc_cnt = 0; arc_cnt < 9; arc_cnt++) {

		//1:
		buf = generate_triangle_in_arc(net, SEG_CNT, arc_cnt);
		out = merge_triangle(out, buf);

		//2:
		buf = generate_triangle_in_arc_II(net, SEG_CNT, arc_cnt);
		out = merge_triangle(out, buf);

		//3 + 4:
		buf = generate_triangle_between_arcs (net, SEG_CNT, arc_cnt);// 3+4
		out = merge_triangle(out, buf);
	}

	return out;
}

vector <TRIANGLE> generate_triangle (vector <vector <vector <VCTR> > > net, INPSET inset) {

	vector <TRIANGLE> buf;
	vector <TRIANGLE> out;

	for (size_t seg_cnt = 0; seg_cnt < 6; seg_cnt++) {

		// 1 + 2 + 3 + 4
		buf = generate_triangle_in_segment (net, seg_cnt);
		out = merge_triangle(out, buf);

		//5:
		buf = generate_central_triangles (net, seg_cnt);
		out = merge_triangle(out, buf);

		//6
		buf = generate_triangle_offnet(net, seg_cnt);
		out = merge_triangle(out, buf);

		//7
		buf =  generate_triangle_offnet_between_segments (net, seg_cnt);
		out = merge_triangle(out, buf);
	}

	//dbg_cout_triangle_coordinates (out);

	//out = increase_triange_density(out);

	//out = increase_triange_density(out);

	out = convert_S_W_net (out, inset);

	return out;
}

vector <TRIANGLE> convert_S_W_net (vector <TRIANGLE> in, INPSET inset) {

	for (size_t i = 0; i < in.size(); i++) {

		if (in.at(i).GROUP < 6) {

			if (inset.plottype == "W") {

				if (in.at(i).A.Z < 1.00) {

					in.at(i).A.X = in.at(i).A.X / (1.00 - in.at(i).A.Z);
					in.at(i).A.Y = in.at(i).A.Y / (1.00 - in.at(i).A.Z);
				}

				if (in.at(i).B.Z < 1.00) {

					in.at(i).B.X = in.at(i).B.X / (1.00 - in.at(i).B.Z);
					in.at(i).B.Y = in.at(i).B.Y / (1.00 - in.at(i).B.Z);
				}

				if (in.at(i).C.Z < 1.00) {

					in.at(i).C.X = in.at(i).C.X / (1.00 - in.at(i).C.Z);
					in.at(i).C.Y = in.at(i).C.Y / (1.00 - in.at(i).C.Z);
				}
			}
			else {

				if (in.at(i).A.Z < 1.00) {

					in.at(i).A.X = in.at(i).A.X / sqrt (1.00 - in.at(i).A.Z);
					in.at(i).A.Y = in.at(i).A.Y / sqrt (1.00 - in.at(i).A.Z);
				}

				if (in.at(i).B.Z < 1.00) {

					in.at(i).B.X = in.at(i).B.X / sqrt (1.00 - in.at(i).B.Z);
					in.at(i).B.Y = in.at(i).B.Y / sqrt (1.00 - in.at(i).B.Z);
				}

				if (in.at(i).C.Z < 1.00) {

					in.at(i).C.X = in.at(i).C.X / sqrt (1.00 - in.at(i).C.Z);
					in.at(i).C.Y = in.at(i).C.Y / sqrt (1.00 - in.at(i).C.Z);
				}
			}
		}
	}

	return in;
}

vector <TRIANGLE> increase_triange_density (vector <TRIANGLE> in) {

	vector <TRIANGLE> out;
	TRIANGLE buf;

	for (size_t i = 0; i < in.size(); i++) {

		buf.GROUP = in.at(i).GROUP;
		buf.A = in.at(i).A;
		buf.B = declare_vector (
				(in.at(i).A.X + in.at(i).B.X) / 2.0,
				(in.at(i).A.Y + in.at(i).B.Y) / 2.0,
				(in.at(i).A.Z + in.at(i).B.Z) / 2.0
				);
		buf.C = declare_vector (
				(in.at(i).A.X + in.at(i).C.X) / 2.0,
				(in.at(i).A.Y + in.at(i).C.Y) / 2.0,
				(in.at(i).A.Z + in.at(i).C.Z) / 2.0
		);
		out.push_back(buf);


		buf.A = in.at(i).B;
		buf.B = declare_vector (
				(in.at(i).B.X + in.at(i).C.X) / 2.0,
				(in.at(i).B.Y + in.at(i).C.Y) / 2.0,
				(in.at(i).B.Z + in.at(i).C.Z) / 2.0
		);
		buf.C = declare_vector (
				(in.at(i).A.X + in.at(i).B.X) / 2.0,
				(in.at(i).A.Y + in.at(i).B.Y) / 2.0,
				(in.at(i).A.Z + in.at(i).B.Z) / 2.0
		);
		out.push_back(buf);

		buf.A = in.at(i).C;
		buf.B = declare_vector (
				(in.at(i).C.X + in.at(i).A.X) / 2.0,
				(in.at(i).C.Y + in.at(i).A.Y) / 2.0,
				(in.at(i).C.Z + in.at(i).A.Z) / 2.0
		);
		buf.C = declare_vector (
				(in.at(i).B.X + in.at(i).C.X) / 2.0,
				(in.at(i).B.Y + in.at(i).C.Y) / 2.0,
				(in.at(i).B.Z + in.at(i).C.Z) / 2.0
		);
		out.push_back(buf);

		buf.A = declare_vector (
				(in.at(i).A.X + in.at(i).B.X) / 2.0,
				(in.at(i).A.Y + in.at(i).B.Y) / 2.0,
				(in.at(i).A.Z + in.at(i).B.Z) / 2.0
		);
		buf.B = declare_vector (
				(in.at(i).B.X + in.at(i).C.X) / 2.0,
				(in.at(i).B.Y + in.at(i).C.Y) / 2.0,
				(in.at(i).B.Z + in.at(i).C.Z) / 2.0
		);
		buf.C = declare_vector (
				(in.at(i).A.X + in.at(i).C.X) / 2.0,
				(in.at(i).A.Y + in.at(i).C.Y) / 2.0,
				(in.at(i).A.Z + in.at(i).C.Z) / 2.0
		);
		out.push_back(buf);
	}

	return out;
}

bool is_data_in_triangle (TRIANGLE in, VCTR D) {

	double AX = in.A.X;
	double AY = in.A.Y;
	double BX = in.B.X;
	double BY = in.B.Y;
	double CX = in.C.X;
	double CY = in.C.Y;

	vector <vector < double > > test = declare_3x3_matrix(
			AX - D.X, AY - D.Y, (AX * AX - D.X * D.X) + (AY * AY - D.Y * D.Y),
			BX - D.X, BY - D.Y, (BX * BX - D.X * D.X) + (BY * BY - D.Y * D.Y),
			CX - D.X, CY - D.Y, (CX * CX - D.X * D.X) + (CY * CY - D.Y * D.Y)
	);

	return (det_3 (test) > 0.0);
}

bool is_neighbouring_internal_triange (TRIANGLE inTRI, TRIANGLE offTRI) {

	size_t fit_counter = 0;

	if (points_distance (inTRI.A, offTRI.A) < 10e-4) fit_counter++;
	if (points_distance (inTRI.A, offTRI.B) < 10e-4) fit_counter++;
	if (points_distance (inTRI.A, offTRI.C) < 10e-4) fit_counter++;
	if (points_distance (inTRI.B, offTRI.A) < 10e-4) fit_counter++;
	if (points_distance (inTRI.B, offTRI.B) < 10e-4) fit_counter++;
	if (points_distance (inTRI.B, offTRI.C) < 10e-4) fit_counter++;
	if (points_distance (inTRI.C, offTRI.A) < 10e-4) fit_counter++;
	if (points_distance (inTRI.C, offTRI.B) < 10e-4) fit_counter++;
	if (points_distance (inTRI.C, offTRI.C) < 10e-4) fit_counter++;

	if (fit_counter == 2) return true;
	else return false;
}

vector <TRIANGLE> add_external_to_internal (vector <TRIANGLE> innet, TRIANGLE offnet) {

	for (size_t i = 0; i < innet.size(); i++) {

		if (is_neighbouring_internal_triange (innet.at(i), offnet)) innet.at(i).COUNT++;
	}

	return innet;
}

vector <TRIANGLE> return_count_in_net (vector <GDB> inGDB, vector <TRIANGLE> innet) {

	for (size_t i = 0; i < innet.size(); i++) {

		size_t counter = 0;

		for (size_t j = 0; j < inGDB.size(); j++) {

			DIPDIR_DIP DD = dipdir_dip_from_DXDYDZ(inGDB.at(j).D);

			if (DD.DIPDIR < 180.0) DD.DIPDIR = DD.DIPDIR + 180.0;
			else DD.DIPDIR = DD.DIPDIR - 180.0;

			DD.DIP = 90.0 - DD.DIP;

			VCTR pole = DXDYDZ_from_dipdir_dip(DD);

			if (is_data_in_triangle(innet.at(i), pole)) {

				if (innet.at(i).GROUP < 6) counter++;
				else innet = add_external_to_internal (innet, innet.at(i));
			}
		}

		innet.at(i).COUNT = counter;
	}

	return innet;
}

void dbg_cout_triangle (string method, VCTR A, VCTR B, VCTR C, size_t SC1, size_t AC1, size_t PC1, size_t SC2, size_t AC2, size_t PC2, size_t SC3, size_t AC3, size_t PC3) {

	cout << fixed << setprecision(4) << endl;

	cout
	<< method << ":" << '\t'
	<< A.X << '\t' << A.Y << '\t'
	<< B.X << '\t' << B.Y << '\t'
	<< C.X << '\t' << C.Y << '\t'
	<< SC1 << AC1 << PC1 << '\t'
	<< SC2 << AC2 << PC2 << '\t'
	<< SC3 << AC3 << PC3 << endl;
}

void dbg_cout_triangle_coordinates (vector <TRIANGLE> in) {

	for (size_t i = 0; i < in.size(); i++) {

		cout
		<< in.at(i).A.X << '\t' << in.at(i).A.Y
		<< in.at(i).B.X << '\t' << in.at(i).B.Y
		<< in.at(i).C.X << '\t' << in.at(i).C.Y
		<< endl;
	}
}

void dbg_cout_kaalsbeek_ps (vector <TRIANGLE> in) {

	string filename = "KAALSBEEK.PS";

	ofstream o(filename.c_str());

	o << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
	o << "%%BoundingBox:  0 0 440 440" << endl;

	for (size_t i = 0; i < in.size(); i++) {

		o
		<< "  newpath  "
		<< in.at(i).A.X * 200.0 + 220.0 << "  " << in.at(i).A.Y * 200.0 + 220.0 << " moveto "
		<< in.at(i).B.X * 200.0 + 220.0 << "  " << in.at(i).B.Y * 200.0 + 220.0 << " lineto" << endl;

		o
		<< in.at(i).C.X * 200.0 + 220.0 << "  " << in.at(i).C.Y * 200.0 + 220.0 << " lineto" << endl;

		if 		(in.at(i).GROUP == 1) o << " 0.50 0.50 0.50 " << flush;
		else if (in.at(i).GROUP == 2) o << " 0.50 0.50 1.00 " << flush;
		else if (in.at(i).GROUP == 3) o << " 1.00 0.50 0.83 " << flush;
		else if (in.at(i).GROUP == 4) o << " 1.00 0.50 0.50 " << flush;
		else if (in.at(i).GROUP == 5) o << " 1.00 0.75 0.50 " << flush;
		else if (in.at(i).GROUP == 6) o << " 1.00 1.00 0.50 " << flush;
		else if (in.at(i).GROUP == 7) o << " 0.50 1.00 0.50 " << flush;

		o << " setrgbcolor fill stroke " << endl;

		o << " 1 0 0 setrgbcolor 0.5 setlinewidth " << endl;

		o
		<< "  newpath  "
		<< in.at(i).A.X * 200.0 + 220.0 << "  " << in.at(i).A.Y * 200.0 + 220.0 << " moveto "
		<< in.at(i).B.X * 200.0 + 220.0 << "  " << in.at(i).B.Y * 200.0 + 220.0 << " lineto" << endl;

		o
		<< in.at(i).C.X * 200.0 + 220.0 << "  " << in.at(i).C.Y * 200.0 + 220.0 << " lineto " << endl;

		o
		<< in.at(i).A.X * 200.0 + 220.0 << "  " << in.at(i).A.Y * 200.0 + 220.0 << " lineto stroke" << endl;

		double X = ((in.at(i).A.X + in.at(i).B.X + in.at(i).C.X) / 3.0) * 200 + 220;
		double Y = ((in.at(i).A.Y + in.at(i).B.Y + in.at(i).C.Y) / 3.0) * 200 + 220;

		o << "/ArialNarrow-Bold findfont 4 scalefont setfont" << endl;

		o << " 0 0 0 setrgbcolor " << endl;

		o
		<< "  " << X - 2<< " " << Y - 2 << " moveto ("
		<< in.at(i).COUNT << ") show "<< endl;
	}
}