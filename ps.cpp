
// Copyright (C) 2012 - 2015 Ágoston Sasvári
// All rights reserved.
// This code is published under the GNU Lesser General Public License.

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "allowed_keys.hpp"
#include "assertions.hpp"
#include "color_management.hpp"
#include "data_io.h"
#include "density.h"
#include "filename.hpp"
#include "homogenity_check.hpp"
#include "kaalsbeek.hpp"
#include "ps.h"
#include "rgf.h"
#include "common.h"
#include "platform_dep.hpp"
#include "ps_RUP_ANG.hpp"
#include "rose.h"
#include "run_mode.h"
#include "rup_clustering.hpp"
#include "standard_output.hpp"
#include "settings.hpp"
#include "stresstensor.hpp"
#include "valley_method.hpp"

using namespace std;

namespace {

const string AV_GRY_CLR = "0.20 0.20 0.20";
const string AV_RGB_CLR = "1.00 0.00 0.00";
const string AVO_GRY_CLR = "0.40 0.40 0.40";
const string AVO_RGB_CLR = "0.80 0.00 0.20";
const string OTB_GRY_CLR = "0.00 0.00 0.00";
const string OTB_RGB_CLR = "0.00 0.00 0.00";
const string FLD_GRY_CLR = "0.60 0.60 0.60";
const string FLD_RGB_CLR = "0.00 0.00 1.00";
const string C_GRY_CLR = "0.80 0.80 0.80";
const string C_RGB_CLR = "0.80 0.80 0.80";

const double AV_GRY_LNW = 2.0;
const double AV_RGB_LNW = 1.5;
const double AVO_GRY_LNW = 2.0;
const double AVO_RGB_LNW = 1.5;
const double OTB_GRY_LNW = 1.5;
const double OTB_RGB_LNW = 1.5;
const double FLD_GRY_LNW = 2.0;
const double FLD_RGB_LNW = 1.5;
const double C_GRY_LNW = 2.0;
const double C_RGB_LNW = 1.5;

const string AV_GRY_DSH = "   ";
const string AV_RGB_DSH = "   ";
const string AVO_GRY_DSH = "6 6";
const string AVO_RGB_DSH = "6 6";
const string OTB_GRY_DSH = "6 6";
const string OTB_RGB_DSH = "6 6";
const string FLD_GRY_DSH = "6 6";
const string FLD_RGB_DSH = "6 6";
const string C_GRY_DSH = "6  6";
const string C_RGB_DSH = "6  6";
}

PAPER PS_dimensions (const bool WELL) {

	PAPER P;

	if (WELL) {

		P.Y = mm_to_point (420);
		P.X = P.Y * 0.707143;
	}
	else {

		P.X = mm_to_point (420);
		P.Y = P.X * 0.707143;
	}

	P.A = P.X * 0.033670;
	P.B = P.A / 2.0;
	P.C = P.B / 2.0;
	P.D = P.C / 10.0122;

	P.R = P.X * 0.235726  * 0.5;

	P.O1X = 1.5 * P.A  + P.R;
	P.O1Y = P.Y - P.A - P.A - P.B - P.R;

	P.O2X = P.O1X;
	P.O2Y = P.O1X + P.B;

	P.O3X = P.O1X + P.R + P.B + P.A + P.R + 3.0 * P.B;
	P.O3Y = P.O1Y;

	P.O4X = P.O3X;
	P.O4Y = P.O2Y;

	P.O5X = P.O3X + P.R + 0.5 * P.B + P.A;
	P.O5Y = P.O3Y + P.R;

	P.O6X = P.O5X;
	P.O6Y = P.O4Y;

	P.O7X = P.O5X + P.R - 0.0 * P.B;
	P.O7Y = P.O3Y + P.B;

	P.O8X = P.O7X;
	P.O8Y = P.O6Y - P.R + P.B;





	P.S1X = P.X - (10.0 * P.A);
	P.S1Y = (P.Y / 2.0) + (3.5 * P.A);

	P.S2X = P.X - (1.00 * P.A);
	P.S2Y = P.S1Y;

	P.S3X = P.X - (1.00 * P.A);
	P.S3Y = (P.Y / 2.0) - (3.5 * P.A);

	P.S4X = P.S1X;
	P.S4Y = P.S3Y;

	P.S5X = P.S1X + 3.0 * P.A;
	P.S5Y = P.S1Y;

	P.S6X = P.S2X - 3.0 * P.A;
	P.S6Y = P.S1Y;

	return P;
}

void PS_stereonet_header (ofstream& o) {

	const string filename = return_ACTUAL_LOCATION() + "_" + return_ACTUAL_DATATYPE() + ".EPS";

	o << "%!PS-Adobe-3.0 EPSF-3.0" << '\n';
	o << "%%BoundingBox:  0 0 1191 842" << '\n';
	o << "%%Title: " << filename << '\n';
	o << "%%Creator: SG2PS" ;

	if (!(is_mode_DEBUG())) o << " (" << version_id() << ")" ;

	o << '\n';
	o << "%%EndComments" << '\n' << '\n';
	o << "<</PageSize [ 1191 842 ]>> setpagedevice " << '\n';

	const bool FRACTURE_TO_PROCESS = is_BINGHAM_USE() && return_ACTUAL_DATATYPE() == "FRACTURE";
	const bool IS_STRIAE = is_allowed_striae_datatype (return_ACTUAL_DATATYPE());
	const bool STRIAE_TO_PROCESS = !is_INVERSION_NONE() && IS_STRIAE;

	if (!FRACTURE_TO_PROCESS && !STRIAE_TO_PROCESS) return;

	text_PS (o, "/extension_arrow {");
	newpath_PS (o);
	moveto_PS (o,  -3.0,  0.0, 3);
	rlineto_PS (o,  0.0,  5.0, 3);
	rlineto_PS (o, -2.0,  0.0, 3);
	rlineto_PS (o,  5.0,  5.0, 3);
	rlineto_PS (o,  5.0, -5.0, 3);
	rlineto_PS (o, -2.0,  0.0, 3);
	rlineto_PS (o,  0.0, -5.0, 3);
	closepath_PS (o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1.0, 1);
	stroke_PS (o);
	text_PS(o, "} def");

	text_PS (o, "/compression_arrow {" );
	newpath_PS (o);
	moveto_PS (o, -5.0,  5.0, 3);
	rlineto_PS (o,  2.0,  0.0, 3);
	rlineto_PS (o,  0.0,  5.0, 3);
	rlineto_PS (o,  6.0,  0.0, 3);
	rlineto_PS (o,  0.0, -5.0, 3);
	rlineto_PS (o,  2.0,  0.0, 3);
	rlineto_PS (o, -5.0, -5.0, 3);
	closepath_PS (o);
	color_PS (o, "0.0 0.0 0.0");
	fill_PS (o);
	stroke_PS (o);
	text_PS (o, "} def");

	vector <string> TITLE;
	vector <string> RGB_CLR;
	vector <string> GRY_CLR;

	TITLE.push_back ("/s1_axis {");
	GRY_CLR.push_back ("0.0 0.0 0.0");
	RGB_CLR.push_back ("1.0 0.0 0.0");
	TITLE.push_back ("/s1_iter_axis {");
	GRY_CLR.push_back ("0.0 0.0 0.0");
	RGB_CLR.push_back ("1.0 0.0 0.0");

	TITLE.push_back ("/s2_axis {");
	GRY_CLR.push_back ("0.5 0.5 0.5");
	RGB_CLR.push_back ("0.0 1.0 0.0");
	TITLE.push_back ("/s2_iter_axis {");
	GRY_CLR.push_back ("0.5 0.5 0.5");
	RGB_CLR.push_back ("0.0 1.0 0.0");

	TITLE.push_back ("/s3_axis {");
	GRY_CLR.push_back ("1.0 1.0 1.0");
	RGB_CLR.push_back ("0.0 0.0 1.0");
	TITLE.push_back ("/s3_iter_axis {");
	GRY_CLR.push_back ("1.0 1.0 1.0");
	RGB_CLR.push_back ("0.0 0.0 1.0");

	for (size_t i = 0; i < TITLE.size(); i++) {

		const bool ITER = (i == 1 || i == 3 || i== 5);

		text_PS (o, TITLE.at(i));
		newpath_PS (o);
		moveto_PS (o,  -2.0,  0.0, 3);
		rlineto_PS (o,  4.0,  4.0, 3);
		rlineto_PS (o, -4.0,  4.0, 3);
		rlineto_PS (o, -4.0, -4.0, 3);
		closepath_PS (o);
		if (!ITER) color_PS (o, "1.0 1.0 1.0"); //ok
		linewidth_PS (o, 1.0, 1);
		if (!ITER) stroke_PS (o);
		if (i == 0 || i == 2 || i== 4) {

			moveto_PS (o,  -2.0,  0.0, 3);
			rlineto_PS (o,  4.0,  4.0, 3);
			rlineto_PS (o, -4.0,  4.0, 3);
			rlineto_PS (o, -4.0, -4.0, 3);
		}
		if (is_GRAYSCALE_USE())	color_PS (o, GRY_CLR.at(i));
		else 					color_PS (o, RGB_CLR.at(i));
		if (!ITER) fill_PS (o);
		stroke_PS(o);
		text_PS (o, "} def");
	}

	text_PS (o, "/normalarrow {");
	color_PS(o, "0.0 0.0 0.0");
	newpath_PS (o);
	linewidth_PS (o, 1.0, 1);
	moveto_PS (o, 0.0, -6.0, 3);
	lineto_PS (o, 0.0, 6.0, 3);
	stroke_PS (o);
	newpath_PS (o);
	moveto_PS (o, -2.0,   3.0, 3);
	rlineto_PS (o, 2.0,  10.0, 3);
	rlineto_PS (o, 2.0, -10.0, 3);
	rlineto_PS (o, -2.0,  3.0, 3);
	rlineto_PS (o, -2.0, -3.0, 3);
	linewidth_PS(o, 1.0, 1);
	fill_PS (o);
	stroke_PS (o);

	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 2, 1);
	arc_PS (o, 0.0, 0.0, 1.0, 0.0, 360.0, 3);
	stroke_PS (o);

	color_PS (o, "1.0 1.0 1.0");
	linewidth_PS (o, 1.5, 1);
	arc_PS (o, 0.0, 0.0, 0.5, 0.0, 360.0, 3);
	stroke_PS (o);
	text_PS (o, "} def");

	text_PS (o, "/dextralarrow {");
	newpath_PS (o);
	color_PS(o, "0.0 0.0 0.0");
	moveto_PS (o, 0.0, -1.5, 3);
	lineto_PS (o, -6.0, -1.5, 3);
	linewidth_PS(o, 1.0, 1);
	stroke_PS(o);
	newpath_PS (o);
	moveto_PS (o, -11.0, -1.2, 3);
	lineto_PS (o, -4.0, -3.5, 3);
	lineto_PS (o, -6.0, -1.2, 3);
	lineto_PS (o, -11.0, -1.2, 3);
	fill_PS (o);
	stroke_PS (o);
	newpath_PS (o);
	moveto_PS(o, 0.0, 1.5, 3);
	lineto_PS(o, 6.0, 1.5, 3);
	stroke_PS (o);
	newpath_PS(o);
	moveto_PS(o, 11.0, 1.2, 3);
	lineto_PS(o, 4.0, 3.5, 3);
	lineto_PS(o, 6.0, 1.2, 3);
	lineto_PS(o, 11.0, 1.2, 3);
	fill_PS (o);
	stroke_PS (o);

	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 2, 1);
	arc_PS (o, 0.0, 0.0, 1.0, 0.0, 360.0, 3);
	stroke_PS (o);

	color_PS (o, "1.0 1.0 1.0");
	linewidth_PS (o, 1.5, 1);
	arc_PS (o, 0.0, 0.0, 0.5, 0.0, 360.0, 3);
	stroke_PS (o);
	text_PS (o, "} def");

	text_PS (o, "/sinistralarrow {");
	newpath_PS(o);
	color_PS(o, "0.0 0.0 0.0");
	moveto_PS(o, 0.0, 1.5, 3);
	lineto_PS(o, -6.0, 1.5, 3);
	linewidth_PS(o, 1.0, 1);
	stroke_PS(o);
	newpath_PS(o);
	moveto_PS(o, -11.0, 1.2, 3);
	lineto_PS(o, -4.0, 3.5, 3);
	lineto_PS(o, -6.0, 1.2, 3);
	lineto_PS(o, -11.0, 1.2, 3);
	fill_PS(o);
	stroke_PS(o);
	newpath_PS(o);
	moveto_PS(o, 0.0, -1.5, 3);
	lineto_PS(o, 6.0, -1.5, 3);
	stroke_PS(o);
	newpath_PS(o);
	moveto_PS(o, 11.0, -1.2, 3);
	lineto_PS(o, 4.0, -3.5, 3);
	lineto_PS(o, 6.0, -1.2, 3);
	lineto_PS(o, 11.0, -1.2, 3);
	fill_PS(o);
	stroke_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 2, 1);
	arc_PS (o, 0.0, 0.0, 1.0, 0.0, 360.0, 3);
	stroke_PS (o);

	color_PS (o, "1.0 1.0 1.0");
	linewidth_PS (o, 1.5, 1);
	arc_PS (o, 0.0, 0.0, 0.5, 0.0, 360.0, 3);
	stroke_PS (o);
	text_PS (o, "} def");

	text_PS(o, "/nonearrow {");
	newpath_PS(o);
	moveto_PS(o, 0.0, -8.0, 3);
	lineto_PS(o, 0.0, 8.0, 3);
	linewidth_PS(o, 1.0, 1);
	stroke_PS(o);
	linewidth_PS(o, 1.0, 1);
	arc_PS(o, 0.0, 0.0, 1.5, 0.0, 360.0, 3);
	gsave_PS(o);
	color_PS(o, "1.00 1.00 1.00");
	fill_PS(o);
	grestore_PS(o);
	stroke_PS(o);
	text_PS (o, "} def");

	TITLE.clear();
	RGB_CLR.clear();
	GRY_CLR.clear();

	//-----KEEP HERE---------------
	/*
	TITLE.push_back("/p_axis {");
	RGB_CLR.push_back("1.0 0.0 0.0");
	GRY_CLR.push_back("0.0 0.0 0.0");

	TITLE.push_back("/t_axis {");
	RGB_CLR.push_back("0.0 0.0 1.0");
	GRY_CLR.push_back("0.33 0.33 0.33");

	TITLE.push_back("/n_axis {");
	RGB_CLR.push_back("0.0 1.0 0.0");
	GRY_CLR.push_back("0.66 0.66 0.66");


	for (size_t i = 0; i < TITLE.size(); i++) {

		text_PS (o, TITLE.at(i));
		newpath_PS (o);
		moveto_PS (o, -1.0,  -1.0, 3);
		rlineto_PS (o, 0.0, 2.0, 3);
		rlineto_PS (o, 2.0, 0.0, 3);
		rlineto_PS (o, 0.0, -2.0, 3);
		closepath_PS (o);
		linewidth_PS (o, 0.5, 1);
		if (is_GRAYSCALE_USE()) color_PS (o, GRY_CLR.at(i));
		else color_PS (o, RGB_CLR.at(i));
		fill_PS (o);
		stroke_PS (o);
		text_PS (o, "} def");

	}
	 */
	return;
}

