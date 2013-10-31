// Copyright (C) 2012, 2013 Ágoston Sasvári
// All rights reserved.
// This code is published under the GNU Lesser General Public License.

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "allowed_keys.hpp"
#include "array_to_vector.hpp"
#include "color_management.hpp"
#include "data_io.h"
#include "density.h"
#include "foldsurface.hpp"
#include "kaalsbeek.hpp"
#include "platform_dep.hpp"
#include "ps.h"
#include "rgf.h"
#include "rose.h"
#include "run_mode.h"

PFN createprojectfoldernames (string projectname) {

	time_t current_time;
	struct tm * TM;
	const string bs = path_separator;

	time ( &current_time );
	TM = localtime ( &current_time );

	PFN output;

	output.datetime = int_to_string (TM->tm_year + 1900);

	if (TM->tm_mon < 9) output.datetime = output.datetime + "0";
	output.datetime = output.datetime + int_to_string (TM->tm_mon + 1);

	if (TM->tm_mday < 10) output.datetime = output.datetime + "0";
	output.datetime = output.datetime + int_to_string (TM->tm_mday) + "-";

	if (TM->tm_hour < 10) output.datetime = output.datetime + "0";
	output.datetime = output.datetime + int_to_string (TM->tm_hour);

	if (TM->tm_min < 10) output.datetime = output.datetime + "0";
	output.datetime = output.datetime + int_to_string (TM->tm_min);

	if (TM->tm_sec < 10) output.datetime = output.datetime + "0";
	output.datetime = output.datetime + int_to_string (TM->tm_sec);

	if (!is_DEBUG()) output.projectfolder 	= output.datetime + "_-_" + capslock(projectname);
	else output.projectfolder = capslock(projectname);

	output.projectname 		= projectname; // Converting to uppercase is a bug on Linux
	output.original			= output.projectfolder +  bs + "1_original";
	output.completed		= output.projectfolder +  bs + "2_completed";
	output.average			= output.projectfolder +  bs + "3_average";
	output.rgfsep			= output.projectfolder +  bs + "4_rgf_separated";
	output.pssep			= output.projectfolder +  bs + "5_ps_separated";

	return output;
}

vector <string> possible_folders_name () {

	vector <string> OUT = allowed_lineation_datatype_vector();

	OUT = merge_datatypes(OUT, allowed_plane_datatype_vector());
	OUT = merge_datatypes(OUT, allowed_striae_datatype_vector());
	OUT = merge_datatypes(OUT, allowed_SC_datatype_vector());

	return OUT;
}

void make_dir(const string& dir_name) {

	string command = "mkdir "+dir_name;

	int ret = system(command.c_str());

	if (ret) {

		throw runtime_error("failed to execute command \""+command+"\"");
	}
}

void create_folders(const PFN& output, const string& dir) {

	make_dir(output.rgfsep + path_separator + dir);
	make_dir(output.pssep  + path_separator + dir);
}

void createprojectfolders (PFN output, vector <GDB> inGDB) {

	make_dir( output.projectfolder );
	make_dir( output.original      );
	make_dir( output.completed     );
	make_dir( output.average       );
	make_dir( output.rgfsep        );
	make_dir( output.pssep         );

	vector <string> possible_folders = possible_folders_name ();

	for (size_t i = 0; i < possible_folders.size(); ++i) {

		const string& dir = possible_folders.at(i);

		if (existence(dir, inGDB)) {

			create_folders(output, dir);
		}
	}
}

