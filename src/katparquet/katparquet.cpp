#include "../include/oasis.h"
#ifdef HAVE_PARQUET
#include "../include/oasisparquet.h"
#endif

#include <cstring>
#include <map>
#include <vector>


namespace katparquet {

  template<typename tableNameT>
  void DoKat(const std::vector<std::string> &inFiles,
	     parquet::StreamWriter &osOut, std::vector<tableNameT> &rows) {

    rows.resize(inFiles.size());
    std::vector<parquet::StreamReader> isIn;
    isIn.resize(inFiles.size());
    std::map<int, int> event_to_file;

    // Read first row from each file
    for (int i = 0; i < (int)inFiles.size(); i++) {
      isIn[i] = OasisParquet::SetupParquetInputStream(inFiles[i]);
      tableNameT row;
      if (isIn[i].eof()) continue;   // Ignore empty files
      isIn[i] >> row;
      rows[i] = row;

      event_to_file[rows[i].eventID] = i;
    }

    if (event_to_file.size() == 0) {
      fprintf(stderr, "ERROR: All input files are empty\n");
      exit(EXIT_FAILURE);
    }

    // First entry should always have lowest event ID
    while(event_to_file.size() != 0) {

      auto iter = event_to_file.begin();
      int currentEventID = iter->first;

      while (rows[iter->second].eventID == currentEventID) {
	osOut << rows[iter->second];
	if (isIn[iter->second].eof()) break;
	isIn[iter->second] >> rows[iter->second];
      }

      if (!isIn[iter->second].eof()) {
	event_to_file[rows[iter->second].eventID] = iter->second;
      }

      event_to_file.erase(iter);

    }

  }


  void doit(const std::vector<std::string> &inFiles,
	    const std::string outFile, const int tableName) {

    enum { NONE = 0, MPLT, QPLT, SPLT, MELT, QELT, SELT };

    parquet::StreamWriter osOut = OasisParquet::GetParquetStreamWriter_(tableName, outFile);

    if (tableName == MPLT) {
      std::vector<OasisParquet::MomentPLTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == QPLT) {
      std::vector<OasisParquet::QuantilePLTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == SPLT) {
      std::vector<OasisParquet::SamplePLTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == MELT) {
      std::vector<OasisParquet::MomentELTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == QELT) {
      std::vector<OasisParquet::QuantileELTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == SELT) {
      std::vector<OasisParquet::SampleELTEntry> rows;
      DoKat(inFiles, osOut, rows);
    } else if (tableName == NONE) {
      fprintf(stderr, "FATAL: No table type selected - please select table "
		      "type for files to be concatenated.\n");
      exit(EXIT_FAILURE);
    } else {   // Should never get here
      fprintf(stderr, "FATAL: Unknown table selected.\n");
      exit(EXIT_FAILURE);
    }

  }

}