void PS_border (const vector <GDB>& inGDB, ofstream& o, const PAPER& P) {

	const string DT = inGDB.at(0).DATATYPE;
	const string LOC = inGDB.at(0).LOC;
	const string GC = inGDB.at(0).GC;
	const string FM = inGDB.at(0).FORMATION;

	const bool STRIAE = is_allowed_striae_datatype (DT);

	const bool asked_KMEANS = !is_CLUSTERING_NONE();
	const bool asked_RUPANG = !is_RUP_CLUSTERING_NONE();

	const bool by_GROUPCODE = is_GROUPSEPARATION_GROUPCODE ();
	const bool by_KMEANS = is_GROUPSEPARATION_KMEANS ();
	const bool by_RUPANG = is_GROUPSEPARATION_RUPANG ();
	const bool IGNORE = is_GROUPSEPARATION_IGNORE ();

	const bool color_by_COLORCODE = is_COLOURING_COLORCODE();
	const bool color_by_GROUPCODE = is_COLOURING_GROUPCODE();
	const bool color_by_KMEANS = is_COLOURING_KMEANS ();
	const bool color_by_RUPANG = is_COLOURING_RUPANG ();
	const bool color_IGNORE = is_COLOURING_IGNORE ();

	const bool exists_GROUPCODE = inGDB.at(0).GC.at(0) != 'X';
	const bool exists_KMEANS = inGDB.at(0).GC.at(1) != 'X';
	const bool exists_RUPANG = inGDB.at(0).GC.at(2) != 'X';

	if (!by_GROUPCODE && !by_KMEANS && !by_RUPANG && !IGNORE) ASSERT_DEAD_END();
	if (!color_by_COLORCODE && !color_by_GROUPCODE && !color_by_KMEANS && !color_by_RUPANG && !color_IGNORE) ASSERT_DEAD_END();

	if (by_GROUPCODE && GC.size() < 1) ASSERT_DEAD_END();
	if (by_KMEANS && GC.size() < 2) ASSERT_DEAD_END();
	if (by_RUPANG && GC.size() < 3) ASSERT_DEAD_END();

	newpath_PS(o);

	linewidth_PS ( o, 1.5, 1);

	moveto_PS (o, P.A, P.Y - P.A - 10.0, 3);
	lineto_PS (o, P.A, P.A, 3);
	lineto_PS (o, P.X - P.A, P.A, 3);
	lineto_PS (o, P.X - P.A, P.Y - P.A, 3);

	color_PS (o, "0.0 0.0 0.0");
	stroke_PS (o);

	color_PS (o, "0.8 0.8 0.8");
	stroke_PS (o);

	font_PS (o, "ArialNarrow-Bold", 36);
	color_PS (o, "0.8 0.8 0.8");
	text_PS (o, P.O5X + 5.0 * P.A, P.O5Y + P.B, 3, "ORIGINAL");
	text_PS (o, P.O6X + 4.0 * P.A, P.O6Y + P.B, 3, "CORRECTED");

	font_PS(o, "ArialNarrow-Bold", 12);
	color_PS (o, "0.0 0.0 0.0");

	string T = DT + " FROM LOCATION " + LOC;

	if (asked_KMEANS) T = T + " - K-MEANS CLUSTERING USED";

	if (asked_RUPANG && STRIAE) T = T + " - RUP/ANG CLUSTERING USED";

	if (by_GROUPCODE) 	T = T + ", GROUP '" + GC.at(0) + "' USING ORIGINAL GROUPCODE" ;
	else if (by_KMEANS) T = T + ", GROUP '" + GC.at(1) + "' USING CLUSTERING RESULT" ;
	else if (by_RUPANG) {

		if (STRIAE) {

			const vector <GDB> hasoffset_GDB = return_striae_with_offset (inGDB);

			const bool ENOUGH_STRIAE = hasoffset_GDB.size() >= minimum_independent_dataset ();

			if (is_RUP_CLUSTERING_ANG()) {

				if (ENOUGH_STRIAE) T = T + ", GROUP '" + GC.at(2) + "' USING ANG CLUSTERING RESULT" ;
			}
			else if (is_RUP_CLUSTERING_ANG()) {

				if (ENOUGH_STRIAE) T = T + ", GROUP '" + GC.at(2) + "' USING RUP CLUSTERING RESULT" ;
			}
			else ASSERT_DEAD_END();
		}
	}
	else if (is_GROUPSEPARATION_IGNORE()) {}
	else ASSERT_DEAD_END();

	if (color_by_COLORCODE) {

		T = T + ", COLOURED USING COLOUR CODE";
	}
	else if (color_by_GROUPCODE) {

		if (exists_GROUPCODE) T = T + ", COLOURED USING GROUP CODE";
	}
	else if (color_by_KMEANS) {

		if (exists_KMEANS) T = T + ", COLOURED USING K-MEANS CLUSTER CODE";
	}
	else if (color_by_RUPANG) {

		if (is_RUP_CLUSTERING_ANG() && exists_RUPANG) {

			T = T + ", COLOURED USING 'ANG' CLUSTER CODE";
		}
		else if (is_RUP_CLUSTERING_RUP() && exists_RUPANG) {

			T = T + ", COLOURED USING 'RUP' CLUSTER CODE";
		}
	}
	else if (color_IGNORE) {}
	else ASSERT_DEAD_END();

	text_PS (o, P.A - P.B + P.D * 20.0, P.Y - P.A - P.D * 5.5, 3, T);

	text_PS(o, "%%-----end PSborder");
}

void PS_stress_scale (ofstream& o, const PAPER& P) {

	double value = 0.0;

	for (size_t step = 1; step < 300; step++) {

		value = step * 0.01;

		const string C = generate_stress_colors (value);

		color_PS (o, C);
		linewidth_PS (o, 2.0, 1);

		newpath_PS (o);
		moveto_PS (o, P.O1X + P.R + 1.0 * P.B, P.O1Y - P.R + 0.666 * P.R * value, 3);
		lineto_PS (o, P.O1X + P.R + 1.5 * P.B, P.O1Y - P.R + 0.666 * P.R * value, 3);
		stroke_PS (o);

		newpath_PS (o);
		moveto_PS (o, P.O2X + P.R + 1.0 * P.B, P.O2Y - P.R + 0.666 * P.R * value, 3);
		lineto_PS (o, P.O2X + P.R + 1.5 * P.B, P.O2Y - P.R + 0.666 * P.R * value, 3);
		stroke_PS (o);
	}

	font_PS (o, "ArialNarrow-Bold", 6);
	color_PS(o, "0.0 0.0 0.0");
	moveto_PS (o, P.O1X + 1.0 * P.R + 0.9 * P.B, P.O1Y - 1.0 * P.R, 3);
	rotate_PS(o, 90.0, 1);
	text_PS(o, " (EXTENSION) show");
	rotate_PS(o, 270.0, 1);

	moveto_PS (o, P.O1X + 1.0 * P.R + 0.9 * P.B, P.O1Y + 1.0 * P.R - 0.92 * P.A , 3);
	rotate_PS(o, 90.0, 1);
	text_PS(o, " (COMPRESSION) show");
	rotate_PS(o, 270.0, 1);

	moveto_PS (o, P.O2X + 1.0 * P.R + 0.9 * P.B, P.O2Y - 1.0 * P.R, 3);
	rotate_PS(o, 90.0, 1);
	text_PS(o, " (EXTENSION) show");
	rotate_PS(o, 270.0, 1);

	moveto_PS (o, P.O2X + 1.0 * P.R + 0.9 * P.B, P.O2Y + 1.0 * P.R - 0.92 * P.A , 3);
	rotate_PS(o, 90.0, 1);
	text_PS(o, " (COMPRESSION) show");
	rotate_PS(o, 270.0, 1);

	return;
}

void PS_net (ofstream& o, const PAPER& P) {

	string 						T = "Schmidt-net,";
	if (is_NET_WULFF())			T = "Wulff-net,";
	if (is_HEMISPHERE_UPPER()) 	T = T + " upper hemisphere";
	else 						T = T + " lower hemisphere";
	if (is_PLOT_HOEPPENER()) 	T = "Hoeppener-plot, " + T;
	else 						T = "Angelier-plot, " + T;

	string C = "";
	if (is_CONTOURING_DIPDIR_BEARING()) 			C = "Contouring plane dip directions / lineation bearings";
	else if (is_CONTOURING_STRIKEDIR_BEARING())		C = "Contouring plane strike directions / lineation bearings";
	else if (is_CONTOURING_PLANE_NORMAL_BEARING())	C = "Contouring plane normal directions / lineation bearings";
	else if (is_CONTOURING_STRIAE_BEARING_BEARING())C = "Contouring striae / lineations bearings";
	else {};

	vector <VCTR> V;

	V.push_back (declare_vector (P.O1X, P.O1Y, NaN()));
	V.push_back (declare_vector (P.O2X, P.O2Y, NaN()));
	V.push_back (declare_vector (P.O3X, P.O3Y, NaN()));
	V.push_back (declare_vector (P.O4X, P.O4Y, NaN()));
	V.push_back (declare_vector (P.O5X, P.O5Y, NaN()));
	V.push_back (declare_vector (P.O6X, P.O6Y, NaN()));

	color_PS (o, "0.0 0.0 0.0");

	for (size_t i = 0; i < 6; i++) {

		double ANG = 0.0;
		if (i >= 4) {

			ANG = 270.0;

			newpath_PS (o);
			moveto_PS (o, V.at(i).X, V.at(i).Y, 3);
			lineto_PS (o, V.at(i).X + P.R + P.C, V.at(i).Y, 3);
			moveto_PS (o, V.at(i).X, V.at(i).Y, 3);
			lineto_PS (o, V.at(i).X, V.at(i).Y - P.R - P.C, 3);
			stroke_PS (o);

			translate_PS(o, V.at(i).X, V.at(i).Y, 3);
			rotate_PS(o, 180.0, 1);
			for (size_t j = 27; j < 36; j++) {

				newpath_PS (o);
				moveto_PS (o, 0.0, P.R - 6.0, 3);
				lineto_PS (o, 0.0, P.R, 3);
				stroke_PS (o);
				rotate_PS (o, 10.0, 1);
			}
			rotate_PS(o, -270.0, 1);
			translate_PS(o, - V.at(i).X, - V.at(i).Y, 3);
		}
		else {

			linewidth_PS (o, 1.20, 1);
			newpath_PS (o);
			moveto_PS (o, V.at(i).X, V.at(i).Y + P.R, 3);
			lineto_PS (o, V.at(i).X, V.at(i).Y + P.R + P.C, 3);
			moveto_PS (o, V.at(i).X, V.at(i).Y - P.R, 3);
			lineto_PS (o, V.at(i).X, V.at(i).Y - P.R - P.C, 3);
			moveto_PS (o, V.at(i).X + P.R, V.at(i).Y, 3);
			lineto_PS (o, V.at(i).X + P.R + P.C, V.at(i).Y, 3);
			moveto_PS (o, V.at(i).X - P.R, V.at(i).Y, 3);
			lineto_PS (o, V.at(i).X - P.R - P.C, V.at(i).Y, 3);
			stroke_PS (o);

			newpath_PS (o);
			moveto_PS (o, V.at(i).X, V.at(i).Y + P.C, 3);
			lineto_PS (o, V.at(i).X, V.at(i).Y - P.C, 3);
			moveto_PS (o, V.at(i).X + P.C, V.at(i).Y, 3);
			lineto_PS (o, V.at(i).X - P.C, V.at(i).Y, 3);
			stroke_PS (o);

			if (i >= 2 && i <= 3) {
				translate_PS(o, V.at(i).X, V.at(i).Y, 3);
				for (size_t j = 0; j < 36; j++) {

					newpath_PS (o);
					moveto_PS (o, 0.0, P.R - 6.0, 3);
					lineto_PS (o, 0.0, P.R, 3);
					stroke_PS (o);
					rotate_PS (o, 10.0, 1);
				}
				translate_PS(o, - V.at(i).X, - V.at(i).Y, 3);
			}
			font_PS (o, "ArialNarrow-Bold", 12);
			text_PS (o, V.at(i).X - 3.5 * P.D, V.at(i).Y + P.R + 14.0 * P.D, 3, "N");

			if (i <= 1) {
				font_PS (o, "ArialNarrow", 8);
				text_PS (o, V.at(i).X - P.R - 0.2 * P.B, V.at(i).Y - P.R - 20.0 * P.D, 3, T);
				text_PS (o, V.at(i).X - P.R - 0.2 * P.B, V.at(i).Y - P.R - 28.0 * P.D, 3, C);
			}
		}
		linewidth_PS (o, 1.0, 1);
		newpath_PS (o);
		arc_PS (o, V.at(i).X, V.at(i).Y, P.R, ANG, 360.0, 1);
		stroke_PS (o);
	}
	font_PS(o, "ArialNarrow", 8);
	text_PS (o, 20.0 * P.A, P.A + 5.0 * P.D, 3, "Plotted by SG2PS - for reference see www.sg2ps.eu webpage.");

	return;
}

