/*
 * well_ps.cpp
 *
 *  Created on: 25 Nov 2014
 *      Author: sasvariagoston_admin
 */

#include <fstream>
#include <iomanip>
#include <iostream>
//#include <sstream>
#include <stdlib.h>
#include <vector>

#include "allowed_keys.hpp"
#include "assertions.hpp"
#include "data_io.h"
#include "run_mode.h"
#include "platform_dep.hpp"
#include "ps.h"
#include "rgf.h"
#include "structs.h"
#include "settings.hpp"
#include "well.hpp"
#include "well_ps.hpp"

namespace {

const double PL_WDT = 4.0;

const double PL_GP = 0.9;
const double PL_AX_GP = 0.2;

const double PL_LF = -2.5;

}

using namespace std;

/*Average
 *
 * horizontal:
 * average
 * bingham
 *
 *
 *
 * vertical:
 * xx
 *
 * either so many measurements,
 * or interval length
 *
 */

void PS_well_header (const string DATATYPE, const string LOC, ofstream& o) {

	const string filename = LOC + "_" + DATATYPE + ".EPS";

	o << "%!PS-Adobe-3.0 EPSF-3.0" << '\n';
	o << "%%BoundingBox:  0 0 842 1191" << '\n';
	o << "%%Title: " << filename << '\n';
	o << "%%Creator: SG2PS" ;

	if (!(is_mode_DEBUG())) o << " (" << version_id() << ")" ;

	o << '\n';
	o << "%%EndComments" << '\n' << '\n';
	o << "<</PageSize [ 842 1191 ]>> setpagedevice " << '\n';

	text_PS (o, "/measurement {");
	newpath_PS (o);
	moveto_PS (o,  -1.0,  0.0, 3);
	rlineto_PS (o,  2.0,  0.0, 3);
	rlineto_PS (o,  0.0,  2.0, 3);
	rlineto_PS (o, -2.0,  0.0, 3);
	closepath_PS (o);
	color_PS (o, "0.00 0.00 0.00");
	fill_PS (o);
	stroke_PS(o);

	linewidth_PS(o, 0.5, 1);

	color_PS (o, "1.00 1.00 1.00");

	newpath_PS (o);
	moveto_PS (o,  -1.0,  0.0, 3);
	rlineto_PS (o,  2.0,  0.0, 3);
	rlineto_PS (o,  0.0,  2.0, 3);
	rlineto_PS (o, -2.0,  0.0, 3);
	closepath_PS (o);
	stroke_PS(o);

	text_PS (o, "} def");

	return;
}