namespace {

struct copy_status { enum code { OK, OPENING_INPUT_FAILED, FAILED }; };

copy_status::code copy_file(const string& source, const string& destination) {

	ifstream in(source.c_str(), ios::binary);

	if (!in.is_open()) {

		return copy_status::OPENING_INPUT_FAILED;
	}

	ofstream out(destination.c_str(), ios::binary);

	out << in.rdbuf();

	return (in.fail() || out.fail()) ? copy_status::FAILED : copy_status::OK;
}

void dispatch_on_error_code(copy_status::code status, const string& src_name, bool should_exist);

void back_up_file(string extension, const string& project_name, const string& new_path, bool should_exist = false) {

	string src  = project_name+extension;

	dispatch_on_error_code(copy_file(src, new_path+src), src, should_exist);
}

void dispatch_on_error_code(copy_status::code status, const string& src_name, bool should_exist) {

	const string EXTENSION = to_uppercase(src_name.substr(src_name.size()-3));

	if      (status == copy_status::OK) {

		cout << "  - Original input "+EXTENSION+" file has been successfully copied to the project folder." << endl;
	}
	else if (status == copy_status::OPENING_INPUT_FAILED && should_exist) {

		throw runtime_error("opening "+src_name+" failed");
	}
	else if (status == copy_status::FAILED) {

		cout << "  - Critical error: failed to copy input "+EXTENSION+" file to the project folder." << endl;

		throw runtime_error("copying "+src_name+" failed");
	}
}

}

void copyoriginalfiles (PFN output) {

	const string project_name = output.projectname;

	const string new_path = output.original + path_separator;

	const bool SHOULD_EXIST = true;

	back_up_file(".rgf", project_name, new_path, SHOULD_EXIST);

	back_up_file(".set", project_name, new_path); // TODO In batch and debug mode, failures were ignored
	                                              // TODO In batch mode, shouldn't it dump a set file?
	back_up_file(".xy", project_name, new_path);
}

void copy_log(const PFN& names) {

	if (is_GUI()) {

		cout.flush();

		string log_file = "log.txt";

		copy_file(log_file, names.original + path_separator + log_file);
	}
}

void create_pointer_to_project_folder(const PFN& names) {

	if (is_GUI()) {

		ofstream pointer_to("project_folder_name");

		pointer_to << names.projectfolder << '\n';
	}
}

void outputrgfheader (ofstream& o) { // TODO Duplication? Same as reserved_column_names?

	o
	<< "DATA_ID" << '\t'
	<< "GROUP_CODE" << '\t'
	<< "COLOR_CODE" << '\t'
	<< "LOCATION" << '\t'
	<< "LOC_X" << '\t'
	<< "LOC_Y" << '\t'
	<< "FORMATION" << '\t'
	<< "DATATYPE" << '\t'
	<< "DIP_DIR" << '\t'
	<< "DIP" << '\t'
	<< "L_DIP_DIR" << '\t'
	<< "L_DIP" << '\t'
	<< "SENSE" << '\t'
	<< "PALEONORTH"	<< '\t'
	<< "COMMENT" << endl;
}

void outputaverageheader (ofstream& o) { // TODO What is the extension? .rgf? How about .csv?

	o
	<< "DATA_ID" << '\t'
	<< "GC" << '\t'
	<< "COLOR" << '\t'
	<< "LOC" << '\t'
	<< "LOCX" << '\t'
	<< "LOCY" << '\t'
	<< "FORMATION" << '\t'
	<< "DATATYPE" << '\t'
	<< "averageDIPDIR" << '\t'
	<< "averageDIP" << '\t'
	<< "NODATA" << '\t'
	<< "NODATA" << '\t'
	<< "NODATA" << '\t'
	<< "NODATA" << '\t'
	<< "COMMENT" << endl;
}

void outputrecord (GDB i, ofstream& o, INPSET inpset) {

	o
	<< i.ID << '\t'
	<< i.GC << '\t'
	<< i.COLOR << '\t'
	<< i.LOC << '\t'
	<< fixed << setprecision (6) << i.LOCX << '\t'
	<< fixed << setprecision (6) << i.LOCY << '\t'
	<< i.FORMATION << '\t'
	<< i.DATATYPE << '\t';

	o << fixed << setprecision (1);

	if ((i.corr.DIPDIR > 361.0) || (i.DATATYPE == "LITHOLOGY")) o << "" << '\t';
	else {

		if (inpset.datarule == "R" ) 	o << german_to_right_hand_rule (i.corr.DIPDIR) << '\t';
		else							o << i.corr.DIPDIR << '\t';
	}

	if ((i.corr.DIP > 360.0) || (i.DATATYPE == "LITHOLOGY")) o << "" << '\t';
	else o << fixed << setprecision (1) << i.corr.DIP << '\t';


	if (i.corrL.DIPDIR > 360.0) o << "" << '\t';
	else o << i.corrL.DIPDIR << '\t';

	if (i.corrL.DIP > 360.0) o << "" << '\t';
	else o << i.corrL.DIP << '\t';

	bool is_STRIAE = 	is_allowed_striae_datatype(i.DATATYPE);
	bool is_BEDDING = 	is_allowed_foldsurface_processing(i.DATATYPE);

	if (is_STRIAE || is_BEDDING) {

		if (is_BEDDING && (i.corrOFFSET == "NONE")) o << '\t';
		else o << i.corrOFFSET << '\t';
	}

	else o << "" << '\t';

	o
	<< i.PALEON	<< '\t'
	<< i.COMMENT;
}