void PS_stressdata (const vector <GDB>& inGDB, ofstream& o, const CENTER& center, const PAPER& P, const STRESSFIELD& sf) {

	const bool FRACTURE = inGDB.at(0).DATATYPE == "FRACTURE" && is_BINGHAM_USE();

	string METHOD = "";
	string RESULT = "";

	font_PS (o, "ArialNarrow", 8);
	color_PS (o, "0.0 0.0 0.0");
	translate_PS (o, center.X + (center.radius / 2.0) - P.A, center.Y - center.radius - 20.0 * P.D, 3);

	if 		(FRACTURE) 					METHOD = "Fracture statistics after Bingham (1964)";
	else if (is_INVERSION_SPRANG())  	METHOD = "Regression after Sprang (1972)";
	else if (is_INVERSION_FRY()) 		METHOD = "Regression after Fry (1999)";
	else if (is_INVERSION_SHAN()) 		METHOD = "Regression after Shan et al. (2003)";
	else if (is_INVERSION_ANGELIER()) 	METHOD = "Inversion after Angelier (1990)";
	else if (is_INVERSION_TURNER()) 	METHOD = "Regression after Turner (1953)";
	else if (is_INVERSION_BRUTEFORCE())	METHOD = "Brute force inversion";
	else if (is_INVERSION_MICHAEL()) 	METHOD = "Inversion after Michael (1984)";
	else if (is_INVERSION_MOSTAFA()) 	METHOD = "Inversion after Mostafa (2005)";
	else if (is_INVERSION_YAMAJI()) 	METHOD = "Inversion after Yamaji (2000)";
	else ASSERT_DEAD_END();

	text_PS (o, 0.0, 0.0, 3, METHOD);

	vector <string> stress_DIPDIR;

	stress_DIPDIR.push_back (double_to_string (sf.S_1.DIPDIR, 0));
	stress_DIPDIR.push_back (double_to_string (sf.S_2.DIPDIR, 0));
	stress_DIPDIR.push_back (double_to_string (sf.S_3.DIPDIR, 0));

	for (size_t i = 0; i < stress_DIPDIR.size(); i++) {

		if 		(stress_DIPDIR.at(i).size() == 3) {}
		else if (stress_DIPDIR.at(i).size() == 2) stress_DIPDIR.at(i) = "0" + stress_DIPDIR.at(i);
		else if (stress_DIPDIR.at(i).size() == 1) stress_DIPDIR.at(i) = "00" + stress_DIPDIR.at(i);
		else ASSERT_DEAD_END();
	}

	vector <string> stress_DIP;

	stress_DIP.push_back (double_to_string (sf.S_1.DIP, 0));
	stress_DIP.push_back (double_to_string (sf.S_2.DIP, 0));
	stress_DIP.push_back (double_to_string (sf.S_3.DIP, 0));

	for (size_t i = 0; i < stress_DIP.size(); i++) {

		if 		(stress_DIP.at(i).size() == 2) {}
		else if (stress_DIP.at(i).size() == 1) stress_DIP.at(i) = "0" + stress_DIP.at(i);
		else ASSERT_DEAD_END();
	}

	string stress_NAME = "S";
	if (FRACTURE) stress_NAME = "e";

	for (size_t i = 0; i < stress_DIPDIR.size(); i++) {

		RESULT = RESULT + stress_NAME + size_t_to_string(i+1) + ": " + stress_DIPDIR.at(i) + "/" + stress_DIP.at(i);
		if (i < 2) RESULT = RESULT + ", ";
	}

	text_PS (o, 0.0, -8.0, 3, RESULT);

	RESULT = "";

	if (FRACTURE)  {

		RESULT =
				"E1: " + double_to_string (sf.EIGENVALUE.X * 100.0, 1) + "%, " +
				"E2: " + double_to_string (sf.EIGENVALUE.Y * 100.0, 1) + "%, " +
				"E3: " + double_to_string (sf.EIGENVALUE.Z * 100.0, 1) + "% ";
	}
	else {

		RESULT =
				"R: " + double_to_string (sf.stressratio, 3) + ", " +
				"R': " + double_to_string (sf.delvaux_str, 3);

	}

	text_PS (o, 0.0, -16.0, 3, RESULT);

	translate_PS(o, - center.X - (center.radius / 2.0) + P.A, - center.Y + center.radius + 20.0 * P.D, 3);
}

void PS_stressarrows (ofstream& o, const CENTER& center, const PAPER& P, const STRESSFIELD& sf) {

	if ((sf.regime == "COMPRESSIONAL") || (sf.regime == "STRIKE-SLIP")) {

		translate_PS (o, center.X, center.Y, 3);

		rotate_PS (o, -sf.shmax, 3);
		translate_PS (o, 0.0, center.radius + (0.2 * P.B), 3);
		text_PS (o, "newpath compression_arrow");
		translate_PS (o, 0.0, -center.radius - (0.2 * P.B), 3);
		rotate_PS(o, sf.shmax, 3);
		rotate_PS(o, 180.0, 1);

		rotate_PS(o, -sf.shmax, 3);
		translate_PS (o, 0.0, center.radius + (0.2 * P.B), 3);
		text_PS (o, "newpath compression_arrow");
		translate_PS (o, 0.0, -center.radius - (0.2 * P.B), 3);
		rotate_PS(o, sf.shmax, 3);
		rotate_PS(o, 180.0, 1);

		translate_PS(o, -center.X, -center.Y, 3);
	}

	if ((sf.regime == "EXTENSIONAL") || (sf.regime == "STRIKE-SLIP")) {

		translate_PS(o, center.X, center.Y, 3);

		rotate_PS(o, - sf.shmin, 3);
		translate_PS (o, 0.0, center.radius + (0.2 * P.B), 3);
		text_PS (o, "newpath extension_arrow");
		translate_PS (o, 0.0, -center.radius - (0.2 * P.B), 3);
		rotate_PS(o, sf.shmin, 3);
		rotate_PS(o, 180.0, 1);

		rotate_PS(o, - sf.shmin, 3);
		translate_PS (o, 0.0, center.radius + (0.2 * P.B), 3);
		text_PS (o, "newpath extension_arrow");
		translate_PS (o, 0.0, -center.radius - (0.2 * P.B), 3);
		rotate_PS(o, sf.shmin, 3);
		rotate_PS(o, 180.0, 1);

		translate_PS(o, -center.X, -center.Y, 3);
	}
	return;
}

void PS_mohr_circle (const vector <GDB>& inGDB, ofstream& o, const CENTER& mohrcenter, const PAPER& P) {

	if (!is_allowed_striae_datatype (inGDB.at(0).DATATYPE)) ASSERT_DEAD_END();

	if (inGDB.at(0).STV.size() < 1 || inGDB.at(0).SFV.size() < 1) return;

	STRESSTENSOR ST = inGDB.at(0).STV.at (inGDB.at(0).STV.size() - 1);
	const STRESSFIELD  SF = inGDB.at(0).SFV.at (inGDB.at(0).SFV.size() - 1);

	if (is_INVERSION_TURNER()) ST = invert_stress_tensor (ST);

	const double X = 5.0 * P.A;
	const double fi = SF.stressratio;

	const double S1 = SF.EIGENVALUE.X;
	const double S3 = SF.EIGENVALUE.Z;

	string CLR = generate_stress_colors (SF.delvaux_str);

	color_PS(o, CLR);
	linewidth_PS (o, 3.0, 1);

	newpath_PS (o);
	arc_PS (o, mohrcenter.X + 2.5 * P.A, mohrcenter.Y, 2.5 * P.A, 0.0, 180.0, 3);
	stroke_PS (o);

	newpath_PS (o);
	arc_PS (o, mohrcenter.X + (0.5 * fi * X), mohrcenter.Y, 0.5 * fi * X, 0.0, 180.0, 3);
	stroke_PS (o);

	newpath_PS (o);
	arc_PS (o, mohrcenter.X + 2.5 * P.A + (0.5 * fi * X), mohrcenter.Y, 2.5 * P.A - (0.5 * fi * X), 0.0, 180.0, 3);
	stroke_PS (o);

	color_PS (o, "0.0 0.0 0.0");
	font_PS (o, "ArialNarrow-Bold", 8);
	text_PS (o, mohrcenter.X - 4.0 * P.D, mohrcenter.Y - 10.0 * P.D, 3, "S3");
	text_PS (o, mohrcenter.X + X - 4.0 * P.D, mohrcenter.Y - 10.0 * P.D, 3, "S1");
	text_PS (o, mohrcenter.X + (fi * X) - 4.0 * P.D, mohrcenter.Y - 10.0 * P.D, 3, "S2");

	for (size_t i = 0; i < inGDB.size(); i++) {

		if (inGDB.at(i).RUP > 0.0 && inGDB.at(i).ANG > 0.0) {

			const VCTR stressvector = return_stressvector (ST, inGDB.at(i).N);
			const double stress_magnitude =
					inGDB.at(i).N.X * stressvector.X +
					inGDB.at(i).N.Y * stressvector.Y +
					inGDB.at(i).N.Z * stressvector.Z;

			double normal_s = sqrt(
					inGDB.at(i).NORMAL_S.X * inGDB.at(i).NORMAL_S.X +
					inGDB.at(i).NORMAL_S.Y * inGDB.at(i).NORMAL_S.Y +
					inGDB.at(i).NORMAL_S.Z * inGDB.at(i).NORMAL_S.Z);

			if (!is_INVERSION_TURNER() && stress_magnitude > 0.0) normal_s = - normal_s;

			normal_s = (normal_s - S3) / (S1 - S3);

			double shear_s = sqrt (
					inGDB.at(i).SHEAR_S.X * inGDB.at(i).SHEAR_S.X +
					inGDB.at(i).SHEAR_S.Y * inGDB.at(i).SHEAR_S.Y +
					inGDB.at(i).SHEAR_S.Z * inGDB.at(i).SHEAR_S.Z);

			shear_s = shear_s / (S1 - S3);

			newpath_PS(o);
			color_PS(o, "1.0 1.0 1.0");
			linewidth_PS (o, 3.0, 1);
			arc_PS (o, mohrcenter.X + X * normal_s, mohrcenter.Y + X * shear_s, 0.7, 0.0, 360.0, 3);
			stroke_PS(o);

			newpath_PS(o);
			color_PS(o, "0.0 0.0 0.0");
			linewidth_PS (o, 2.0, 1);
			arc_PS(o, mohrcenter.X + X * normal_s, mohrcenter.Y + X * shear_s, 0.7, 0.0, 360.0, 3);
			stroke_PS(o);
		}
		else {}
	}
	return;
}

void PS_RUP_ANG_distribution (const vector <GDB>& inGDB, ofstream& o, const CENTER& center, const PAPER& P, const string method) {

	const bool RUP = method == "RUP";
	const bool ANG = method == "ANG";
	if (!RUP && !ANG) ASSERT_DEAD_END();

	if (RUP && !is_INVERSION_ANGELIER() && !is_INVERSION_MOSTAFA() && !is_INVERSION_SHAN() && !is_INVERSION_FRY()) return;

	vector <VALLEY> V;

	if 		(ANG) V = return_valleygraph_for_dataset (inGDB, "ANG");
	else if (RUP) V = return_valleygraph_for_dataset (inGDB, "RUP");
	else ASSERT_DEAD_END();

	if (V.size() == 1 && V.at(0).BIN_ID == 999 && V.at(0).DIR == "X") return;

	double DATA_min = return_datamin (inGDB, method);
	double DATA_max = return_datamax (inGDB, method);

	const size_t bin_number = return_DATA_ideal_bin_number (GDB_to_table (inGDB, method));

	const double binsize = (DATA_max - DATA_min) / bin_number;

	vector <HISTOGRAM> H = generate_DATA_histogram (GDB_to_table (inGDB, method), bin_number);

	H = sort_by_COUNT (H);

	vector <line_RUP_ANG> L_R_A = generate_graph_histogram (H, V, DATA_max);

	ps_RUP_ANG_header (o, center, P);

	ps_draw_histogram_bars (inGDB, L_R_A, o, center, P, DATA_min, DATA_max, bin_number, binsize, method);

	ps_percentage (o, center, P, method, DATA_max);

	ps_percentage_max (o, center, P, method, DATA_max);
}