void PS_well_border (const vector <GDB>& inGDB, ofstream& o, const PAPER& P, const bool TILT) {

	const string DT = inGDB.at(0).DATATYPE;
	const string LOC = inGDB.at(0).LOC;
	const string GC = inGDB.at(0).GC;
	const string FM = inGDB.at(0).FORMATION;

	//const bool STRIAE = is_allowed_striae_datatype (DT);

	const bool asked_KMEANS = !is_CLUSTERING_NONE();
	//const bool asked_RUPANG = !is_RUP_CLUSTERING_NONE();

	const bool by_GROUPCODE = is_GROUPSEPARATION_GROUPCODE ();
	//const bool by_KMEANS = is_GROUPSEPARATION_KMEANS ();
	const bool by_RUPANG = is_GROUPSEPARATION_RUPANG ();
	//const bool IGNORE = is_GROUPSEPARATION_IGNORE ();

	const bool color_by_COLORCODE = is_COLOURING_COLORCODE();
	const bool color_by_GROUPCODE = is_COLOURING_GROUPCODE();
	//const bool color_by_KMEANS = is_COLOURING_KMEANS ();
	const bool color_by_RUPANG = is_COLOURING_RUPANG ();
	const bool color_IGNORE = is_COLOURING_IGNORE ();

	const bool exists_GROUPCODE = inGDB.at(0).GC.at(0) != 'X';
	//const bool exists_KMEANS = inGDB.at(0).GC.at(1) != 'X';
	const bool exists_RUPANG = inGDB.at(0).GC.at(2) != 'X';

	//if (!by_GROUPCODE && !by_KMEANS && !by_RUPANG && !IGNORE) ASSERT_DEAD_END();
	//if (!color_by_COLORCODE && !color_by_GROUPCODE && !color_by_KMEANS && !color_by_RUPANG && !color_IGNORE) ASSERT_DEAD_END();

	if (by_GROUPCODE && GC.size() < 1) ASSERT_DEAD_END();
	//if (by_KMEANS && GC.size() < 2) ASSERT_DEAD_END();
	if (by_RUPANG && GC.size() < 3) ASSERT_DEAD_END();

	newpath_PS(o);

	linewidth_PS ( o, 1.5, 1);

	moveto_PS (o, P.A, P.Y - P.A - 10.0, 3);
	lineto_PS (o, P.A, P.A, 3);
	lineto_PS (o, P.X - P.A, P.A, 3);
	lineto_PS (o, P.X - P.A, P.Y - P.A, 3);

	color_PS (o, "0.0 0.0 0.0");
	stroke_PS (o);

	//color_PS (o, "0.8 0.8 0.8");
	//stroke_PS (o);

	//font_PS (o, "ArialNarrow-Bold", 36);
	//color_PS (o, "0.8 0.8 0.8");
	//text_PS (o, 1.5 * P.A, 			P.Y - 2.5 * P.A, 3, "ORIGINAL");
	//text_PS (o, P.X - 8.5 * P.A, 	P.Y - 2.5 * P.A, 3, "CORRECTED");

	font_PS(o, "ArialNarrow-Bold", 12);
	color_PS (o, "0.0 0.0 0.0");

	string T = DT + " FROM LOCATION " + LOC;

	if (asked_KMEANS) T = T + " - K-MEANS CLUSTERING USED";


	if (by_GROUPCODE) 	T =  T + ", GROUP '" + GC.at(0) + "' USING ORIGINAL GROUPCODE" ;
	else if (is_GROUPSEPARATION_IGNORE()) {}
	else {}

	if (color_by_COLORCODE) {

		T = T + ", COLOURED USING COLOUR CODE";
	}
	else if (color_by_GROUPCODE) {

		if (exists_GROUPCODE) T = T + ", COLOURED USING GROUP CODE";
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
	else {}

	if (TILT) 	T = T + " - BEDDING CORRECTED DATA SET";
	else 		T = T + " - ORIGINAL DATA SET";

	text_PS (o, P.A - P.B + P.D * 20.0, P.Y - P.A - P.D * 5.5, 3, T);

	text_PS(o, "%%-----end PSborder");
}

double well_axes_step (const double MIN, const double MAX) {

	const double DIF = MAX - MIN;

	if (DIF < 10.0) return 1.0;
	else {
		if (DIF < 20.0) return 2.0;
		else {
			if (DIF < 50.0) return 5.0;
			else {
				if (DIF < 100.0) return 10.0;
				else {
					if (DIF < 200.0) return 20.0;
					else {
						if (DIF < 500.0) return 50.0;
						else {
							if (DIF < 1000.0) return 100.0;
							else {
								if (DIF < 2000.0) return 200.0;
								else {
									if (DIF < 5000.0) return 500.0;
									else {
										return 1000.0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

bool has_GDB_DEPTH_value_in_range (const vector <GDB>& inGDB, const double MIN, const double MAX) {

	for (size_t i = 0; i < inGDB.size(); i++) {

		const double D = inGDB.at(i).DEPTH;

		if (is_in_range(MIN, MAX, D)) return true;
	}
	return false;
}

double return_MIN_value (const vector <GDB>& inGDB, const size_t STEP) {

	const vector <GDB> temp = sort_by_DEPTH (inGDB);

	const size_t LIMIT = inGDB.at(inGDB.size() - 1).DEPTH;

	for (double i = 0.0; i < LIMIT; i+=STEP) {

		if (has_GDB_DEPTH_value_in_range (temp, i, i + STEP)) return i;
	}
	return NaN();
}

double return_MAX_value (const vector <GDB>& inGDB, const size_t STEP) {

	const vector <GDB> temp = sort_by_rev_DEPTH (inGDB);

	for (double i = STEP * 2000.0; i > 0.0; i-=STEP) {

		if (has_GDB_DEPTH_value_in_range (temp, i, i + STEP)) return i + STEP;
	}
	return NaN();
}

void PS_well_coordinate_axes_INTERVAL (ofstream& o, const PAPER& P, const double X, const double LENGTH) {

	newpath_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1, 1);

	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X, P.O1Y - LENGTH, 3);
	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X + PL_WDT * P.A, P.O1Y, 3);
	stroke_PS (o);

	/*
	font_PS(o, "ArialNarrow-Bold", 24);
	color_PS (o, "0.9 0.9 0.9");

	text_PS (o, X + (PL_WDT - 3.2) * P.A, P.O1Y + 45.0 * P.D, 3, "BIN SIZE");

	font_PS(o, "ArialNarrow", 12);
	color_PS (o, "0.0 0.0 0.0");

	for (double i = MIN_VAL; i < MAX_VAL + 0.5*STEP; i+=STEP) {

		const double INT = (i - MIN_VAL) / (MAX_VAL - MIN_VAL);

		if (i > MIN_VAL + 0.5*STEP) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + PL_WDT * P.A, P.O1Y - (INT * LENGTH), 3);
			lineto_PS (o, X + 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		moveto_PS (o, X, P.O1Y - (INT * LENGTH), 3);
		lineto_PS (o, X - 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
		stroke_PS(o);

		text_PS (o, X - 40.0 * P.D, P.O1Y - (INT * LENGTH) - 6.0 * P.D, 3, double_to_string (i, 0));
	}
	*/
	return;
}

void PS_well_coordinate_grid_INTERVAL (ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const double STEP) {

	//newpath_PS(o);
	///color_PS (o, "0.0 0.0 0.0");
	//linewidth_PS (o, 1, 1);

	//moveto_PS (o, X, P.O1Y, 3);
	//lineto_PS (o, X, P.O1Y - LENGTH, 3);
	//moveto_PS (o, X, P.O1Y, 3);
	//lineto_PS (o, X + PL_WDT * P.A, P.O1Y, 3);
	//stroke_PS (o);

	font_PS(o, "ArialNarrow-Bold", 24);
	color_PS (o, "0.9 0.9 0.9");

	text_PS (o, X + (PL_WDT - 3.2) * P.A, P.O1Y + 45.0 * P.D, 3, "BIN SIZE");

	font_PS(o, "ArialNarrow", 12);
	color_PS (o, "0.0 0.0 0.0");

	for (double i = MIN_VAL; i < MAX_VAL + 0.5*STEP; i+=STEP) {

		const double INT = (i - MIN_VAL) / (MAX_VAL - MIN_VAL);

		if (i > MIN_VAL + 0.5*STEP) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + PL_WDT * P.A, P.O1Y - (INT * LENGTH), 3);
			lineto_PS (o, X + 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		moveto_PS (o, X, P.O1Y - (INT * LENGTH), 3);
		lineto_PS (o, X - 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
		stroke_PS(o);

		text_PS (o, X - 40.0 * P.D, P.O1Y - (INT * LENGTH) - 6.0 * P.D, 3, double_to_string (i, 0));
	}
	return;
}

void PS_well_coordinate_axes_DIPDIR (ofstream& o, const PAPER& P, const double X, const double LENGTH) {

	newpath_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1, 1);

	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X, P.O1Y - LENGTH, 3);
	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X + PL_WDT * P.A, P.O1Y, 3);
	stroke_PS (o);

	font_PS(o, "ArialNarrow-Bold", 24);
	color_PS (o, "0.9 0.9 0.9");

	if (is_DATARULE_GERMAN()) 	text_PS (o, X + (PL_WDT - 3.5) * P.A, P.O1Y + 45.0 * P.D, 3, "DIP DIRECTION");
	else text_PS (o, X + (PL_WDT - 1.2) * P.A, P.O1Y + 45.0 * P.D, 3, "STRIKE DIRECTION");

	font_PS(o, "ArialNarrow", 12);

	color_PS (o, "0.0 0.0 0.0");
	text_PS(o, X + 1.5 * P.A, P.O1Y + 25.0 * P.D, 3, "VALUE");

	color_PS (o, "1.0 0.0 0.0");
	text_PS(o, X + 4.3 * P.A, P.O1Y + 25.0 * P.D, 3, "DERIVATE");
}

void PS_well_coordinate_grid_DIPDIR (ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const double STEP) {

	linewidth_PS (o, 1, 1);

	font_PS(o, "ArialNarrow", 12);
	color_PS (o, "0.0 0.0 0.0");

	for (size_t i = 0; i < 13; i++) {

		const double stp = (i * PL_WDT * P.A) / 12.0;

		moveto_PS (o, X + stp, P.O1Y, 3);
		lineto_PS (o, X + stp, P.O1Y + 5.0 * P.D, 3);
		stroke_PS(o);

		if (i == 0 || i == 3 || i == 6 || i == 9 || i == 12) {

			if (i > 0 ) {

				color_PS (o, "0.9 0.9 0.9");

				moveto_PS (o, X + stp, P.O1Y - LENGTH, 3);
				lineto_PS (o, X + stp, P.O1Y - 5.0 * P.D, 3);
				stroke_PS(o);

				color_PS (o, "0.0 0.0 0.0");
			}
			if (i == 0) text_PS (o, X + stp - 3.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (0.0, 0));
			if (i == 3) text_PS (o, X + stp - 8.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (90.0, 0));
			if (i == 6) text_PS (o, X + stp - 12.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (180.0, 0));
			if (i == 9)	text_PS (o, X + stp - 12.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (270.0, 0));
			if (i == 12) text_PS (o, X + stp - 12.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (360.0, 0));
		}
	}

	for (double i = MIN_VAL; i < MAX_VAL + 0.5*STEP; i+=STEP) {

		const double INT = (i - MIN_VAL) / (MAX_VAL - MIN_VAL);

		if (i > MIN_VAL + 0.5*STEP) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + PL_WDT * P.A, P.O1Y - (INT * LENGTH), 3);
			lineto_PS (o, X + 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		moveto_PS (o, X, P.O1Y - (INT * LENGTH), 3);
		lineto_PS (o, X - 5.0 * P.D, P.O1Y - (INT * LENGTH), 3);
		stroke_PS(o);

		//text_PS (o, X - 40.0 * P.D, P.O1Y - (INT * LENGTH) - 6.0 * P.D, 3, double_to_string (i, 0));
	}

	return;
}

void PS_well_coordinate_axes_DIP (ofstream& o, const PAPER& P, const double X, const double LENGTH) {

	newpath_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1, 1);

	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X, P.O1Y - LENGTH, 3);
	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X + PL_WDT * P.A, P.O1Y, 3);
	stroke_PS (o);

	font_PS(o, "ArialNarrow-Bold", 24);
	color_PS (o, "0.9 0.9 0.9");

	text_PS(o, X + (PL_WDT - 2.5) * P.A, P.O1Y + 45.0 * P.D, 3, "DIP ANGLE");

	font_PS(o, "ArialNarrow", 12);

	color_PS (o, "0.0 0.0 0.0");
	text_PS(o, X + 1.5 * P.A, P.O1Y + 25.0 * P.D, 3, "VALUE");

	color_PS (o, "1.0 0.0 0.0");
	text_PS(o, X + 4.3 * P.A, P.O1Y + 25.0 * P.D, 3, "DERIVATE");
}

void PS_well_coordinate_grid_DIP (ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const double STEP) {

	linewidth_PS (o, 1, 1);

	font_PS(o, "ArialNarrow", 12);
	color_PS (o, "0.0 0.0 0.0");

	for (size_t i = 0; i < 4; i++) {

		const double stp = (i * PL_WDT * P.A) / 3.0;

		moveto_PS (o, X + stp, P.O1Y, 3);
		lineto_PS (o, X + stp, P.O1Y + 5.0 * P.D, 3);
		stroke_PS(o);

		if (i > 0 ) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + stp, P.O1Y - LENGTH, 3);
			lineto_PS (o, X + stp, P.O1Y - 5.0 * P.D, 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		if (i == 0) text_PS (o, X + stp - 3.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (0.0, 0));
		if (i == 1) text_PS (o, X + stp - 7.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (30.0, 0));
		if (i == 2) text_PS (o, X + stp - 7.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (60.0, 0));
		if (i == 3) text_PS (o, X + stp - 8.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (90.0, 0));
	}

	for (double i = MIN_VAL; i < MAX_VAL + 0.5*STEP; i+=STEP) {

		const double INT = (i - MIN_VAL) / (MAX_VAL - MIN_VAL);

		if (i > MIN_VAL + 0.5*STEP) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + PL_WDT * P.A, 	P.O1Y - (INT * LENGTH), 3);
			lineto_PS (o, X + 5.0 * P.D, 	P.O1Y - (INT * LENGTH), 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		moveto_PS (o, X, 				P.O1Y - (INT * LENGTH), 3);
		lineto_PS (o, X - 5.0 * P.D, 	P.O1Y - (INT * LENGTH), 3);
		stroke_PS(o);
	}
	return;
}

void PS_well_coordinate_axes_FREQUENCY (ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const double STEP) {

	newpath_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1, 1);

	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X, P.O1Y - LENGTH, 3);
	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X + 4.0 * P.A, P.O1Y, 3);

	font_PS(o, "ArialNarrow-Bold", 24);
	color_PS (o, "0.9 0.9 0.9");

	text_PS(o, X + 1.0 * P.A, P.O1Y + 45.0 * P.D, 3, "FREQUENCY");

	font_PS(o, "ArialNarrow", 12);

	color_PS (o, "0.0 0.0 0.0");
	text_PS(o, X + 1.5 * P.A, P.O1Y + 25.0 * P.D, 3, "VALUE");

	color_PS (o, "1.0 0.0 0.0");
	text_PS(o, X + 4.3 * P.A, P.O1Y + 25.0 * P.D, 3, "DERIVATE");

	color_PS (o, "0.0 0.0 0.0");

	for (size_t i = 0; i < 4; i++) {

		const double stp = (i * 4.0 * P.A) / 3.0;

		if (i == 3) {

			moveto_PS (o, X + stp, P.O1Y, 3);
			lineto_PS (o, X + stp, P.O1Y + 5.0 * P.D, 3);
			stroke_PS(o);

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + stp, P.O1Y - LENGTH, 3);
			lineto_PS (o, X + stp, P.O1Y - 5.0 * P.D, 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		if (i == 0) text_PS (o, X + stp - 3.0 * P.D, P.O1Y + 8.0 * P.D, 3, double_to_string (0.0, 0));
		if (i == 3) text_PS (o, X + stp - 30.0 * P.D, P.O1Y + 8.0 * P.D, 3, "MAX");
	}

	for (double i = MIN_VAL; i < MAX_VAL + 0.5*STEP; i+=STEP) {

		const double INT = (i - MIN_VAL) / (MAX_VAL - MIN_VAL);

		if (i > MIN_VAL + 0.5*STEP) {

			color_PS (o, "0.9 0.9 0.9");

			moveto_PS (o, X + 4.0 * P.A, 	P.O1Y - (INT * LENGTH), 3);
			lineto_PS (o, X + 5.0 * P.D, 	P.O1Y - (INT * LENGTH), 3);
			stroke_PS(o);

			color_PS (o, "0.0 0.0 0.0");
		}
		moveto_PS (o, X, 				P.O1Y - (INT * LENGTH), 3);
		lineto_PS (o, X - 5.0 * P.D, 	P.O1Y - (INT * LENGTH), 3);
		stroke_PS(o);
	}
	return;
}

void PS_derivate_DIPDIR_DIP (ofstream& o, const PAPER& P, const double X) {

	newpath_PS(o);
	color_PS (o, "0.0 0.0 0.0");
	linewidth_PS (o, 1, 1);

	moveto_PS (o, X, P.O1Y, 3);
	lineto_PS (o, X + PL_WDT * P.A * 0.5, P.O1Y, 3);

	stroke_PS (o);

	return;
}

void plot_well_frequency_derivate (const vector <WELL_FREQUENCY> IN, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL) {

	vector <double> DEPTH;
	vector <double> VALUE;

	for (size_t i = 0; i < IN.size() - 1; i++) {

		DEPTH.push_back (IN.at(i).DERIV_DEPTH);
		VALUE.push_back (IN.at(i).DERIV);
	}
	plot_curve (DEPTH, VALUE, o, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "DERIVATE");

	return;
}

void plot_well_frequency (const vector <WELL_FREQUENCY> IN, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL) {

	vector <double> DEPTH;
	vector <double> VALUE;

	for (size_t i = 0; i < IN.size(); i++) {

		DEPTH.push_back (IN.at(i).DEPTH);
		VALUE.push_back (IN.at(i).FREQ);
	}
	plot_curve (DEPTH, VALUE, o, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "FREQUENCY");

	return;
}

vector <XY> cutting_points (const vector <XY>& IN) {

	vector <XY> OUT;

	//handle if IN.size() < 2
	//X = VALUE, Y = depth

	for (size_t i = 0; i < IN.size() - 1; i++) {

		//cout << i << endl;

		//cout << IN.size() << endl;

		const bool LAST = i == IN.size() - 2;

		XY buf = IN.at(i);

		const double ACT = IN.at(i).X;
		const double NXT = IN.at(i+1).X;

		const double ACT_DPT = IN.at(i).Y;
		const double NXT_DPT = IN.at(i+1).Y;

		const bool PLUS = (NXT - ACT) > 180.0; 		// 012 > 355
		const bool MINUS = (NXT - ACT) < -180.0;	// 355 > 012

		//cout << "g7" << endl;

		OUT.push_back (buf);

		if (PLUS) {

			buf.X = -999.99;
			buf.Y = (2.0 * ACT_DPT + NXT_DPT) / 3.0;
			OUT.push_back (buf);

			buf.X = 999.99;
			buf.Y = (ACT_DPT + 2.0 * NXT_DPT) / 3.0;
			OUT.push_back (buf);

		}
		else if (MINUS) {

			buf.X = 999.99;
			buf.Y = (2.0 * ACT_DPT + NXT_DPT) / 3.0;
			OUT.push_back (buf);

			buf.X = -999.99;
			buf.Y = (ACT_DPT + 2.0 * NXT_DPT) / 3.0;
			OUT.push_back (buf);
		}
		else {}


		if (LAST) {

			// << "LAST" << endl;

			OUT.push_back (IN.at(i+1));
		}
	}
	return OUT;
}

vector <XY> generate_xy_vector (const vector <double>& VALUE, const vector <double>& DEPTH, const bool DIPDIR) {

	if (DEPTH.size() != VALUE.size()) ASSERT_DEAD_END();



	vector <XY> OUT;

	//handle if IN.size() < 2

	for (size_t i = 0; i < VALUE.size(); i++) {

		XY buf;

		const double V = VALUE.at(i);

		buf.Y = DEPTH.at(i);

		if (DIPDIR) {

			if (V < 0.0) 		buf.X = 360.0 + V;
			else if (V > 360.0) buf.X = V - 360.0;
			else 				buf.X = V;
		}
		else {

			if (V < 0.0) 		buf.X = 0.0;
			else if (V > 90.0) 	buf.X = 90.0;
			else 				buf.X = V;
		}

		//cout << buf.Y << "  -  " << buf.X << endl;

		OUT.push_back (buf);
	}
	return OUT;
}

vector <vector <XY> >  generate_xy_vector_vector (const vector <XY>& IN) {

	vector <vector <XY> > OUT;

	vector <XY> BUF;

	for (size_t i = 0; i < IN.size() - 1; i++) {

		const bool LAST = i == IN.size() - 2;

		XY buf = IN.at(i);
		BUF.push_back (buf);

		const double ACT_VAL = IN.at(i).X;
		const double NXT_VAL = IN.at(i+1).X;

		const bool CUT = (ACT_VAL < -900.0 && NXT_VAL > 900.0) || (ACT_VAL > 900.0 && NXT_VAL < -900.0);

		if (CUT) {

			OUT.push_back(BUF);
			BUF.clear();
		}

		if (LAST) {

			XY buf = IN.at(i+1);
			BUF.push_back (buf);
			OUT.push_back(BUF);
		}
	}
	return OUT;
}


vector <vector <XY> > tidy_xy_vector_vector (vector <vector <XY> >& IN) {

	vector <vector <XY> > OUT = IN;

	for (size_t i = 0; i < OUT.size(); i++) {
		for (size_t j = 0; j < OUT.at(i).size(); j++) {

			if (OUT.at(i).at(j).X < -900.0) OUT.at(i).at(j).X = 0.0;
			if (OUT.at(i).at(j).X > 900.0) OUT.at(i).at(j).X = 360.0;
		}
	}
	return OUT;
}


vector <double> generate_VALUE_from_XY_vector (const vector <XY>& IN) {

	vector <double> OUT;

	for (size_t i = 0; i < IN.size(); i++) OUT.push_back(IN.at(i).X);

	return OUT;
}


vector <double> generate_DEPTH_from_XY_vector (const vector <XY>& IN) {

	vector <double> OUT;

	for (size_t i = 0; i < IN.size(); i++) OUT.push_back(IN.at(i).Y);

	return OUT;
}

void plot_curve (const vector <double> DEPTH, const vector <double> VALUE, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const bool DIPDIR, const string TYPE) {

	if (DEPTH.size() != VALUE.size()) ASSERT_DEAD_END();

	//cout << "plot_curve" << endl;

	const bool A = TYPE == "AVERAGE";
	const bool E_MN = TYPE == "LOWER_STDEV";
	const bool E_MX = TYPE == "UPPER_STDEV";
	const bool D = TYPE == "DERIVATE";
	const bool F = TYPE == "FREQUENCY";

	double MX = 90.0;
	if (DIPDIR) MX = 360.0;
	if (D) MX = 2.0;
	if (F) MX = 1.0;

	string CLR = "0.80 0.80 0.80";
	if (A) CLR = "0.00 0.00 0.00";
	if (D) CLR = "1.00 0.00 0.00";
	if (F) CLR = "0.00 0.00 0.00";

	double LW = 1.0;
	if (E_MN || E_MX) LW = 2.0;

	color_PS (o, CLR);
	linewidth_PS(o, LW, 1);

	for (size_t i = 0; i < DEPTH.size() - 1; i++) {

		const double ACT_D = DEPTH.at(i);
		const double ACT_V = VALUE.at(i);

		const double NXT_D = DEPTH.at(i + 1);
		const double NXT_V = VALUE.at(i + 1);

		//if (ACT_DATA < 0.0) ACT_DATA = 0.0;
		//if (NXT_DATA < 0.0) NXT_DATA = 0.0;

		const double ACT_data_X = X + (PL_WDT * P.A * (ACT_V / MX ));
		const double NXT_data_X = X + (PL_WDT * P.A * (NXT_V / MX ));

		const double ACT_data_Y = P.O1Y - (LENGTH * ((ACT_D - MIN_VAL) / (MAX_VAL - MIN_VAL)));
		const double NXT_data_Y = P.O1Y - (LENGTH * ((NXT_D - MIN_VAL) / (MAX_VAL - MIN_VAL)));

		moveto_PS(o, ACT_data_X, ACT_data_Y, 3);
		lineto_PS(o, NXT_data_X, NXT_data_Y, 3);
	}
	stroke_PS (o);

	return;
}

double return_plot_value (const WELL_INTERVAL ACT, const bool DIPDIR, const string TYPE) {

	const bool A = TYPE == "AVERAGE";
	const bool E_MN = TYPE == "LOWER_STDEV";
	const bool E_MX = TYPE == "UPPER_STDEV";
	const bool D = TYPE == "DERIVATE";

	if (A && DIPDIR) return ACT.INT_AV_DD.DIPDIR;

	else if (A && !DIPDIR)	return ACT.INT_AV_DD.DIP;

	else if (E_MN && DIPDIR) return ACT.INT_AV_DD.DIPDIR - ACT.INT_AV_DD_STDEV;

	else if (E_MX && DIPDIR) return ACT.INT_AV_DD.DIPDIR + ACT.INT_AV_DD_STDEV;

	else if (E_MN && !DIPDIR) return ACT.INT_AV_DD.DIP - ACT.INT_AV_D_STDEV;

	else if (E_MX && !DIPDIR) return ACT.INT_AV_DD.DIP + ACT.INT_AV_D_STDEV;

	else if (D && DIPDIR) return ACT.DD_DERIV;

	else if (D && !DIPDIR) return ACT.D_DERIV;

	else {

		ASSERT_DEAD_END();
		return NaN();
	}
}

void plot_well_curve (const vector <WELL_INTERVAL>& IN, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const bool DIPDIR, const string TYPE) {

	if (IN.size() < 2) return;

	vector <double> DEPTH;
	vector <double> VALUE;

	//cout << "plot_well_curve" << endl;

	//cout << "IN.size(): " << IN.size() << endl;

	for (size_t i = 0; i < IN.size() - 1; i++) {

		const bool LAST = i == IN.size() - 1;

		const WELL_INTERVAL ACT = IN.at(i);
		const WELL_INTERVAL NXT = IN.at(i + 1);

		//cout << "ACT.SIZE: " << ACT.SIZE << endl;
		//cout << "NXT.SIZE: " << NXT.SIZE << endl;


		if (ACT.SIZE > 0 && NXT.SIZE > 0) {

			const double ACT_DATA = return_plot_value (ACT, DIPDIR, TYPE);
			const double NXT_DATA = return_plot_value (NXT, DIPDIR, TYPE);

			VALUE.push_back (ACT_DATA);
			DEPTH.push_back (ACT.DEPTH);

			if (LAST) {

				VALUE.push_back (NXT_DATA);
				DEPTH.push_back (NXT.DEPTH);
			}
		}
	}

	//ez itt lentebb indefele modositasokat csinal a frequency-gorben, am
	//nem piszkalja a dipdir-gorbet, ha szar

	//cout << "A1" << endl;

	vector <XY> PLOT = generate_xy_vector (VALUE, DEPTH, DIPDIR);

	//cout << "VALUE.size(): " << VALUE.size() << endl;

	//cout << "A2" << endl;

	//cout << "PLOT.size(): " << PLOT.size() << endl;

	PLOT = cutting_points (PLOT);

	//cout << "PLOT.size(): " << PLOT.size() << endl;

	//cout << "A3" << endl;

	vector <vector <XY> > PLOT_V = generate_xy_vector_vector (PLOT);

	//cout << "A4" << endl;

	PLOT_V = tidy_xy_vector_vector (PLOT_V);

	//cout << "A5" << endl;

	for (size_t i = 0; i < PLOT_V.size(); i++) {

		VALUE = generate_VALUE_from_XY_vector (PLOT_V.at(i));
		DEPTH = generate_DEPTH_from_XY_vector (PLOT_V.at(i));

		plot_curve (DEPTH, VALUE, o, P, X, LENGTH, MIN_VAL, MAX_VAL, DIPDIR, TYPE);
	}

	//cout << "PLOT.size(): " << PLOT.size() << endl;
	//	for (size_t i = 0; i < PLOT.size(); i++) {

	//		cout << PLOT.at(i).X << "  -  " << PLOT.at(i).Y << endl;
	//	}


	/*cout << "PLOT.size(): " << PLOT_V.size() << endl;
	for (size_t i = 0; i < PLOT_V.size(); i++) {

		cout << "  --------  " << endl;

		for (size_t j = 0; j < PLOT_V.at(i).size(); j++) {

			cout << PLOT_V.at(i).at(j).X << "  -  " << PLOT_V.at(i).at(j).Y << endl;
		}
	}
	*/

	//exit (555);

	//plot_curve (DEPTH, VALUE, o, P, X, LENGTH, MIN_VAL, MAX_VAL, DIPDIR, TYPE);

	//if (STDEV) plot_curve_stdev (DEPTH_1, VALUE_1, DEPTH_2, VALUE_2, o, P, X, LENGTH, MIN_VAL, MAX_VAL, DIPDIR);

	//cout << "end plot_well_curve" << endl;

	return;
}





/*
 * void plot_well_curve (const vector <WELL_INTERVAL>& IN, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const bool DIPDIR, const string TYPE) {

	const bool A = TYPE == "AVERAGE";
	//const bool E_MN = TYPE == "LOWER_STDEV";
	//const bool E_MX = TYPE == "UPPER_STDEV";
	const bool STDEV = TYPE == "STDEV";
	const bool D = TYPE == "DERIVATE";

	vector <double> DEPTH_1;
	vector <double> VALUE_1;

	vector <double> DEPTH_2;
	vector <double> VALUE_2;

	for (size_t i = 0; i < IN.size() - 1; i++) {

		const bool LAST = i == IN.size() - 1;

		const WELL_INTERVAL ACT = IN.at(i);
		const WELL_INTERVAL NXT = IN.at(i + 1);

		double ACT_DATA_1 = NaN();
		double NXT_DATA_1 = NaN();

		double ACT_DATA_2 = NaN();
		double NXT_DATA_2 = NaN();

		if (ACT.SIZE > 0 && NXT.SIZE > 0) {

			if (A && DIPDIR) {

				ACT_DATA_1 = ACT.INT_AV_DD.DIPDIR;
				NXT_DATA_1 = NXT.INT_AV_DD.DIPDIR;
			}
			else if (A && !DIPDIR) {

				ACT_DATA_1 = ACT.INT_AV_DD.DIP;
				NXT_DATA_1 = NXT.INT_AV_DD.DIP;
			}
			//else if (E_MN && DIPDIR) {

			//	ACT_DATA = ACT.INT_AV_DD.DIPDIR - ACT.INT_AV_DD_STDEV;
			//	NXT_DATA = NXT.INT_AV_DD.DIPDIR - NXT.INT_AV_DD_STDEV;

			//	cout << "ACT_DATA: " << ACT_DATA << endl;
			//	if (LAST) {

			//		cout << "NXT_DATA: " << NXT_DATA << endl;
			//	}
			//}
			//else if (E_MN && !DIPDIR) {

			//	ACT_DATA = ACT.INT_AV_DD.DIP - ACT.INT_AV_D_STDEV;
			//	NXT_DATA = NXT.INT_AV_DD.DIP - NXT.INT_AV_D_STDEV;
			//}
			//else if (E_MX && DIPDIR) {

			//	ACT_DATA = ACT.INT_AV_DD.DIPDIR + ACT.INT_AV_DD_STDEV;
			//	NXT_DATA = NXT.INT_AV_DD.DIPDIR + NXT.INT_AV_DD_STDEV;
			//}
			//else if (E_MX && !DIPDIR) {

			//	ACT_DATA = ACT.INT_AV_DD.DIP + ACT.INT_AV_D_STDEV;
			//	NXT_DATA = NXT.INT_AV_DD.DIP + NXT.INT_AV_D_STDEV;
			//}
			else if (STDEV && DIPDIR) {

				ACT_DATA_1 = ACT.INT_AV_DD.DIPDIR - ACT.INT_AV_DD_STDEV;
				NXT_DATA_1 = NXT.INT_AV_DD.DIPDIR - NXT.INT_AV_DD_STDEV;

				ACT_DATA_2 = ACT.INT_AV_DD.DIPDIR + ACT.INT_AV_DD_STDEV;
				NXT_DATA_2 = NXT.INT_AV_DD.DIPDIR + NXT.INT_AV_DD_STDEV;
			}
			else if (STDEV && !DIPDIR) {

				ACT_DATA_1 = ACT.INT_AV_DD.DIP - ACT.INT_AV_D_STDEV;
				NXT_DATA_1 = NXT.INT_AV_DD.DIP - NXT.INT_AV_D_STDEV;

				ACT_DATA_2 = ACT.INT_AV_DD.DIP + ACT.INT_AV_D_STDEV;
				NXT_DATA_2 = NXT.INT_AV_DD.DIP + NXT.INT_AV_D_STDEV;
			}
			else if (D && DIPDIR) {

				ACT_DATA_1 = ACT.DD_DERIV;
				NXT_DATA_1 = NXT.DD_DERIV;
			}
			else if (D && !DIPDIR) {

				ACT_DATA_1 = ACT.D_DERIV;
				NXT_DATA_1 = NXT.D_DERIV;
			}
			else ASSERT_DEAD_END();


			const bool OTHER_SIDE_1 =	(ACT_DATA_1 < 0.0 && NXT_DATA_1 > 0.0) ||
										(ACT_DATA_1 > 0.0 && NXT_DATA_1 < 0.0) ||
										(ACT_DATA_1 < 360.0 && NXT_DATA_1 > 360.0) ||
										(ACT_DATA_1 > 360.0 && NXT_DATA_1 < 360.0);


			const bool OTHER_SIDE_2 =	(ACT_DATA_2 < 0.0 && NXT_DATA_2 > 0.0) ||
										(ACT_DATA_2 > 0.0 && NXT_DATA_2 < 0.0) ||
										(ACT_DATA_2 < 360.0 && NXT_DATA_2 > 360.0) ||
										(ACT_DATA_2 > 360.0 && NXT_DATA_2 < 360.0);




			//if (ACT_DATA_1 < 0.0) ACT_DATA_1 = 0.0;
			//if (NXT_DATA_1 < 0.0) NXT_DATA_1 = 0.0;
			//if (ACT_DATA_2 < 0.0) ACT_DATA_2 = 0.0;
			//if (NXT_DATA_2 < 0.0) NXT_DATA_2 = 0.0;
			//if (ACT_DATA_1 > 360.0) ACT_DATA_1 = 360.0;
			//if (NXT_DATA_1 > 360.0) NXT_DATA_1 = 360.0;
			//if (ACT_DATA_2 > 360.0) ACT_DATA_2 = 360.0;
			//if (NXT_DATA_2 > 360.0) NXT_DATA_2 = 360.0;

			if (ACT_DATA_1 < 0.0) ACT_DATA_1 = 360.0 + ACT_DATA_1;
			if (NXT_DATA_1 < 0.0) NXT_DATA_1 = 360.0 + NXT_DATA_1;
			if (ACT_DATA_2 < 0.0) ACT_DATA_2 = 360.0 + ACT_DATA_2;
			if (NXT_DATA_2 < 0.0) NXT_DATA_2 = 360.0 + NXT_DATA_2;

			if (ACT_DATA_1 > 360.0) ACT_DATA_1 = ACT_DATA_1 - 360.0;
			if (NXT_DATA_1 > 360.0) NXT_DATA_1 = NXT_DATA_1 - 360.0;
			if (ACT_DATA_2 > 360.0) ACT_DATA_2 = ACT_DATA_2 - 360.0;
			if (NXT_DATA_2 > 360.0) NXT_DATA_2 = NXT_DATA_2 - 360.0;



			VALUE_1.push_back (ACT_DATA_1);
			DEPTH_1.push_back (ACT.DEPTH);


			if (OTHER_SIDE_1) {

				VALUE_1.push_back (999.0);
				DEPTH_1.push_back ((ACT.DEPTH + NXT.DEPTH) / 2.0);
			}



			if (STDEV) {
				VALUE_2.push_back (ACT_DATA_2);
				DEPTH_2.push_back (ACT.DEPTH);

				if (OTHER_SIDE_2) {

					VALUE_2.push_back (999.0);
					DEPTH_2.push_back ((ACT.DEPTH + NXT.DEPTH) / 2.0);
				}
			}


//nem kezeli, ha az utolso elotti elemnel valt a fuggveny!




			if (LAST) {

				VALUE_1.push_back (ACT_DATA_1);
				DEPTH_1.push_back (NXT.DEPTH);

				if (STDEV) {
					VALUE_2.push_back (ACT_DATA_2);
					DEPTH_2.push_back (ACT.DEPTH);
				}
			}
		}
	}
	plot_curve (DEPTH_1, VALUE_1, o, P, X, LENGTH, MIN_VAL, MAX_VAL, DIPDIR, TYPE);

	if (STDEV) plot_curve_stdev (DEPTH_1, VALUE_1, DEPTH_2, VALUE_2, o, P, X, LENGTH, MIN_VAL, MAX_VAL, DIPDIR);

	return;
}
 */


void plot_well_measurements (const vector <GDB>& inGDB, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const bool DIPDIR) {

	const vector <GDB> temp = sort_by_DEPTH (inGDB);

	double MX = 90.0;
	if (DIPDIR) MX = 360.0;

	for (size_t i = 0; i < inGDB.size(); i++) {

		double 		DATA = inGDB.at(i).corr.DIP;
		if (DIPDIR) DATA = inGDB.at(i).corr.DIPDIR;

		const double data_X = PL_WDT * P.A * (DATA / MX );
		const double data_Y = LENGTH * ((inGDB.at(i).DEPTH - MIN_VAL) / (MAX_VAL - MIN_VAL));

		translate_PS (o, X + data_X, P.O1Y - data_Y, 3);

		text_PS (o, " newpath measurement");

		translate_PS (o, - X - data_X, - P.O1Y + data_Y, 3);
	}
	stroke_PS (o);

	return;
}

void PS_well_intervals_error (const vector <WELL_INTERVAL>& INTERVAL, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL, const bool DIPDIR) {

	//has bugs while generating colour code

	double  MIN_ERROR = 999.0;
	double  MAX_ERROR = 0.0;

	for (size_t i = 0; i < INTERVAL.size(); i++) {

		double 		SD = INTERVAL.at(i).INT_AV_D_STDEV;
		if (DIPDIR) SD = INTERVAL.at(i).INT_AV_DD_STDEV;

		if (SD > MAX_ERROR) MAX_ERROR = SD;
		if (SD < MIN_ERROR) MIN_ERROR = SD;
	}

	for (size_t i = 0; i < INTERVAL.size(); i++) {

		const WELL_INTERVAL I = INTERVAL.at(i);

		double 		SD = I.INT_AV_D_STDEV;
		if (DIPDIR) SD = I.INT_AV_DD_STDEV;

		const double MIN = I.MIN;
		const double MAX = I.MAX;

		const double COUNT = string_to_double (size_t_to_string (i));
		const double END = string_to_double (size_t_to_string (INTERVAL.size()));

		const double WIDTH = (PL_WDT * P.A) / END;

		const double X1 = X + PL_WDT * P.A * (COUNT / END);

		const double Y1 = P.O1Y - LENGTH * ((MAX - MIN_VAL) / (MAX_VAL - MIN_VAL));
		const double Y2 = P.O1Y - LENGTH * ((MIN - MIN_VAL) / (MAX_VAL - MIN_VAL));

		const VCTR RED = declare_vector (1.00, 0.00, 0.00);
		const VCTR GRY = declare_vector (1.00, 1.00, 0.00);

		double CLR_RATIO = (1.0 * (SD - MIN_ERROR)) / (MAX_ERROR - MIN_ERROR);
		if (INTERVAL.at(i).SIZE == 0) CLR_RATIO = 0.0;

		//cout
		//<< "CLR_RATIO: " << CLR_RATIO << "  "
		//<< "MIN_ERROR: " << MIN_ERROR << "  "
		//<< "MAX_ERROR: " << MAX_ERROR << "  "
		//<< "SD: " << SD
		//<< endl;

		VCTR RES = declare_vector (
					RED.X * CLR_RATIO + GRY.X * (1.0 - CLR_RATIO),
					RED.Y * CLR_RATIO + GRY.Y * (1.0 - CLR_RATIO),
					RED.Z * CLR_RATIO + GRY.Z * (1.0 - CLR_RATIO));

		string CLR = double_to_string (RES.X, 2) + " " + double_to_string (RES.Y, 2) + " " + double_to_string (RES.Z, 2);

		newpath_PS(o);
		linewidth_PS (o, 0.5, 1);
		color_PS (o, CLR);

		moveto_PS (o, X1, Y1, 3);
		lineto_PS (o, X1, Y2, 3);
		lineto_PS (o, X1 + WIDTH, Y2, 3);
		lineto_PS (o, X1 + WIDTH, Y1, 3);
		lineto_PS (o, X1, Y1, 3);

		fill_PS (o);

		stroke_PS (o);
	}
}

void PS_well_intervals (const vector <WELL_INTERVAL>& INTERVAL, ofstream& o, const PAPER& P, const double X, const double LENGTH, const double MIN_VAL, const double MAX_VAL) {

	const double LW = 0.5;

	linewidth_PS(o, LW, 1);

	size_t MAX_DATA = 0;
	size_t MIN_DATA = 999;

	for (size_t i = 0; i < INTERVAL.size(); i++) {

		//cout << "INTERVAL.at(i).SIZE: " << INTERVAL.at(i).SIZE << endl;

		if (INTERVAL.at(i).SIZE > MAX_DATA) MAX_DATA = INTERVAL.at(i).SIZE;
		if (INTERVAL.at(i).SIZE < MIN_DATA) MIN_DATA = INTERVAL.at(i).SIZE;
	}

	//cout << MIN_DATA << endl;
	//cout << MAX_DATA << endl;

	for (size_t i = 0; i < INTERVAL.size(); i++) {

		newpath_PS(o);

		const double MIN = INTERVAL.at(i).MIN;
		const double MAX = INTERVAL.at(i).MAX;

		const double COUNT = string_to_double (size_t_to_string (i));
		const double END = string_to_double (size_t_to_string (INTERVAL.size()));

		const double WIDTH = (PL_WDT * P.A) / END;

		const double X1 = X + PL_WDT * P.A * (COUNT / END);

		const double Y1 = P.O1Y - LENGTH * ((MAX - MIN_VAL) / (MAX_VAL - MIN_VAL));
		const double Y2 = P.O1Y - LENGTH * ((MIN - MIN_VAL) / (MAX_VAL - MIN_VAL));

		const VCTR GRN = declare_vector (0.00, 1.00, 0.00);
		const VCTR GRY = declare_vector (1.00, 1.00, 0.00);

		double CLR_RATIO = (1.0 * (INTERVAL.at(i).SIZE) - MIN_DATA) / (MAX_DATA - MIN_DATA);

		if (INTERVAL.at(i).SIZE == 0) CLR_RATIO = 0.0;

		VCTR RES = declare_vector (
					GRN.X * CLR_RATIO + GRY.X * (1.0 - CLR_RATIO),
					GRN.Y * CLR_RATIO + GRY.Y * (1.0 - CLR_RATIO),
					GRN.Z * CLR_RATIO + GRY.Z * (1.0 - CLR_RATIO));

		string CLR = double_to_string (RES.X, 2) + " " + double_to_string (RES.Y, 2) + " " + double_to_string (RES.Z, 2);

		////if (is_WELL_INTERVAL_DATANUMBER ()) CLR = "0.83 0.83 0.83";

		color_PS (o, CLR);

		moveto_PS (o, X1, Y1, 3);
		lineto_PS (o, X1, Y2, 3);
		lineto_PS (o, X1 + WIDTH, Y2, 3);
		lineto_PS (o, X1 + WIDTH, Y1, 3);
		lineto_PS (o, X1, Y1, 3);

		fill_PS (o);

		stroke_PS (o);
	}
}

void OUTPUT_TO_WELL_PS (const vector <vector <vector <vector <GDB> > > > GDB_G, const PFN PF, const bool TILT) {

	if (is_WELLDATA_NO()) return;

	const vector < vector <GDB> > prGDB_G = MERGE_GROUPS_TO_GDB_G (GDB_G);

	////vector <vector <vector <vector <WELL_INTERVAL> > > > I = RETURN_INTERVAL ();

	////const vector <vector <WELL_INTERVAL> > INTERVAL = MERGE_INTERVAL (I);

	////vector <vector <vector <vector <WELL_FREQUENCY> > > > F = RETURN_FREQUENCY ();

	////const vector <vector <WELL_FREQUENCY> > FREQUENCY = MERGE_FREQUENCY (F);

	const bool IGNORE = is_GROUPSEPARATION_IGNORE ();
	const bool by_GROUPCODE = is_GROUPSEPARATION_GROUPCODE ();
	const bool by_KMEANS = is_GROUPSEPARATION_KMEANS ();
	const bool by_RUPANG = is_GROUPSEPARATION_RUPANG ();

	if (!IGNORE && !by_GROUPCODE && !by_KMEANS && !by_RUPANG) ASSERT_DEAD_END() ;

	const string BS = path_separator;
	const string US = "_";

	for (size_t i = 0; i < prGDB_G.size(); i++) {

		const vector <GDB> temp = sort_by_DEPTH (prGDB_G.at(i));

		const string LOC = temp.at(0).LOC;
		const string DT = temp.at(0).DATATYPE;

		const bool PLN = is_allowed_plane_datatype (DT);
		const bool LIN = is_allowed_lineation_datatype (DT);

		const bool PROCESS_AS_WELL = PLN || LIN;

		if (PROCESS_AS_WELL) {

			//cout << "PROCESS: " << temp.size() << endl;

			string PS_NAME = PF.well_ps + BS + DT + BS + LOC + US + DT;

			if (by_GROUPCODE) 	PS_NAME = PS_NAME + US + temp.at(0).GC;
			else if (by_KMEANS) PS_NAME = PS_NAME + US + temp.at(1).GC;
			else if (by_RUPANG) PS_NAME = PS_NAME + US + temp.at(2).GC;
			else {}

			PS_NAME = PS_NAME + ".eps";

			ofstream OPS (PS_NAME.c_str());

			const double MIN = temp.at(0).DEPTH;
			const double MAX = temp.at(temp.size() - 1).DEPTH;

			const double STEP = well_axes_step (MIN, MAX);

			const double MIN_VAL = return_MIN_value (temp, STEP);
			const double MAX_VAL = return_MAX_value (temp, STEP);

			const PAPER P = PS_dimensions (true);

			PS_well_header (DT, LOC, OPS);

			PS_well_border (temp, OPS, P, TILT);

			const double LENGTH = P.Y * 0.8;

			//cout << "1A" << endl;
			double X = P.O1X + PL_LF * P.A;
			//PS_well_coordinate_grid_INTERVAL (OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, STEP);
		////	PS_well_intervals (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL);
			//PS_well_coordinate_axes_INTERVAL (OPS, P, X, LENGTH);


			//cout << "2A" << endl;
			X = X + (PL_WDT + PL_GP) * P.A;
			PS_well_coordinate_grid_DIPDIR (OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, STEP);
			//PS_well_intervals_error (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true);
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true, "LOWER_STDEV");
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true, "UPPER_STDEV");
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true, "AVERAGE");
			plot_well_measurements (temp, OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true);
			PS_well_coordinate_axes_DIPDIR (OPS, P, X, LENGTH);


			//cout << "2B" << endl;
			X = X + (PL_WDT + PL_AX_GP) * P.A;
			PS_derivate_DIPDIR_DIP (OPS, P, X);
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, true, "DERIVATE");


			//3A
			X = X + (PL_WDT * 0.5 + PL_GP) * P.A;
			PS_well_coordinate_grid_DIP (OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, STEP);
			//PS_well_intervals_error (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false);
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "LOWER_STDEV");
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "UPPER_STDEV");
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "AVERAGE");
			//plot_well_measurements (temp, OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false);
			PS_well_coordinate_axes_DIP (OPS, P, X, LENGTH);

			//3B
			X = X + (PL_WDT + PL_AX_GP) * P.A;
			PS_derivate_DIPDIR_DIP (OPS, P, X);
			//plot_well_curve (INTERVAL.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, false, "DERIVATE");

			//4A
			X = X + (PL_WDT * 0.5 + PL_GP) * P.A;
			PS_well_coordinate_axes_FREQUENCY (OPS, P, X, LENGTH, MIN_VAL, MAX_VAL, STEP);
		////	plot_well_frequency (FREQUENCY.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL);


			//4B
			X = X + (PL_WDT + PL_AX_GP) * P.A;
			PS_derivate_DIPDIR_DIP(OPS, P, X);
			////plot_well_frequency_derivate (FREQUENCY.at(i), OPS, P, X, LENGTH, MIN_VAL, MAX_VAL);

			//PS_well_coordinate_axes (prGDB_G.at(i), OPS, PPR);
		}
	}

	return;
}