void outputveragerecord (GDB i, ofstream& o) {

	o
	<< i.ID << '\t'
	<< i.GC << '\t'
	<< i.COLOR << '\t'
	<< i.LOC << '\t'
	<< setprecision (6) << i.LOCX << '\t'
	<< setprecision (6) << i.LOCY << '\t'
	<< i.FORMATION << '\t'
	<< i.DATATYPE << '\t';

	if (i.avd.DIPDIR > 361.0) o << "" << '\t';
	else o << i.avd.DIPDIR << '\t';

	if (i.avd.DIP > 361.0) o << "" << '\t';
	else o << i.avd.DIP << '\t';

	o << "" << '\t';
	o << "" << '\t';
	o << "" << '\t';
	o << "" << '\t';
	o << i.COMMENT<< '\t';
}

void outputresultrgf (PFN output, vector <GDB> outGDB, bool tilted, INPSET inset) {

	ofstream outputfile;
	string outputfilename;
	string bs = path_separator;

	size_t i = 0;

	outputfilename = output.completed + bs + capslock(output.projectname) + "_completed";
	if (tilted) outputfilename = outputfilename + "_tilted";
	outputfilename = outputfilename + ".rgf";

	outputfile.open (outputfilename.c_str());

	outputrgfheader (outputfile);

	while (i < outGDB.size()) {

		outputrecord (outGDB.at(i), outputfile, inset);

		if (i < outGDB.size() - 1) outputfile << endl;

		i++;
	}

	outputfile.close();

	if (tilted) 	cout << "  - Tilted RGF file exported." <<  endl;
	else 			cout << "  - Completed RGF file exported." <<  endl;
}

void outputaveragergf (PFN output, vector <GDB> outGDB) {

	ofstream outputfile;
	string bs = path_separator;
	string outputfilename = output.average + bs + capslock(output.projectname) + "_average.rgf";

	size_t i = 0;
	size_t independentrecordcounter = 0;

	outputfile.open (outputfilename.c_str());

	outputaverageheader (outputfile);

	if ((outGDB.size() == 1) && (!((outGDB.at(0).DATATYPE == "STRIAE") || (outGDB.at(0).DATATYPE == "SC")))) {

		outputveragerecord (outGDB.at(0), outputfile);
		outputfile.close();
		cout << "  - Average RGF output completed." <<  endl;
	}

	do {

		do {

			i++;
			if (i == outGDB.size()) break;
		}

		while ((outGDB.at(i-1).DATATYPE == outGDB.at(i).DATATYPE) && (outGDB.at(i-1).LOC == outGDB.at(i).LOC));

		independentrecordcounter++;

		if (!((outGDB.at(i-1).DATATYPE == "STRIAE") || (outGDB.at(i-1).DATATYPE == "SC"))) {

			outputveragerecord (outGDB.at(i - 1), outputfile);
		}

		if (i < outGDB.size()) 	outputfile << endl;
	}

	while (i < outGDB.size());

	outputfile.close();

	cout << "  - Average RGF output completed." <<  endl;
}