void PS_stress_state (ofstream& o, const PAPER P, const CENTER& center, const STRESSFIELD& sf) {

	const double value = sf.delvaux_str;

	color_PS(o, "1.0 1.0 1.0");
	linewidth_PS (o, 5.0, 1);
	newpath_PS (o);
	moveto_PS (o, center.X + P.R + 0.9 * P.B, center.Y - P.R + (0.666 * P.R * value), 3);
	lineto_PS (o, center.X + P.R + 1.6 * P.B, center.Y - P.R + (0.666 * P.R * value), 3);
	stroke_PS(o);
}

void PS_folddata (GDB in, ofstream& o, CENTER center) {

	//cout << "PS_folddata" << endl;

	font_PS(o, "ArialNarrow-Bold", 8);

	color_PS (o, "0 0 0");

	translate_PS (o, center.X + (center.radius / 2.0), center.Y - center.radius, 3);

	string T = "Fold great circle: ";

	//cout << in.fold_great_circle_N.X << endl;

	DIPDIR_DIP DD = dipdir_dip_from_NXNYNZ (in.fold_great_circle_N);

	//cout << DD.DIPDIR << endl;

	if (DD.DIPDIR < 10.0) T = T + "00" + double_to_string (DD.DIPDIR, 0);
	else if (DD.DIPDIR < 100.0) T = T + "0" + double_to_string (DD.DIPDIR, 0);
	else T = T + double_to_string (DD.DIPDIR, 0);

	T = T + "/";

	if (DD.DIP < 10.0) T = T + "0" + double_to_string (DD.DIP, 0);
	else T = T + double_to_string (DD.DIP, 0);

	text_PS(o, 0.0, 0.0, 3, T);

	translate_PS (o, - center.X - (center.radius / 2.0), - center.Y + center.radius, 3);

	//cout << "end PS_folddata" << endl;

	return;
}

void PS_lineation (const GDB& i, ofstream& o, const CENTER& center, const STRESSFIELD& sf, const bool label, const string type) {

	VCTR L;
	string T = "";

	bool OTHER = false;

	if (type == "S1") {

		L = sf.EIGENVECTOR1;
		T = " newpath s1_axis";
	}
	else if (type == "S1_ITER") {

		L = sf.EIGENVECTOR1;
		T = "  newpath s1_iter_axis";
	}
	else if (type == "S2") {

		L = sf.EIGENVECTOR2;
		T = " newpath s2_axis";
	}
	else if (type == "S2_ITER") {

		L = sf.EIGENVECTOR2;
		T = "  newpath s2_iter_axis";
	}
	else if (type == "S3") {

		L = sf.EIGENVECTOR3;
		T = " newpath s3_axis";
	}
	else if (type == "S3_ITER") {

		L = sf.EIGENVECTOR3;
		T = "  newpath s3_iter_axis";
	}
	else {

		L = i.D;
		OTHER = true;
	}


	if (is_D_up (L)) {

		cout << fixed << setprecision(6) << endl;
		cout << L.X << "  -  " << L.Y << "  -  " << L.Z << endl;

		ASSERT_DEAD_END();
		L = flip_vector(L);
	}
	//if (L.Z > 0.0) L.Z = - L.Z;

	double X = 0.0;
	double Y = 0.0;

	if (is_NET_SCHMIDT()) {

		X = (L.X / sqrt (1.00 - L.Z)) * center.radius;
		Y = (L.Y / sqrt (1.00 - L.Z)) * center.radius;
	}
	else {

		X = (L.X / (1.00 - L.Z)) * center.radius;
		Y = (L.Y / (1.00 - L.Z)) * center.radius;
	}

	if (is_HEMISPHERE_UPPER()) {

		X = - X;
		Y = - Y;
	}

	X = X + center.X;
	Y = Y + center.Y;


	if (is_CHK_PLOT_LINEATION ()) {

		if (i.ID == "BZ0090") {

			cout << fixed << setprecision(8) << endl;

			cout << "**** PS_LINEATION test - if fails listed below." << endl;

			if (! is_in_range (211.13382129, 211.13382129, X)) cout << "X test failed: " << 211.13382129 << " <> " << X << endl;

			cout << "**** End of PS_LINEATION test." << endl;

			exit (100);
		}
	}

	if (OTHER) {

		newpath_PS (o);
		color_PS (o, i.PSCOLOR);
		linewidth_PS (o, 1, 1);
		arc_PS (o, X, Y, 1.2, 0.0, 360, 3);
		stroke_PS (o);
	}
	else {

		translate_PS (o, X, Y, 3);
		text_PS (o, T);
		translate_PS (o, -X, -Y, 3);
	}

	if (label) {

		font_PS (o, "ArialNarrow-Italic", 6);
		linewidth_PS (o, 0.5, 1);
		color_PS (o, "0.5 0.5 0.5");
		translate_PS (o, X, Y, 3);
		moveto_PS (o, 0.0, 0.0, 3);
		lineto_PS (o, 10.0, 10.0, 3);
		stroke_PS (o);
		//moveto_PS (o, 10.0, 10.0, 3);
		//text_PS (o, i.ID);
		text_PS(o, 10.0, 10.0, 3, i.ID);
		text_PS (o, "%%--------1");
		translate_PS (o, -X, -Y, 3);
	}
}

void PS_plane (const GDB& i, ofstream& o, const double X, const double Y, const double R, const string TYPE) {

	const bool OT = is_allowed_bedding_overturned_sense (i.avS0offset);
	const bool OTB = is_allowed_bedding_overturned_sense (i.OFFSET);

	const bool AV = TYPE == "AV";
	const bool C = TYPE == "C";
	const bool FOLD = TYPE == "FOLD";

	const bool AVO = OT && AV;

	size_t 					steps = 1;
	if (is_NET_SCHMIDT()) 	steps = 60;

	VCTR N;
	//VCTR D;

	DIPDIR_DIP DD;

	if (C) {

		DD = i.corrL;
		VCTR D = i.DC;
		if (is_DC_up(D)) D = flip_vector(D);
		N = NXNYNZ_from_DXDYDZ (D);
	}
	else if (AV || AVO) {

		DD = i.avS0d;
		VCTR D =  i.avS0D;
		if (is_D_up(D)) D = flip_vector(D);
		N = NXNYNZ_from_DXDYDZ (D);
	}
	else if (FOLD) {

		DD = dipdir_dip_from_NXNYNZ (i.fold_great_circle_N);
		//D = DXDYDZ_from_NXNYNZ (i.fold_great_circle_N);
		N = i.fold_great_circle_N;
	}
	else {

		DD = i.corr;
		//D = i.D;
		N = i.N;
	}

	if (is_N_down(N)) N = flip_vector(N);

	double DIPDIR = DD.DIPDIR;

	if (is_HEMISPHERE_UPPER()) DIPDIR = DIPDIR + 180.0;

	vector <VCTR> PP;

	const VCTR torotate = declare_vector(
			SIN (DIPDIR - 90.0),
			COS (DIPDIR - 90.0),
			0.0);

	const VCTR axis = N;

	for (size_t j = 0; j < (steps * 2 + 1); j++) {

		PP.push_back (ROTATE (axis, torotate, - (180.0 / (steps * 2)) * j));

		if (is_HEMISPHERE_UPPER()) {

			PP.at(j).X = - PP.at(j).X;
			PP.at(j).Y = - PP.at(j).Y;
		}

		if (is_NET_WULFF()) {

			PP.at(j).X = (PP.at(j).X / (1.00 - PP.at(j).Z)) * R + X;
			PP.at(j).Y = (PP.at(j).Y / (1.00 - PP.at(j).Z)) * R + Y;
		}
		else {

			PP.at(j).X = (PP.at(j).X / sqrt(1.00 - PP.at(j).Z)) * R + X;
			PP.at(j).Y = (PP.at(j).Y / sqrt(1.00 - PP.at(j).Z)) * R + Y;
		}
	}

	const double X_A = PP.at (steps).X;
	const double Y_A = PP.at (steps).Y;
	const double X_B = PP.at (steps * 2).X;
	const double Y_B = PP.at (steps * 2).Y;
	const double X_C = PP.at (0).X;
	const double Y_C = PP.at (0).Y;

	const double b = sqrt ((X_A - X_C) * (X_A - X_C) + (Y_A - Y_C) * (Y_A - Y_C));
	const double c = sqrt ((X_C - X_B) * (X_C - X_B) + (Y_C - Y_B) * (Y_C - Y_B)) / 2.0;

	const double alfa = ACOS (c / b) * 2.0;

	const double r =  c / SIN (alfa);

	double X_O = X_A - r * SIN (DIPDIR);
	double Y_O = Y_A - r * COS (DIPDIR);

	if (is_in_range (90.0, 180.0, DIPDIR)) {

		X_O = X_A - r * SIN (180.0 - DIPDIR);
		Y_O = Y_A + r * COS (DIPDIR - 180.0);
	}
	else if (is_in_range (180.0, 270.0, DIPDIR)) {

		X_O = X_A + r * SIN (DIPDIR - 180.0);
		Y_O = Y_A + r * COS (DIPDIR - 180.0);
	}
	else if (is_in_range (270.0, 360.0, DIPDIR)) {

		X_O = X_A + r * SIN (360.0 - DIPDIR);
		Y_O = Y_A - r * COS (DIPDIR - 360.0);
	}
	else {}

	if (is_CHK_PLOT_PLANE()) {

		if (i.ID == "BZ66") {

			cout << fixed << setprecision(8) << endl;

			cout << "**** PS_plane test - if fails listed below." << endl;

			if (! is_in_range (208.39022910, 208.39022910, X_A)) cout << "X_A test failed: " << 208.39022910 << " <> " << X_A << endl;
			if (! is_in_range (590.01278679, 590.01278679, Y_A)) cout << "Y_A test failed: " << 590.01278679 << " <> " << Y_A << endl;

			if (! is_in_range (85.50555128, 85.50555128, X_B)) cout << "X_B test failed: " << 85.50555128 << " <> " << X_B << endl;
			if (! is_in_range (520.86695782, 520.86695782, Y_B)) cout << "Y_B test failed: " << 520.86695782 << " <> " << Y_B << endl;

			if (! is_in_range (315.39508944, 315.39508944, X_C)) cout << "X_C test failed: " << 315.39508944 << " <> " << X_C << endl;
			if (! is_in_range (681.83734542, 681.83734542, Y_C)) cout << "Y_C test failed: " << 681.83734542 << " <> " << Y_C << endl;

			if (! is_in_range (141.00280035, 141.002800358, b)) cout << "X_B test failed: " << 141.00280035 << " <> " << b << endl;
			if (! is_in_range (140.32165321, 140.32165321, c)) cout << "Y_B test failed: " << 140.32165321 << " <> " << c << endl;
			if (! is_in_range (11.26806151, 11.26806151, alfa)) cout << "X_C test failed: " << 11.26806151 << " <> " << alfa << endl;
			if (! is_in_range (718.127026252, 718.12702625, r)) cout << "Y_C test failed: " << 718.12702625 << " <> " << r << endl;

			cout << "**** End of PS_plane test." << endl;

			exit (77);
		}
	}

	string CLR;
	string DSH;
	double LWD;

	if (AVO) {

		if (is_GRAYSCALE_USE()) {

			CLR = AVO_GRY_CLR;
			LWD = AVO_GRY_LNW;
			DSH = AVO_GRY_DSH;
		}
		else {

			CLR = AVO_RGB_CLR;
			LWD = AVO_RGB_LNW;
			DSH = AVO_RGB_DSH;
		}
	}
	else if (AV) {

		if (is_GRAYSCALE_USE()) {

			CLR = AV_GRY_CLR;
			LWD = AV_GRY_LNW;
			DSH = AV_GRY_DSH;
		}
		else{

			CLR = AV_RGB_CLR;
			LWD = AV_RGB_LNW;
			DSH = AV_RGB_DSH;
		}
	}
	else if (OTB) {

		if (is_GRAYSCALE_USE()) {

			CLR = i.PSCOLOR;
			LWD = is_LINEWIDTH();
			DSH = OTB_GRY_DSH;
		}
		else{

			CLR = i.PSCOLOR;
			LWD = is_LINEWIDTH();
			DSH = OTB_GRY_DSH;
		}
	}
	else if (FOLD) {
		if (is_GRAYSCALE_USE()) {

			CLR = FLD_GRY_CLR;
			LWD = FLD_GRY_LNW;
			DSH = FLD_GRY_DSH;
		}
		else {

			CLR = FLD_RGB_CLR;
			LWD = FLD_RGB_LNW;
			DSH = FLD_RGB_DSH;
		}
	}
	else if (C) {
		if (is_GRAYSCALE_USE()) {

			CLR = C_GRY_CLR;
			LWD = C_GRY_LNW;
			DSH = C_GRY_DSH;
		}
		else {

			CLR = C_RGB_CLR;
			LWD = C_RGB_LNW;
			DSH = C_RGB_DSH;
		}
	}
	else {

		CLR = i.PSCOLOR;
		LWD = is_LINEWIDTH();
		DSH = i.DASHED;
	}
	color_PS (o, CLR);
	linewidth_PS (o, LWD, 1);
	newpath_PS (o);

	if (is_NET_WULFF()) {

		double BA = 0.0;

		if 	    ((X_B > X_O) && (Y_B > Y_O)) 	BA =         ASIN((Y_B - Y_O) / r);
		else if ((X_B < X_O) && (Y_B > Y_O)) 	BA = 180.0 - ASIN((Y_B - Y_O) / r);
		else if ((X_B < X_O) && (Y_B < Y_O)) 	BA = 180.0 + ASIN((Y_O - Y_B) / r);
		else 									BA = 360.0 - ASIN((Y_O - Y_B) / r);

		double EA = BA + (2.0 * alfa);

		arc_PS (o, X_O, Y_O, r, BA, EA, 3);
	}
	else {

		moveto_PS(o, PP.at(0).X, PP.at(0).Y, 3);

		for (size_t j = 1; j < steps * 2; j+=3) {

			curveto_PS (o, PP.at(j).X, PP.at(j).Y, PP.at(j+1).X, PP.at(j+1).Y, PP.at(j+2).X, PP.at(j+2).Y, 3);
		}
	}
	setdash_PS (o, DSH);
	stroke_PS(o);

	setdash_PS (o, "   ");
}

