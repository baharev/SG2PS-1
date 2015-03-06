
// Copyright (C) 2012 - 20143 Ágoston Sasvári
// All rights reserved.
// This code is published under the GNU Lesser General Public License.

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "common.h"
#include "inversion.h"
#include "fry.h"
#include "rgf.h"
#include "stresstensor.hpp"

using namespace std;

namespace {

size_t second_eigenvalue = 0;
size_t first_eigenvalue = 0;

vector <vector <double> > A = init_matrix (6);
vector <vector <double> > D = init_matrix (6);

vector <vector <double> > EVEV;

}

bool check_fry_matrix (const size_t first_eigenvalue, const vector <vector <double> >& in_eigenvector) {

	vector <sort_jacobi> buffer;
	sort_jacobi buf;

	for (size_t i = 0; i < in_eigenvector.size(); i++) {

		buf.ID = i;
		buf.eigenvalue = in_eigenvector.at(first_eigenvalue).at(i);
		buffer.push_back(buf);
	}

	sort(buffer.begin(), buffer.end(), byeigenvalue);

	if (
		is_in_range( -0.005, 0.005, buffer.at(0).eigenvalue) &&
		is_in_range( -0.005, 0.005, buffer.at(1).eigenvalue) &&
		is_in_range( -0.005, 0.005, buffer.at(2).eigenvalue) &&
		is_in_range( -0.570, 0.580, buffer.at(3).eigenvalue) &&
		is_in_range( -0.570, 0.580, buffer.at(4).eigenvalue) &&
		is_in_range( -0.570, 0.580, buffer.at(5).eigenvalue)
	) return true;
	return false;
}

bool fry_correct (const vector <GDB>& inGDB) {

	EVEV = FRY_matrix (inGDB);

	A = generate_A (EVEV);
	D = generate_D (EVEV);

	first_eigenvalue = return_first_eigenvalue (A);
	second_eigenvalue = return_second_eigenvalue (A);

	if (check_fry_matrix (first_eigenvalue, D)) return true;
	return false;
}

vector <double> hyperplane_from_GDB (const GDB& inGDB)  {

	vector <double>  out = init_vector (6);
	CENTR_VECT o;

	const VCTR n = inGDB.N;
	const VCTR b = inGDB.SV;

	VCTR t = unitvector (crossproduct (b, n));

	o.U = - (n.X * t.X);
	o.V = - (n.Y * t.Y);
	o.W = - (n.Z * t.Z);

	o.X = - ((t.X * n.Y) + (t.Y * n.X));
	o.Y = - ((t.Y * n.Z) + (t.Z * n.Y));
	o.Z = - ((t.Z * n.X) + (t.X * n.Z));

	o = unitvector (o);

	out.at(0) = o.U;
	out.at(1) = o.V;
	out.at(2) = o.W;
	out.at(3) = o.X;
	out.at(4) = o.Y;
	out.at(5) = o.Z;

	return out;
}

vector <vector <double> > FRY_matrix (const vector <GDB>& inGDB) {

	vector <GDB> processGDB = inGDB;

	vector <vector <double> > TNSR6 = init_matrix (6);

	//if (inset.virt_striae == "Y" ) processGDB = generate_virtual_striae (processGDB);

	for (size_t i = 0; i < processGDB.size(); i++) {

		const vector <double> hyperplane = hyperplane_from_GDB (processGDB.at(i));

		vector <vector <double> > temp  = init_matrix (6);
		temp = outer_product (hyperplane);

		TNSR6 = add_mtrx (TNSR6, temp);
	}
	return jacobi (TNSR6);
}

STRESSTENSOR st_FRY (const vector <GDB>& inGDB) {

	STRESSTENSOR st;

	st._11 = D.at(second_eigenvalue).at(0);
	st._22 = D.at(second_eigenvalue).at(1);
	st._33 = D.at(second_eigenvalue).at(2);
	st._12 = D.at(second_eigenvalue).at(3);
	st._23 = D.at(second_eigenvalue).at(4);
	st._13 = D.at(second_eigenvalue).at(5);

	const double misfit1 = return_average_misfit (st, inGDB);

	st = invert_stress_tensor (st);

	const double misfit2 = return_average_misfit (st, inGDB);

	if (misfit1 < misfit2) return invert_stress_tensor (st);
	return st;
}

STRESSFIELD sf_FRY (const STRESSTENSOR& st) {

	STRESSFIELD sf = eigenvalue_eigenvector (st);

	sf = computestressfield_DXDYDZ (sf);

	return stress_regime (sf);
}