void outputselected_ps_rgf (PFN output, vector <GDB> outGDB, vector <GDB> tiltoutGDB, INPSET inset) {

	vector <GDB> processGDB, tiltprocessGDB;
	size_t i = 0;
	size_t independentrecordcounter = 0;
	CENTER center;
	PAPER P = PS_dimensions ();

	center.X = P.O1X;
	center.Y = P.O1Y;
	center.radius = P.R;

	do {

		processGDB.clear();
		tiltprocessGDB.clear();

		do {

			processGDB.push_back(outGDB.at(i));
			tiltprocessGDB.push_back(tiltoutGDB.at(i));

			i++;

			if (i == outGDB.size()) break;

		} while (stopcriteria (outGDB.at(i-1).DATATYPE, outGDB.at(i).DATATYPE, outGDB.at(i-1).LOC, outGDB.at(i).LOC, outGDB.at(i-1).GC, outGDB.at(i).GC, inset));

		independentrecordcounter++;

		if (! is_allowed_lithology_datatype(processGDB.at(0).DATATYPE)) {

			output_to_rgf (output, processGDB, inset, false);

			output_to_rgf (output, tiltprocessGDB, inset, true);

			output_to_ps (output, processGDB, tiltprocessGDB, inset, P, center);
		}

	} while (i < outGDB.size());

	cout << "DATA EXPORT FROM '" << capslock(output.projectname) << ".RGF' DATABASE FILE" << endl;
	cout << "  - Postscript output completed for " << independentrecordcounter << " file containing " << i << " records." <<  endl;
	cout << "  - RGF output completed for " << independentrecordcounter << " file containing " << i << " records." <<  endl;
	cout << "  - Tilted RGF output completed for " << independentrecordcounter << " file containing " << i << " records." <<  endl;
}

void output_to_rgf (PFN output, vector <GDB> processGDB, INPSET inset, bool tilted) {

	ofstream output_rgf_file;
	string output_rgf_filename;
	string bs = path_separator;
	size_t j = 0;

	if (inset.group == "Y") output_rgf_filename = output.rgfsep + bs + processGDB.at(0).DATATYPE + bs + processGDB.at(0).LOC + "_" + processGDB.at(0).DATATYPE + "_" + processGDB.at(0).GC;
	else 					output_rgf_filename = output.rgfsep + bs + processGDB.at(0).DATATYPE + bs + processGDB.at(0).LOC + "_" + processGDB.at(0).DATATYPE;

	if (tilted) output_rgf_filename = output_rgf_filename + "_tilted";
	else output_rgf_filename = output_rgf_filename + "_original";

	output_rgf_filename = output_rgf_filename + ".rgf";

	output_rgf_file.open (output_rgf_filename.c_str());

	sort(processGDB.begin(), processGDB.end(), byiID);

	outputrgfheader (output_rgf_file);

	do {

		outputrecord (processGDB.at(j), output_rgf_file, inset);
		if (j < processGDB.size()-1) output_rgf_file << endl;
		j++;

	} while (j < processGDB.size());

	output_rgf_file.close();
}

void output_to_ps (PFN output, vector <GDB> processGDB, vector <GDB> tiltprocessGDB, INPSET inset, PAPER P, CENTER center) {

	string output_ps_filename;
	string bs = path_separator;

	if (inset.group == "Y") output_ps_filename = output.pssep +  bs + processGDB.at(0).DATATYPE + bs + processGDB.at(0).LOC + "_" + processGDB.at(0).DATATYPE + "_" + processGDB.at(0).GC + ".eps";
	else 					output_ps_filename = output.pssep +  bs + processGDB.at(0).DATATYPE + bs + processGDB.at(0).LOC + "_" + processGDB.at(0).DATATYPE + ".eps";

	ofstream output_ps_file(output_ps_filename.c_str());

	PS_header (processGDB.at(0).DATATYPE, inset, processGDB.at(0).LOC, output_ps_file);
	PS_SYMBOLS(processGDB, output_ps_file, inset, P);

	if (processGDB.at(0).DATATYPE == "STRIAE" && (inset.inversion != "N")) PS_stress_scale (output_ps_file, inset, P);


	PS_border (processGDB.at(0), output_ps_file, inset, P);


	process_group_by_group (processGDB, output_ps_file, inset, center, P, false);
	process_group_by_group (tiltprocessGDB, output_ps_file, inset, center, P, true);

	PS_datanumber_averagebedding (processGDB.at(0), output_ps_file, inset, P, center, processGDB.size());

	PS_net (output_ps_file, inset, P);

	//contouring (processGDB, inset);
}