void PS_polepoint (const GDB& i, ofstream& o, const double X, const double Y, const double R, const string TYPE) {

	VCTR O;

	const bool FL = TYPE == "FOLD";
	const bool AV = TYPE == "AV";
	const bool C = TYPE == "C";
	const bool AVO = TYPE == "AVO";
	const bool ID = TYPE == "IDEAL";

	const bool G = is_GRAYSCALE_USE();

	if (FL) 			O = i.fold_great_circle_N;
	else if (AV || AVO) O = NXNYNZ_from_DXDYDZ (i.avS0D);
	else if (C) 		O = i.NC;
	else if (ID) 		O = unitvector (i.SHEAR_S);
	////else if (ID) 		O = flip_N_vector (unitvector (i.SHEAR_S));
	else 				O = i.N;

	if (is_N_down (O)) O = flip_vector(O);

	O = declare_vector( -O.X, -O.Y, O.Z);

	if (is_NET_SCHMIDT()) {

		O.X = (O.X / sqrt (1.00 + O.Z)) * R;
		O.Y = (O.Y / sqrt (1.00 + O.Z)) * R;
	}
	else {

		O.X = (O.X / (1.00 + O.Z)) * R;
		O.Y = (O.Y / (1.00 + O.Z)) * R;
	}

	if (is_HEMISPHERE_UPPER()) {

		O.X = - O.X;
		O.Y = - O.Y;
	}

	O.X = O.X + X;
	O.Y = O.Y + Y;

	string CLR = "";

	if (AV) {

		if (G) 	CLR = AV_GRY_CLR;
		else 	CLR = AV_RGB_CLR;
	}
	else if (AVO) {

		if (G) 	CLR = AVO_GRY_CLR;
		else 	CLR = AVO_RGB_CLR;
	}
	else if (FL) {

		if (G) 	CLR = FLD_GRY_CLR;
		else 	CLR = FLD_RGB_CLR;
	}
	else if (C) {

		if (G) 	CLR = C_GRY_CLR;
		else 	CLR = C_RGB_CLR;
	}
	else if (ID) {

		if (G) 	CLR = "0.0 0.0 0.0";
		else 	CLR = "0.0 0.0 0.5";
	}
	else CLR = i.PSCOLOR;

	newpath_PS (o);
	color_PS (o, CLR);
	linewidth_PS (o, 1, 1);
	arc_PS (o, O.X, O.Y, 1.2, 0.0, 360.0, 3);
	stroke_PS (o);

	if (is_LABELLING_USE() && !is_PLOT_HOEPPENER()) {

		font_PS(o, "ArialNarrow-Italic", 6);
		color_PS (o, "0.5 0.5 0.5");
		text_PS (o, "%%--------2");
		const string T = double_to_string (i.avS0d.DIPDIR, 0) + "/" + double_to_string (i.avS0d.DIP, 0);
		text_PS (o, O.X + 3.0, O.Y - 3.0, 3, T);
	}
}

void PS_striaearrow (const GDB& i, ofstream& o, const CENTER& center) {

	double ANGLE = NaN();
	string TEXT = "";

	VCTR DATA;// = i.DC;
	//if (is_DC_up(DATA)) DATA = flip_vector(DATA);

	if (is_PLOT_HOEPPENER()) {

		//DATA = declare_vector (-i.N.X, -i.N.Y, -i.N.Z);
		DATA = declare_vector (-i.N.X, -i.N.Y, -i.N.Z);
	}
	else{

		DATA = i.DC;
		if (is_DC_up(DATA)) DATA = flip_vector(DATA);
	}

	double X = (DATA.X / (1.00 - DATA.Z)) * center.radius;
	double Y = (DATA.Y / (1.00 - DATA.Z)) * center.radius;

	if (is_NET_SCHMIDT()) {

		X = (DATA.X / (sqrt(1.00 - DATA.Z))) * center.radius;
		Y = (DATA.Y / (sqrt(1.00 - DATA.Z))) * center.radius;
	}

	if (is_HEMISPHERE_UPPER()) {

		X = - X;
		Y = - Y;
	}

	X = X + center.X;
	Y = Y + center.Y;

	if (is_CHK_PLOT_STRIAE()) {

		if (i.ID == "ANG001") {

			cout << "**** PS_lineation test - if fails listed below." << endl;

			if (! is_in_range (239.87819162, 239.87819162, X)) cout << "X test failed: " << 239.87819162 << " <> " << X << endl;
			if (! is_in_range (567.06172046, 567.06172046, Y)) cout << "X test failed: " << 567.06172046 << " <> " << Y << endl;

			cout << "**** End of PS_plane test." << endl;

			exit (100);
		}
		else cout << "To call PS_plane test run as '--debug standard_output15'." << endl;
	}

	color_PS (o, i.PSCOLOR);

	const bool NONE = is_allowed_striae_none_sense(i.OFFSET);
	const bool NORMAL = is_allowed_striae_normal_sense(i.OFFSET);
	const bool REVERSE = is_allowed_striae_inverse_sense(i.OFFSET);
	const bool SINISTRAL = is_allowed_striae_sinistral_sense(i.OFFSET);
	const bool DEXTRAL = is_allowed_striae_dextral_sense(i.OFFSET);

	if (!NONE && !NORMAL && !REVERSE && !SINISTRAL && !DEXTRAL) ASSERT_DEAD_END();

	if (is_PLOT_HOEPPENER() && !NONE) {

		ANGLE = - i.corrL.DIPDIR;
		////if (i.UP) ANGLE = ANGLE + 180.0;
		TEXT = " newpath normalarrow";
	}
	else {
		if (REVERSE) {

			ANGLE = - i.corrL.DIPDIR + 180.0;
			TEXT = " newpath normalarrow";
		}
		else if (NORMAL) {

			ANGLE = - i.corrL.DIPDIR;
			TEXT = " newpath normalarrow";
		}
		else if (DEXTRAL) {

			ANGLE = - i.corrL.DIPDIR + 90.0;
			TEXT = " newpath dextralarrow";
		}
		else if (SINISTRAL) {

			ANGLE = - i.corrL.DIPDIR + 90.0;
			TEXT = " newpath sinistralarrow";
		}
		else if (NONE) {

			ANGLE = - i.corrL.DIPDIR;
			TEXT = " newpath nonearrow";
		}
		else {}
	}

	translate_PS (o, X, Y, 3);
	rotate_PS (o, ANGLE, 1);
	text_PS (o, TEXT);
	rotate_PS (o, -ANGLE, 1);
	translate_PS (o, -X, -Y, 3);

	if (is_LABELLING_USE()) {

		font_PS (o, "ArialNarrow-Italic", 6);
		text_PS (o, "%%--------3");
		linewidth_PS (o, 0.5, 1);
		translate_PS (o, X, Y, 3);
		moveto_PS (o, 0.0, 0.0, 3);
		lineto_PS (o, 10.0, 10.0, 3);
		stroke_PS (o);

		text_PS(o, 10.0, 10.0, 3, i.ID);
		translate_PS(o, -X, -Y, 3);
	}
	return;
}

void PS_datanumber_averagebedding (const GDB& i, ofstream& o, const PAPER& P, const size_t datanumber) {

	const bool HAS_BEDDING = (is_allowed_DIR (i.avS0d.DIPDIR) && is_allowed_DIP (i.avS0d.DIP));


	//cout << "PS__Y__1" << endl;

	//cout << i.avS0d.DIPDIR << " / " << i.avS0d.DIP << endl;


	if (HAS_BEDDING) {

		if (is_PLOT_HOEPPENER()) 	PS_polepoint (i, o, P.O1X, P.O1Y, P.R, "AV");
		else 						PS_plane     (i, o, P.O1X, P.O1Y, P.R, "AV");
	}


	//cout << "PS__Y__2" << endl;

	font_PS(o, "ArialNarrow-Bold", 8);
	color_PS(o, "0.0 0.0 0.0");

	string T1 = "Data number: " + int_to_string (datanumber);
	text_PS(o, P.O1X - P.R - 0.2 * P.B, P.O1Y + P.R + 1.8 * P.B, 3, T1);
	text_PS(o, P.O2X - P.R - 0.2 * P.B, P.O2Y + P.R + 1.8 * P.B, 3, T1);

	string D, DD;

	string T2 = "Average bedding: ";
	if (!HAS_BEDDING) T2 = T2 + "not measured";
	else {

		DD = double_to_string (i.avS0d.DIPDIR, 0);
		if (DD.size() == 1) DD = "00" + DD;
		if (DD.size() == 2) DD = "0" + DD;

		D = double_to_string (i.avS0d.DIP, 0);
		if (D.size() == 1) D = "0" + D;
		T2 = T2 + DD + "/" + D;
	}

	string O = "";

	if (is_allowed_bedding_overturned_sense (i.avS0offset)) O = ", overturned";

	T2 = T2 + O;

	text_PS(o, P.O1X - P.R - 0.2 * P.B, P.O1Y + P.R + 1.3 * P.B, 3, T2);
	text_PS(o, P.O3X - P.R - 0.2 * P.B, P.O3Y + P.R + 1.3 * P.B, 3, T2);

	string T3 = "Corrected by the average bedding: ";
	if (!HAS_BEDDING) T3 = T3 + "no bedding measured";
	else T3 = T3 + DD + "/" + D;
	T3 = T3 + O;

	string T4 = "Corrected by palaeo north directions";;

	if (is_TILTING_BEDDING()) {

		text_PS (o, P.O2X - P.R - 0.2 * P.B, P.O2Y + P.R + 1.3 * P.B, 3, T3);
		text_PS (o, P.O4X - P.R - 0.2 * P.B, P.O4Y + P.R + 1.3 * P.B, 3, T3);
	}
	else if (is_TILTING_PALEONORTH()) {

		text_PS (o, P.O2X - P.R - 0.2 * P.B, P.O2Y + P.R + 1.3 * P.B, 3, T4);
		text_PS (o, P.O4X - P.R - 0.2 * P.B, P.O4Y + P.R + 1.3 * P.B, 3, T4);
	}
	else {

		text_PS (o, P.O2X - P.R - 0.2 * P.B, P.O2Y + P.R + 1.3 * P.B, 3, T3);
		text_PS (o, P.O2X - P.R - 0.2 * P.B, P.O2Y + P.R + 0.8 * P.B, 3, T4);

		text_PS (o, P.O4X - P.R - 0.2 * P.B, P.O4Y + P.R + 1.3 * P.B, 3, T3);
		text_PS (o, P.O4X - P.R - 0.2 * P.B, P.O4Y + P.R + 0.8 * P.B, 3, T4);
	}
	text_PS(o, P.O3X - P.R - 0.2 * P.B, P.O3Y + P.R + 1.8 * P.B, 3, "Rose plot for measured data");
	text_PS(o, P.O4X - P.R - 0.2 * P.B, P.O4Y + P.R + 1.8 * P.B, 3, "Rose plot for dip corrected data");

	return;
}

void PS_GDB_DATA (const vector <GDB>& inGDB, ofstream& o, const CENTER& center) {

	for (size_t i = 0; i < inGDB.size(); i++) {

		const string DG = inGDB.at(i).DATAGROUP;

		if (is_allowed_lineation_datatype (DG)) 	PS_DRAW_lineation (inGDB.at(i), o, center);
		else if (is_allowed_plane_datatype (DG)) 	PS_DRAW_plane (inGDB.at(i), o, center);
		else if (is_allowed_striae_datatype(DG)) 	PS_DRAW_striae (inGDB.at(i), o, center);
		else if (is_allowed_SC_datatype (DG)) 		PS_DRAW_sc (inGDB.at(i), o, center);
		else ASSERT_DEAD_END();
	}
	return;
}

void PS_GDB (const vector <GDB>& inGDB, ofstream& o, PAPER P, bool TILT) {

	CENTER center, rosecenter, vrosecenter, mohrcenter;

	center.radius = P.R;
	rosecenter.radius = P.R;
	vrosecenter.radius = P.R;

	if (!TILT) {

		center.X = P.O1X;
		center.Y = P.O1Y;
		rosecenter.X = P.O3X;
		rosecenter.Y = P.O3Y;
		vrosecenter.X = P.O5X;
		vrosecenter.Y = P.O5Y;
		mohrcenter.X = P.O7X;
		mohrcenter.Y = P.O7Y;
	}
	else {

		center.X = P.O2X;
		center.Y = P.O2Y;
		rosecenter.X = P.O4X;
		rosecenter.Y = P.O4Y;
		vrosecenter.X = P.O6X;
		vrosecenter.Y = P.O6Y;
		mohrcenter.X = P.O8X;
		mohrcenter.Y = P.O8Y;
	}
	CONTOURING (inGDB, o, P, center, TILT);

	PS_draw_rose_DIPDIR_DIP (inGDB, o, rosecenter, "DIPDIR", TILT);
	PS_draw_rose_DIPDIR_DIP (inGDB, o, vrosecenter, "DIP", TILT);

	PS_GDB_DATA (inGDB, o, center);

	PS_FOLD_GREAT_CIRCLE (inGDB, o, center);

	PS_INVERSION_RESULTS (inGDB, o, center, mohrcenter, P);

	return;
}

void PS_rosesegment (ofstream& o, const CENTER center, const double percentage, const double degree, const bool c_plane) {

	double 						step_angle =  2.5;
	if (is_ROSEBINSIZE_5_00()) 	step_angle =  5.0;
	if (is_ROSEBINSIZE_10_00()) step_angle = 10.0;
	if (is_ROSEBINSIZE_22_50()) step_angle = 22.5;

	const double angle = 90.0 - step_angle;

	const double radius = center.radius * percentage;

	translate_PS (o, center.X, center.Y, 3);
	rotate_PS (o, -degree, 3);

	if (is_GRAYSCALE_USE()) {

		if (c_plane){

			linewidth_PS (o, 1.0, 3);
			color_PS (o, "0.0 0.0 0.0");
		}
		else {

			linewidth_PS (o, 0.7, 3);
			color_PS (o, "0.5 0.5 0.5");
		}
	}
	else {
		if (c_plane) {

			linewidth_PS (o, 1.0, 3);
			color_PS (o, "0.0 0.0 1.0");
		}
		else {

			linewidth_PS (o, 0.7, 3);
			color_PS (o, "0.0 0.5 0.0");
		}
	}
	newpath_PS (o);
	moveto_PS (o, 0.0, 0.0, 3);
	lineto_PS (o, radius * SIN (step_angle), radius * COS(step_angle), 3);
	arc_PS (o, 0.0, 0.0, radius, angle, 90.0, 3);
	moveto_PS (o, 0.0, 0.0, 3);
	lineto_PS (o, 0.0, radius, 3);
	closepath_PS (o);

	if (!c_plane)	{

		text_PS(o, " gsave");

		if (is_GRAYSCALE_USE()) color_PS (o, "0.8 0.8 0.8");
		else 					color_PS (o, "0.0 1.0 0.0");

		text_PS(o, " fill grestore");
	}
	stroke_PS(o);

	rotate_PS(o, degree, 1);
	translate_PS (o, -center.X, -center.Y, 3);
}

void PS_draw_rose_circle (ofstream& o, const CENTER& center, const double percent, const double VERTICAL) {

	double P = percent;

	if (is_ROSETYPE_SYMMETRICAL()) P = percent / 2.0;

	if (P < 0.0001) return;

	double STEP = 0.00005;
	if 		(is_in_range(0.0001, 0.0005, P)) STEP = 0.0001;
	else if (is_in_range(0.0005, 0.0020, P)) STEP = 0.0005;
	else if (is_in_range(0.0020, 0.0050, P)) STEP = 0.001;
	else if (is_in_range(0.0050, 0.0200, P)) STEP = 0.005;
	else if (is_in_range(0.0200, 0.0500, P)) STEP = 0.01;
	else if (is_in_range(0.0500, 0.2000, P)) STEP = 0.05;
	else if (is_in_range(0.2000, 0.5000, P)) STEP = 0.1;
	else 									 STEP = 0.5;

	font_PS(o, "ArialNarrow-Bold", 8);
	translate_PS (o, center.X, center.Y, 3);

	linewidth_PS (o, 0.5, 1);
	if (is_GRAYSCALE_USE()) color_PS(o, "0.0 0.0 0.0");
	else					color_PS(o, "0.5 0.5 0.5");
	setdash_PS(o, "3 3");

	for (double i = STEP; i < P; i+=STEP) {

		newpath_PS (o);

		double ANG = 0.0;
		double X = -5.0;
		double Y = center.radius * (i / P) + 2.0;

		if (VERTICAL) {

			ANG = 270.0;
			X = center.radius * (i / P) - 4.0;
			Y = 5.0;
		}

		arc_PS (o, 0.0, 0.0, center.radius *  i / P, ANG, 360.0, 3);
		stroke_PS (o);

		const string T = double_to_string (i * 100.0, 0) + "%";

		text_PS (o, X, Y, 3, T);
	}
	setdash_PS (o, "   ");
	translate_PS (o, - center.X, - center.Y, 3);
}

void PS_DRAW_plane (const GDB i, ofstream& o, const CENTER& center) {

	const double X = center.X;
	const double Y = center.Y;
	const double R = center.radius;

	if (is_PLOT_HOEPPENER()) PS_polepoint (i, o, X, Y, R, "");
	else 					 PS_plane (i, o, X, Y, R, "");

}

void PS_DRAW_lineation (const GDB& i, ofstream& o, const CENTER& center) {

	STRESSFIELD empty_sf;

	//cout << "PS_LINEATION_called for fault data" << endl;

	if (is_LABELLING_USE()) 		PS_lineation 	(i, o, center, empty_sf, true, "");
	else 							PS_lineation 	(i, o, center, empty_sf, false, "");

	return;
}

void PS_DRAW_striae (const GDB& i, ofstream& o, const CENTER& center) {

	if (is_PLOT_ANGELIER()) PS_plane (i, o, center.X, center.Y, center.radius, "");

	PS_striaearrow (i, o, center);

	return;
}

void PS_DRAW_sc (const GDB& i, ofstream& o, const CENTER& center) {

	const double X = center.X;
	const double Y = center.Y;
	const double R = center.radius;

	//const bool L = is_LABELLING_USE();
	const bool H = is_PLOT_HOEPPENER();

	if (H) {

		PS_polepoint (i, o, X, Y, R, "C");
		PS_polepoint (i, o, X, Y, R, "");
	}
	else {

		PS_plane (i, o, X, Y, R, "C");
		PS_plane (i, o, X, Y, R, "");
	}
	return;
}

void PS_idealmovement (const vector <GDB>& inGDB, ofstream& o, const CENTER& center) {

	if (is_IDEALMOVEMENT_NONE()) return;
	if (is_PLOT_HOEPPENER()) return;

	for (size_t i = 0; i < inGDB.size(); i++) {

		if (vectorlength (inGDB.at(i).SHEAR_S) > 10e-5) {

			PS_polepoint (inGDB.at(i), o, center.X, center.Y, center.radius, "IDEAL");
		}
	}
}

void PS_FOLD_GREAT_CIRCLE (const vector <GDB>& inGDB, ofstream& o, const CENTER& center) {

	//cout << "PS_FOLD_GREAT_CIRCLE" << endl;

	const string DT = inGDB.at(0).DATATYPE;

	if (!is_allowed_foldsurface_processing(DT)) return;

	PS_folddata (inGDB.at(0), o, center);

	//const bool LABEL = is_LABELLING_USE();

	PS_plane (inGDB.at(0), o, center.X, center.Y, center.radius, "FOLD");

	//cout << "end PS_FOLD_GREAT_CIRCLE" << endl;

	return;
}


void PS_INVERSION_RESULTS (const vector <GDB>& inGDB, ofstream& o, const CENTER& center, const CENTER& mohr_center, const PAPER& P) {

	const bool STRIAE = is_allowed_striae_datatype(inGDB.at(0).DATATYPE);
	const bool FRACTURE = inGDB.at(0).DATATYPE == "FRACTURE";

	const bool no_INVERSION = is_INVERSION_NONE();
	const bool no_BINGHAM = is_BINGHAM_NONE();

	if ((STRIAE && no_INVERSION) || (FRACTURE && no_BINGHAM)) return;

	const vector <STRESSTENSOR> STV = inGDB.at(0).STV;
	const vector <STRESSFIELD> SFV = inGDB.at(0).SFV;

	if (STV.size() < 1 || SFV.size() < 1) return;

	PS_stressdata (inGDB, o, center, P, SFV.at(SFV.size() - 1));

	if (STRIAE && !no_INVERSION) {

		PS_stressarrows (o, center, P, SFV.at(SFV.size() - 1));

		PS_mohr_circle (inGDB, o, mohr_center, P);

		PS_RUP_ANG_distribution (inGDB, o, center, P, "RUP");
		PS_RUP_ANG_distribution (inGDB, o, center, P, "ANG");

		PS_stress_state (o, P, center, SFV.at(SFV.size() - 1));

		PS_idealmovement (inGDB, o, center);

		PS_stress_axes (inGDB, o, center);
	}
	else if (FRACTURE && !no_BINGHAM) {

		PS_stress_axes (inGDB, o, center);
	}
	else ASSERT_DEAD_END();
}

void PS_stress_axes (const vector <GDB>& inGDB, ofstream& o, const CENTER& center) {

	const bool MOSTAFA = is_INVERSION_MOSTAFA();

	vector <STRESSFIELD> SF;

	if (inGDB.size() < 1) ASSERT_DEAD_END();

	const size_t SFV_size = inGDB.at(0).SFV.size();
	const bool STRIAE = is_allowed_striae_datatype(inGDB.at(0).DATATYPE);

	if (MOSTAFA) 	SF = inGDB.at(0).SFV;
	else 			SF.push_back (inGDB.at(0).SFV.at (SFV_size - 1));

	const GDB dummy;

	for (size_t i = 0; i < SF.size(); i++) {

		if (STRIAE && MOSTAFA && i < SF.size()) {

			//cout << "PS_LINEATION_called for strss axis" << endl;
			PS_lineation (dummy, o, center, SF.at(i), false, "S1_ITER");
			PS_lineation (dummy, o, center, SF.at(i), false, "S2_ITER");
			PS_lineation (dummy, o, center, SF.at(i), false, "S3_ITER");
		}
		else {
			//cout << "PS_LINEATION_called for strss axis" << endl;
			PS_lineation (dummy, o, center, SF.at(i), false, "S1");
			PS_lineation (dummy, o, center, SF.at(i), false, "S2");
			PS_lineation (dummy, o, center, SF.at(i), false, "S3");
		}
	}
	return;
}

void PS_SYMBOLS_border (ofstream& o, const PAPER& P) {

	newpath_PS (o);

	moveto_PS (o, P.A, 					(P.Y / 2.0) - 0.15 * P.A, 	3);
	lineto_PS (o, P.X - (10.0 * P.A), 	 (P.Y / 2.0) - 0.15 * P.A, 		3);

	moveto_PS (o, P.S2X, P.S2Y, 3);
	lineto_PS (o, P.S6X, P.S6Y, 3);

	moveto_PS (o, P.S5X, P.S5Y, 3);
	lineto_PS (o, P.S1X, P.S1Y, 3);
	lineto_PS (o, P.S4X, P.S4Y, 3);
	lineto_PS (o, P.S3X, P.S3Y, 3);

	linewidth_PS (o, 3, 1);

	color_PS (o, "0.8 0.8 0.8");
	stroke_PS (o);

	moveto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.25, P.S1Y - 0.3 * P.A, 3);
	lineto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.25, P.S4Y + 0.3 * P.A, 3);

	moveto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.5, P.S1Y - 0.3 * P.A, 3);
	lineto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.5, P.S4Y + 0.3 * P.A, 3);

	moveto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.75, P.S1Y - 0.3 * P.A, 3);
	lineto_PS (o, P.S1X + (P.S2X - P.S1X) * 0.75, P.S4Y + 0.3 * P.A, 3);

	linewidth_PS (o, 1, 1);
	color_PS (o, "0.8 0.8 0.8");
	stroke_PS (o);

	font_PS (o, "ArialNarrow-Bold", 12);

	text_PS (o, P.X - (6.1 * P.A), P.Y / 2.0 + 3.5 * P.A, 3, "SYMBOLS");
}