void cout_method_text (vector <GDB> inGDB, INPSET inset) {

	cout
	<< "  - For '" << inGDB.at(0).LOC << "' location" << flush;

	if (inset.group == "Y")	cout << ", '"<< inGDB.at(0).GC << "', " << flush;
	else 					cout << "," << flush;

	if 		(inGDB.at(0).DATATYPE == "FRACTURE" && inset.fracture == "B")		cout << "fracture statistics after Bingham (1964): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "F")	cout << "regression after Fry (1999): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "M")	cout << "regression after Michael (1984): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "S")	cout << "regression after Shan et al. (2003): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "A")	cout << "inversion after Angelier (1990): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "O") 	cout << "inversion after Mostafa (2005): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "D") 	cout << "regression after Sprang (1972): " << flush;
	else if (inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion == "P") 	cout << " regression after Turner (1953): " << flush;
	else if (inGDB.at(0).DATATYPE == "FOLDSURFACE")cout << " fold axis calculation: " << flush;
	else {};
}

void cout_original_tilted_text (bool tilt) {

	if (!tilt) 	{

		cout << endl;
		cout << "    - Original : " << flush;
	}
	else {

		cout << "    - Corrected: " << flush;
	}
}

void process_group_by_group (vector <GDB> inGDB, ofstream& o, INPSET inset, CENTER center, PAPER P, bool tilt) {

	CENTER mohr_center;

	if (!tilt) {

		center.X = P.O1X;
		center.Y = P.O1Y;
		mohr_center.X = P.O7X;
		mohr_center.Y = P.O7Y;
	}
	else {

		center.X = P.O2X;
		center.Y = P.O2Y;
		mohr_center.X = P.O8X;
		mohr_center.Y = P.O8Y;
	}


	bool IS_FRACTURE = 	(inset.fracture == "B" && inGDB.at(0).DATATYPE == "FRACTURE");
	bool IS_STRIAE = 	(inGDB.at(0).DATATYPE == "STRIAE" && inset.inversion != "N");
	bool IS_FOLD = 		(is_allowed_foldsurface_processing(inGDB.at(0).DATATYPE));

	bool TO_INVERT = (IS_FRACTURE || IS_STRIAE);  // was followed by "|| IS_FOLD"


	vector <GDB> processGDB = inGDB;;

	if (inGDB.at(0).DATATYPE == "STRIAE") processGDB = return_striae_with_offset (inGDB);
	else {}

	bool has_right_data_number = correct_inhomogeneous_number (processGDB, inset);

	PS_draw_rose (inGDB, o, inset, center, P, tilt);


	if (IS_FOLD) calculate_foldsurface (inGDB, o, inset, center);
	else {}


	if (TO_INVERT) {

		if (!tilt) cout_method_text (inGDB, inset);

		if (has_right_data_number) {

			cout_original_tilted_text (tilt);

			processGDB = inversion (processGDB, o, inset, center, mohr_center, P);

			process_one_by_one (processGDB, o, inset, center, P, tilt);
		}
		else {

			process_one_by_one (processGDB, o, inset, center, P, tilt);

			if (!tilt) cout << "less (independent) data than required." << endl;
			else {} //ok
		}
	}
	else {

		process_one_by_one (processGDB, o, inset, center, P, tilt);
	}
}

void process_one_by_one (vector <GDB> inGDB, ofstream& o, INPSET inset, CENTER center, PAPER P, bool tilt) {

	if (!tilt) {

		center.X = P.O1X;
		center.Y = P.O1Y;
	}
	else {

		center.X = P.O2X;
		center.Y = P.O2Y;
	}

	inGDB = colorcode_grom_groupcode(inGDB, inset);

	for (size_t i = 0; i < inGDB.size(); i++) {

		PS_DRAW_record (inGDB.at(i), o, inset, center);
	}
}

void dbg_cout_RGF_colors (vector <GDB> inGDB) {

	cout << "RGF colors" << endl;

	for (size_t i = 0; i < inGDB.size(); i++) {

		cout
		<< inGDB.at(i).ID << '\t'
		<< inGDB.at(i).GC << '\t'
		<< inGDB.at(i).PSCOLOR << '\t'
		<< inGDB.at(i).COLOR << '\t'
		<< endl;
	}
}