void PS_SYMBOL_draw_plane (ofstream& o, const double X, const double Y, const PAPER& P, const string& TYPE) {

	const bool GROUP = is_allowed_basic_groupcode_str (TYPE);
	const bool AV = TYPE == "AV";
	const bool AV_O = TYPE == "AV_O";
	const bool O = TYPE == "O";
	const bool FOLD = TYPE == "FOLD";
	const bool SC = TYPE == "SC";
	const bool H = is_PLOT_HOEPPENER();

	string PS_COLOR = "";
	string DASH = "   ";
	double LINEWIDTH = NaN();

	if (!GROUP && !AV && !AV_O && !O && !FOLD && !SC) ASSERT_DEAD_END();

	newpath_PS (o);

	if (H && !FOLD) {

		arc_PS(o, P.S1X + 1.2 * X - 0.7 * P.A, P.S1Y - 3.5 * Y + 2.75 * P.A, 1.5, 0.0, 360.0, 3);
	}
	else {

		arc_PS(o, P.S1X + 1.2 * X, P.S1Y - 3.5 * Y, 3.0 * P.A, 80.0, 110.0, 3);
	}

	if (GROUP) {

		PS_COLOR = generate_PSCOLOR_from_GC (TYPE); //itt x is lehet
		if (is_GRAYSCALE_USE()) DASH = generate_DASH (TYPE); //itt x is lehet
		LINEWIDTH = 1.0;
	}
	else if (AV) {

		if (is_GRAYSCALE_USE()) {
			PS_COLOR = AV_GRY_CLR;
			DASH = AV_GRY_DSH;
			LINEWIDTH = AV_GRY_LNW;
		}
		else {
			PS_COLOR = AV_RGB_CLR;
			DASH = AV_RGB_DSH;
			LINEWIDTH = AV_RGB_LNW;
		}
	}
	else if (AV_O) {

		if (is_GRAYSCALE_USE()) {
			PS_COLOR = AVO_GRY_CLR;
			if (!H) DASH = AVO_GRY_DSH;
			LINEWIDTH = AVO_GRY_LNW;
		}
		else {
			PS_COLOR = AVO_RGB_CLR;
			if (!H) DASH = AVO_RGB_DSH;
			LINEWIDTH = AVO_RGB_LNW;
		}
	}
	else if (O) {

		if (is_GRAYSCALE_USE()) {
			PS_COLOR = OTB_GRY_CLR;
			LINEWIDTH = OTB_GRY_LNW;
			DASH = OTB_GRY_DSH;
		}
		else {
			PS_COLOR = OTB_RGB_CLR;
			LINEWIDTH = OTB_RGB_LNW;
			DASH = OTB_RGB_DSH;
		}
	}
	else if (FOLD) {

		if (is_GRAYSCALE_USE()) {
			PS_COLOR = FLD_GRY_CLR;
			DASH = "6 6";
			LINEWIDTH = FLD_GRY_LNW;
		}
		else {
			PS_COLOR = "0.00 0.00 1.00";
			DASH = "6 6";
			LINEWIDTH = 1.0;
		}
	}
	else if (SC) {

		if (is_GRAYSCALE_USE()) {
			PS_COLOR = "0.80 0.80 0.80";
			LINEWIDTH = 2.0;
		}
		else {
			PS_COLOR = "0.80 0.80 0.80";
			LINEWIDTH = 1.0;
		}
	}
	else {}

	color_PS (o, PS_COLOR);
	linewidth_PS (o, LINEWIDTH, 1);
	setdash_PS (o, DASH);
	stroke_PS (o);

	setdash_PS(o, "  ");

	return;
}

void PS_SYMBOLS_STRIAE (ofstream& o, const PAPER& P) {

	const bool A = is_PLOT_ANGELIER ();
	const bool I = is_IDEALMOVEMENT_USE();

	color_PS (o, "0.0 0.0 0.0");

	if (A) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 1.80 * P.A, P, "X");
	translate_PS (o, P.S1X + 0.6 * P.A, P.S1Y - 3.355 * P.A, 3);
	rotate_PS (o, 20.0, 1);
	text_PS(o, " newpath normalarrow");
	rotate_PS (o, -20.0, 1);
	translate_PS (o, - P.S1X - 0.6 * P.A, - P.S1Y + 3.355 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 3.655 * P.A, 3, "Normal offset");

	if (A) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 1.97 * P.A, P, "X");
	translate_PS (o, P.S1X + 0.6 * P.A, P.S1Y - 3.955 * P.A, 3);
	rotate_PS (o, 160.0, 1);
	text_PS(o, " newpath normalarrow");
	rotate_PS (o, -160.0, 1);
	translate_PS (o, - P.S1X - 0.6 * P.A, - P.S1Y + 3.955 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 4.255 * P.A, 3, "Reverse offset");

	if (A) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 2.14 * P.A, P, "X");
	translate_PS (o, P.S1X + 0.6 * P.A, P.S1Y - 4.555 * P.A, 3);
	rotate_PS (o, 60.0, 1);
	text_PS(o, " newpath dextralarrow");
	rotate_PS (o, -60.0, 1);
	translate_PS (o, - P.S1X - 0.6 * P.A, - P.S1Y + 4.555 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 4.855 * P.A, 3, "Dextral offset");

	if (A) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 2.31 * P.A, P, "X");
	translate_PS (o, P.S1X + 0.6 * P.A, P.S1Y - 5.155 * P.A, 3);
	rotate_PS (o, 60.0, 1);
	text_PS(o, " newpath sinistralarrow");
	rotate_PS (o, -60.0, 1);
	translate_PS (o, - P.S1X - 0.6 * P.A, - P.S1Y + 5.155 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 5.455 * P.A, 3, "Sinistral offset");

	if (A) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 2.48 * P.A, P, "X");
	translate_PS (o, P.S1X + 0.6 * P.A, P.S1Y - 5.755 * P.A, 3);
	rotate_PS (o, 20.0, 1);
	text_PS(o, " newpath nonearrow");
	rotate_PS (o, -20.0, 1);
	translate_PS (o, - P.S1X - 0.6 * P.A, - P.S1Y + 5.755 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 6.055 * P.A, 3, "Unknown offset");

	if (A && I) PS_SYMBOL_draw_plane (o, 1.0 * P.A, 2.65 * P.A, P, "X");
	color_PS (o, "0.0 0.0 0.0");
	if (I) text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 6.655 * P.A, 3, "Ideal movement");

	linewidth_PS (o, 0.7, 1);
	if (is_GRAYSCALE_USE()) color_PS (o, "0.0 0.0 0.0");
	else color_PS (o, "0.0 0.0 5.0");

	if (I) {

		newpath_PS(o);
		arc_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 6.305 * P.A, 1.5, 0.0, 360.0, 3);
		text_PS (o, " gsave");
		color_PS (o, "1.0 1.0 1.0");
		text_PS (o, "fill grestore");
		stroke_PS (o);
	}
	return;
}

void PS_SYMBOLS_PLANE (const string& DATATYPE, ofstream& o, const PAPER& P) {

	const bool FOLDSURFACE = is_allowed_foldsurface_processing(DATATYPE);
	const bool BEDDING = is_allowed_handle_as_bedding(DATATYPE);
	const bool C = is_allowed_SC_datatype(DATATYPE);
	const double X = P.S1X + 0.6 * P.A + 5.0 * P.D;
	const double Y = P.S1Y - 3.355 * P.A;

	color_PS (o, "0.0 0.0 0.0");

	if (C)	{
		text_PS (o, X + 5.0 * P.D, Y + 2.50 * P.A, 3, "Schistosity");
		text_PS (o, X + 5.0 * P.D, Y + 1.80 * P.A, 3, "Cleavage");
	}
	else if (BEDDING) {

		text_PS (o, X + 5.0 * P.D, Y + 2.50 * P.A, 3, "Bedding");
		text_PS (o, X + 5.0 * P.D, Y + 1.80 * P.A, 3, "Bedding");
		text_PS (o, X + 5.0 * P.D, Y + 1.62 * P.A, 3, "Overturned");
	}
	else text_PS (o, X + 5.0 * P.D, Y + 2.50 * P.A, 3, DATATYPE);

	text_PS (o, X + 5.0 * P.D, Y + 1.10 * P.A, 3, "Average bedding");
	text_PS (o, X + 5.0 * P.D, Y + 0.40 * P.A, 3, "Average bedding");
	text_PS (o, X + 5.0 * P.D, Y + 0.22 * P.A, 3, "Overturned");

	PS_SYMBOL_draw_plane (o, P.A, 1.00 * P.A, P, "X");
	if (C) PS_SYMBOL_draw_plane (o, P.A, 1.20 * P.A, P, "SC");
	if (BEDDING) PS_SYMBOL_draw_plane (o, P.A, 1.20 * P.A, P, "O");
	PS_SYMBOL_draw_plane (o, P.A, 1.40 * P.A, P, "AV");
	PS_SYMBOL_draw_plane (o, P.A, 1.60 * P.A, P, "AV_O");

	if (FOLDSURFACE) {

		PS_SYMBOL_draw_plane (o, P.A, 1.80 * P.A, P, "FOLD");
		color_PS (o, "0.0 0.0 0.0");
		text_PS (o, X + 5.0 * P.D, Y - (0.3 * P.A), 3, "Fold great circle");
	}
	return;
}

void PS_SYMBOLS_LINEATION (const string& DATATYPE, ofstream& o, const PAPER& P) {

	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 0.855 * P.A, 3, DATATYPE);

	text_PS (o, P.S1X + 0.6 * P.A + 10.0 * P.D, P.S1Y - 3.355 * P.A + 1.10 * P.A, 3, "Average bedding");
	text_PS (o, P.S1X + 0.6 * P.A + 10.0 * P.D, P.S1Y - 3.355 * P.A + 0.40 * P.A, 3, "Average bedding");
	text_PS (o, P.S1X + 0.6 * P.A + 10.0 * P.D, P.S1Y - 3.355 * P.A + 0.22 * P.A, 3, "Overturned");

	PS_SYMBOL_draw_plane (o, 1.0 * P.A, 1.40 * P.A, P, "AV");
	PS_SYMBOL_draw_plane (o, 1.0 * P.A, 1.60 * P.A, P, "AV_O");

	color_PS (o, "0.0 0.0 0.0");
	newpath_PS(o);
	linewidth_PS(o, 1.5, 1);
	arc_PS(o, P.S1X + 0.25 * P.A, P.S1Y - 0.555 * P.A, 1.5, 0.0, 360, 3);
	stroke_PS(o);
}

void PS_SYMBOLS_GROUPS (ofstream& o, const PAPER& P) {

	const double X = 6.8 * P.A;
	double Y = 1.4 * P.A;

	const bool GROUPS = is_COLOURING_GROUPCODE();
	const bool KMEANS = is_COLOURING_KMEANS();
	const bool RUP = is_COLOURING_RUPANG();

	if (GROUPS || KMEANS || RUP) {

		for (size_t i = 1; i < allowed_basic_groupcode_str_vector().size(); i++) {

			const string GROUP = allowed_basic_groupcode_str_vector().at(i);

			PS_SYMBOL_draw_plane (o, X, Y + (i-1)*0.18*P.A - 10.0*P.D, P, GROUP);

			color_PS (o, "0.0 0.0 0.0");

			text_PS(o, P.S1X + 7.5 * P.A, P.S1Y - 0.855 * P.A - 0.63*(i-1)*P.A, 3, "Group '" + GROUP + "'");
		}
	}
	else {

		const string GROUP = allowed_basic_groupcode_str_vector().at(0);

		PS_SYMBOL_draw_plane (o, X, Y, P, GROUP);

		color_PS (o, "0.0 0.0 0.0");

		text_PS(o, P.S1X + 7.5 * P.A, P.S1Y - 0.855 * P.A, 3, "Default group");
	}
	return;
}

void PS_SYMBOLS_INVERSION (ofstream& o, const PAPER& P) {

	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 1.255 * P.A, 3);
	text_PS (o, " newpath s1_axis");
	translate_PS (o, - P.S1X - 3.4 * P.A, - P.S1Y + 1.255 * P.A, 3);
	color_PS(o, "0.0 0.0 0.0");
	text_PS(o, P.S1X + 3.1 * P.A, P.S1Y - 1.555 * P.A, 3, "S1 axis");

	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 1.955 * P.A, 3);
	text_PS (o, " newpath s2_axis");
	translate_PS (o, - P.S1X - 3.4 * P.A, - P.S1Y + 1.955 * P.A, 3);
	color_PS(o, "0.0 0.0 0.0");
	text_PS(o, P.S1X + 3.1 * P.A, P.S1Y - 2.255 * P.A, 3, "S2 axis");

	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 2.655 * P.A, 3);
	text_PS (o, " newpath s3_axis");
	translate_PS (o, - P.S1X - 3.4 * P.A, - P.S1Y + 2.655 * P.A, 3);
	color_PS(o, "0.0 0.0 0.0");
	text_PS(o, P.S1X + 3.1 * P.A, P.S1Y - 2.955 * P.A, 3, "S3 axis");

	translate_PS(o, P.S1X + 3.4 * P.A, P.S1Y - 3.955 * P.A, 3);
	rotate_PS(o, 90.0, 1);
	translate_PS(o, 0.0, 10.0, 3);
	text_PS(o, " newpath compression_arrow");
	translate_PS(o, 0.0, -10.0, 3);
	rotate_PS(o, 180.0, 1);
	translate_PS(o, 0.0, 10.0, 3);
	text_PS(o, " newpath compression_arrow");
	translate_PS(o, 0.0, -10.0, 3);
	rotate_PS(o, 180.0, 1);
	rotate_PS(o, -90.0, 1);
	translate_PS(o, - P.S1X - 3.4 * P.A, - P.S1Y + 3.955 * P.A, 3);
	text_PS(o, P.S1X + 2.4 * P.A, P.S1Y - 4.455 * P.A, 3, "Maximum horizontal stress");

	translate_PS(o, P.S1X + 3.4 * P.A, P.S1Y - 5.355 * P.A, 3);
	rotate_PS(o, 90.0, 1);
	translate_PS(o, 0.0, 10.0, 3);
	text_PS(o, " newpath extension_arrow");
	translate_PS(o, 0.0, -10.0, 3);
	rotate_PS(o, 180.0, 1);
	translate_PS(o, 0.0, 10.0, 3);
	text_PS(o, " newpath extension_arrow");
	translate_PS(o, 0.0, -10.0, 3);
	rotate_PS(o, 180.0, 1);
	rotate_PS(o, -90.0, 1);
	translate_PS(o, - P.S1X - 3.4 * P.A, - P.S1Y + 5.355 * P.A, 3);
	text_PS(o, P.S1X + 2.4 * P.A, P.S1Y - 5.8055 * P.A, 3, "Minimum horizontal stress");
}

void PS_SYMBOLS_BINGHAM (ofstream& o, const PAPER& P) {

	color_PS (o, "0.0 0.0 0.0");
	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 1.255 * P.A, 3);
	text_PS (o, "newpath s1_axis");
	translate_PS (o, -P.S1X - 3.4 * P.A, -P.S1Y + 1.255 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 2.6 * P.A, P.S1Y - 1.555 * P.A, 3, "Maximum weight point");

	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 1.955 * P.A, 3);
	text_PS (o, "newpath s2_axis");
	translate_PS (o, - P.S1X - 3.4 * P.A, - P.S1Y + 1.955 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 2.5 * P.A, P.S1Y - 2.255 * P.A, 3, "Intermediate weight point");

	translate_PS (o, P.S1X + 3.4 * P.A, P.S1Y - 2.655 * P.A, 3);
	text_PS (o, "newpath s3_axis");
	translate_PS (o, - P.S1X - 3.4 * P.A, - P.S1Y + 2.655 * P.A, 3);
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 2.6 * P.A, P.S1Y - 2.955 * P.A, 3, "Minimum weight point");
}

void PS_SYMBOLS_ROSE (const vector <GDB>& inGDB, ofstream& o, const PAPER& P) {

	const double angle = 80.0;
	const double radius = 80.0 * P.D;

	const string DATAGROUP = inGDB.at(0).DATAGROUP;

	const bool LINEATION = is_allowed_lineation_datatype (DATAGROUP);
	const bool PLANE = is_allowed_plane_datatype (DATAGROUP);
	const bool SC = is_allowed_SC_datatype (DATAGROUP);
	const bool STRIAE = is_allowed_striae_datatype (DATAGROUP);

	string outtext1 = "";

	if (LINEATION) 	 outtext1 = "Lineation direction";
	else if (SC) 	 outtext1 = "Schistosity dip";
	else if (STRIAE) outtext1 = "Plane dip direction";
	else 			 outtext1 = "Plane dip direction";

	translate_PS (o, P.S1X + 4.9 * P.A, P.S1Y - 2.955 * P.A, 3);//ok
	rotate_PS (o, -30, 1);

	linewidth_PS (o, 0.7, 1);
	color_PS (o, "0.5 0.5 0.5");
	newpath_PS (o);
	moveto_PS (o, 0.0, 0.0, 1);
	lineto_PS (o, radius * SIN (10.0), radius * COS (10.0), 3);

	arc_PS (o, 0.0, 0.0, radius, angle, 90.0, 3);

	moveto_PS (o, 0.0, 0.0, 3);
	lineto_PS (o, 0.0, radius, 3);
	closepath_PS (o);

	gsave_PS (o);
	if (is_GRAYSCALE_USE()) color_PS (o, "0.8 0.8 0.8");
	else 					color_PS (o, "0.0 1.0 0.0");
	fill_PS (o);
	grestore_PS (o);
	stroke_PS (o);

	rotate_PS (o, 30.0, 1);
	translate_PS (o, - (P.S1X + 4.9 * P.A), - (P.S1Y - 2.955 * P.A), 3);//ok
	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 5.0 * P.A, P.S1Y - 3.655 * P.A, 3, outtext1);

	if (LINEATION || PLANE) return;

	string 	outtext2 = "Lineation dip direction";
	if (SC) outtext2 = "Cleavage dip";

	translate_PS (o, P.S1X + 4.9 * P.A, P.S1Y - 5.755 * P.A, 3);
	rotate_PS (o, -30, 1);

	linewidth_PS (o, 1.0, 1);
	if (is_GRAYSCALE_USE()) color_PS(o, "0.0 0.0 0.0");
	else 					color_PS(o, "0.0 0.0 1.0");

	newpath_PS (o);
	moveto_PS (o, 0.0, 0.0, 3);
	lineto_PS (o, radius * SIN(10.0), radius * COS(10.0), 3);

	arc_PS (o, 0.0, 0.0, radius, angle, 90.0, 3);

	moveto_PS (o, 0.0, 0.0, 3);
	lineto_PS(o, 0.0, radius, 3);
	closepath_PS(o);
	stroke_PS (o);

	rotate_PS (o, 30, 1);
	translate_PS (o, - (P.S1X + 4.9 * P.A), - (P.S1Y - 5.755 * P.A), 3);

	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 4.9 * P.A, P.S1Y - 6.455 * P.A, 3, outtext2);

	return;
}

void PS_SYMBOLS_LABEL (ofstream& o, const PAPER& P) {

	if (! is_LABELLING_USE()) return;

	font_PS (o, "ArialNarrow-Italic", 6);
	text_PS (o, "%%--------4");
	linewidth_PS (o, 0.5, 1);
	color_PS (o, "0.5 0.5 0.5");


	translate_PS (o, P.S1X + 0.3 * P.A, P.S1Y - 1.55 * P.A, 3);
	rotate_PS (o, 90.0, 1);

	moveto_PS (o, P.D * 28.0, P.D * 2.0, 1);
	lineto_PS (o, P.D * 35.0, P.D * 2.0, 1);
	stroke_PS (o);

	text_PS(o, P.D * 0.0, P.D * 0.0, 3, "OXAM_001");

	rotate_PS (o, -90.0, 1);
	translate_PS (o, -(P.S1X + 0.3 * P.A), -(P.S1Y - 1.55 * P.A), 3);

	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 0.6 * P.A + 5.0 * P.D, P.S1Y - 3.355 * P.A + 2.14 * P.A, 3, "with labeling");

	return;
}

void PS_STEREONET_SYMBOLS (const vector <GDB>& inGDB, ofstream& o, const PAPER& P) {

	const string DATAGROUP = inGDB.at(0).DATAGROUP;
	const string DATATYPE = inGDB.at(0).DATATYPE;

	string DATAGROUP_TEXT = "";

	const bool LINEATION = is_allowed_lineation_datatype (DATAGROUP);
	const bool PLANE = is_allowed_plane_datatype (DATAGROUP);
	const bool SC = is_allowed_SC_datatype (DATAGROUP);
	const bool STRIAE = is_allowed_striae_datatype (DATAGROUP);

	PS_SYMBOLS_border (o, P);
	PS_SYMBOLS_LABEL (o, P);

	font_PS (o, "ArialNarrow", 8);

	color_PS (o, "0.0 0.0 0.0");
	text_PS (o, P.S1X + 5.2 * P.A, P.S1Y - 0.3 * P.A, 3, "ROSE PLOT");
	color_PS (o, "0.0 0.0 0.0");
	text_PS(o, P.S1X + 7.4 * P.A, P.S1Y - 0.3 * P.A, 3, "GROUPS");

	PS_SYMBOLS_ROSE (inGDB, o, P);

	PS_SYMBOLS_GROUPS (o, P);

	color_PS (o, "0.0 0.0 0.0");

	if (PLANE) {

		text_PS(o, P.S1X + 0.8 * P.A, P.S1Y - 0.3 * P.A, 3, "PLANES");
		PS_SYMBOLS_PLANE (DATATYPE, o, P);

		color_PS (o, "0.0 0.0 0.0");
		text_PS (o, P.S1X + 2.5 * P.A, P.S1Y - 0.3 * P.A, 3, "BINGHAM STATISTICS");

		const bool FRACTURE = DATATYPE == "FRACTURE";

		if (is_BINGHAM_USE() && FRACTURE) PS_SYMBOLS_BINGHAM (o, P);
	}
	else if (LINEATION) {

		text_PS(o, P.S1X + 0.6 * P.A, P.S1Y - 0.3 * P.A, 3, "LINEATION");
		PS_SYMBOLS_LINEATION (DATATYPE, o, P);
	}
	else if (SC) {

		text_PS(o, P.S1X + 0.1 * P.A, P.S1Y - 0.3 * P.A, 3, "SCHISTOSITY, CLEAVEGE");
		PS_SYMBOLS_PLANE (DATATYPE, o, P);
	}
	else if (STRIAE) {

		text_PS (o, P.S1X + 0.1 * P.A, P.S1Y - 0.3 * P.A, 3, "FAULT AND STRIAE DATA");
		PS_SYMBOLS_PLANE (DATATYPE, o, P);
		PS_SYMBOLS_STRIAE (o, P);

		color_PS (o, "0.0 0.0 0.0");
		text_PS (o, P.S1X + 2.6 * P.A, P.S1Y - 0.3 * P.A, 3, "STRESS INVERSION");

		PS_SYMBOLS_INVERSION (o, P);
	}
	else ASSERT_DEAD_END();
}

void newpath_PS (ofstream& o) {

	o << " newpath" << '\n';
}

void color_PS (ofstream& o, const string& RGB) {

	o << " " << RGB << " setrgbcolor" << '\n';
}

void text_PS (ofstream& o, const double X, const double Y, const size_t decimals, const string text) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " moveto (" << text << ") show" << '\n';
}

void text_PS (ofstream& o, const string text) {

	o << " " << text << '\n';
}

void moveto_PS (ofstream& o, const double X, const double Y, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " moveto" << '\n';
}

void curveto_PS (ofstream& o, const double AX, const double AY, const double BX, const double BY, const double CX, const double CY, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << AX << " " << AY << " " << BX << " " << BY << " " << CX << " " << CY << " curveto" << '\n';
}

void lineto_PS (ofstream& o, const double X, const double Y, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " lineto" << '\n';
}

void rlineto_PS (ofstream& o, const double X, const double Y, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " rlineto" << '\n';
}

void translate_PS (ofstream& o, const double X, const double Y, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " translate" << '\n';
}

void rotate_PS (ofstream& o, const double ANG, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << ANG << " rotate" << '\n';
}

void linewidth_PS (ofstream& o, const double LW, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << "  " << LW << " setlinewidth" << '\n';
}

void stroke_PS (ofstream& o) {

	o << " stroke" << '\n';
}

void closepath_PS (ofstream& o) {

	o << "  closepath " << '\n';
}

void font_PS (ofstream& o, const string& font, const size_t size) {

	o << "/" << font << " findfont " << size << " scalefont setfont" << '\n';
}

void arc_PS (ofstream& o, const double X, const double Y, const double R, const double ANG_STR, const double ANG_END, const size_t decimals) {

	o << fixed << setprecision(decimals) << flush;
	o << " " << X << " " << Y << " " << R << " " << ANG_STR << " " << ANG_END << " arc" << '\n';
}

void gsave_PS (ofstream& o) {

	o << " gsave" << '\n';
}

void fill_PS (ofstream& o) {

	o << " fill" << '\n';
}

void grestore_PS (ofstream& o) {

	o << " grestore" << '\n';
}

void setdash_PS (ofstream& o, const string DASH) {

	o << " [" << DASH << "] 0 setdash" << '\n';
}

void OUTPUT_TO_PS (const vector <vector <GDB> >& in_GDB_G, const vector <vector <GDB> >& t_GDB_G) {

	if (in_GDB_G.size() != t_GDB_G.size()) ASSERT_DEAD_END();

	//const bool IGNORE = is_GROUPSEPARATION_IGNORE ();
	//const bool by_GROUPCODE = is_GROUPSEPARATION_GROUPCODE ();
	//const bool by_KMEANS = is_GROUPSEPARATION_KMEANS ();
	//const bool by_RUPANG = is_GROUPSEPARATION_RUPANG ();

	//if (!IGNORE && !by_GROUPCODE && !by_KMEANS && !by_RUPANG) ASSERT_DEAD_END() ;

	//const string BS = path_separator;
	//const string US = "_";

	for (size_t i = 0; i < in_GDB_G.size(); i++) {

		setup_ACTUAL_DATATYPE 	(in_GDB_G.at(i).at(0).DATATYPE);
		setup_ACTUAL_LOCATION 	(in_GDB_G.at(i).at(0).LOC);
		setup_ACTUAL_GROUPCODE 	(in_GDB_G.at(i).at(0).GC);
		setup_ACTUAL_FORMATION 	(in_GDB_G.at(i).at(0).FORMATION);

		const string DT = return_ACTUAL_DATATYPE();

		const bool LITHOLOGY = is_allowed_lithology_datatype (DT);

		if (!LITHOLOGY) {

			//string PS_NAME = P.pssep + BS + DT + BS + LOC + US + DT;

			//if (TRJ) PS_NAME = PS_NAME + "_TRAJECTORY_CORRECTED";

			//if (by_GROUPCODE) 	PS_NAME = PS_NAME + US + in_GDB_G.at(i).at(0).GC.at(0);
			//else if (by_KMEANS) PS_NAME = PS_NAME + US + in_GDB_G.at(i).at(0).GC.at(1);
			//else if (by_RUPANG) PS_NAME = PS_NAME + US + in_GDB_G.at(i).at(0).GC.at(2);
			//else {}

			//PS_NAME = PS_NAME + ".EPS";

			const string PS_NAME = generate_ACTUAL_PS_NAME();

			ofstream OPS (PS_NAME.c_str());

			PS_stereonet_header (OPS);

			const PAPER PPR = PS_dimensions (false);

			PS_STEREONET_SYMBOLS (in_GDB_G.at(i), OPS, PPR);

			if (is_allowed_striae_datatype (DT) && ! is_INVERSION_NONE()) PS_stress_scale (OPS, PPR);

			PS_border (in_GDB_G.at(i), OPS, PPR);

			PS_GDB (in_GDB_G.at(i), OPS, PPR, false);
			PS_GDB (t_GDB_G.at(i), OPS, PPR, true);

			PS_datanumber_averagebedding (in_GDB_G.at(i).at(0), OPS, PPR, in_GDB_G.at(i).size());

			PS_net (OPS, PPR);
		}
	}
	return;
}
