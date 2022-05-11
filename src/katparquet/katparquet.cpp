#include "../include/oasis.h"
#ifdef HAVE_PARQUET
#include "../include/oasisparquet.h"
#endif

#include <cstring>
#include <map>
#include <vector>


namespace katparquet {

  struct period_event {
    int period;
    int eventID;
  };

  bool operator<(const period_event &lhs, const period_event &rhs) {
    if (lhs.period != rhs.period) {
      return lhs.period < rhs.period;
    } else {
      return lhs.eventID < rhs.eventID;
    }
  }

  bool operator==(const period_event &lhs, const period_event &rhs) {
    return lhs.period == rhs.period && lhs.eventID == rhs.eventID;
  }

  template<typename tableNameT>
  void GetRow(std::map<int, int> &event_to_file, const tableNameT &row,
	      const int i) {
    event_to_file[row.eventID] = i;
  }

  template<typename tableNameT>
  void GetRow(std::map<period_event, int> &period_to_file,
	      const tableNameT &row, const int i) {
    period_event periodEventID;
    periodEventID.period = row.period;
    periodEventID.eventID = row.eventID;
    period_to_file[periodEventID] = i;
  }

  template<typename tableNameT, typename rowMapT>
  void ReadFirstRows(const std::vector<std::string> &inFiles,
		     std::vector<parquet::StreamReader> &isIn,
		     std::vector<tableNameT> &rows, rowMapT &rowMap_to_file) {

    // Read first row from each file
    for (int i = 0; i < (int)inFiles.size(); i++) {
      isIn[i] = OasisParquet::SetupParquetInputStream(inFiles[i]);
      tableNameT row;
      if (isIn[i].eof()) continue;   // Ignore empty files
      isIn[i] >> row;
      rows[i] = row;

      GetRow(rowMap_to_file, rows[i], i);
    }

  }

  template<typename tableNameT>
  void WriteOutput(std::vector<tableNameT> &rows,
		   std::map<int, int> &event_to_file,
		   parquet::StreamWriter &osOut,
		   std::vector<parquet::StreamReader> &isIn) {

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
	GetRow(event_to_file, rows[iter->second], iter->second);
      }

      event_to_file.erase(iter);

    }

  }

  template<typename tableNameT>
  void WriteOutput(std::vector<tableNameT> &rows,
		   std::map<period_event, int> &period_to_file,
		   parquet::StreamWriter &osOut,
		   std::vector<parquet::StreamReader> &isIn) {

    // First entry should always have lowest period number followed by event ID
    while (period_to_file.size() != 0) {

      auto iter = period_to_file.begin();
      period_event currentPeriodEvent = iter->first;
      period_event periodEventID;
      periodEventID.period = rows[iter->second].period;
      periodEventID.eventID = rows[iter->second].eventID;

      while (periodEventID == currentPeriodEvent) {
	osOut << rows[iter->second];
	if (isIn[iter->second].eof()) break;
	isIn[iter->second] >> rows[iter->second];
	periodEventID.period = rows[iter->second].period;
	periodEventID.eventID = rows[iter->second].eventID;
      }

      if (!isIn[iter->second].eof()) {
	GetRow(period_to_file, rows[iter->second], iter->second);
      }

      period_to_file.erase(iter);

    }

  }

  template<typename tableNameT, typename rowMapT>
  void DoKat(const std::vector<std::string> &inFiles,
	     parquet::StreamWriter &osOut, std::vector<tableNameT> &rows,
	     rowMapT &rowMap_to_file) {

    rows.resize(inFiles.size());
    std::vector<parquet::StreamReader> isIn;
    isIn.resize(inFiles.size());

    ReadFirstRows(inFiles, isIn, rows, rowMap_to_file);

    if (rowMap_to_file.size() == 0) {
      fprintf(stderr, "ERROR: All input files are empty\n");
      exit(EXIT_FAILURE);
    }

    WriteOutput(rows, rowMap_to_file, osOut, isIn);

  }


  void doit(const std::vector<std::string> &inFiles,
	    const std::string outFile, const int tableName) {

    parquet::StreamWriter osOut = OasisParquet::GetParquetStreamWriter(tableName, outFile);
    std::map<int, int> event_to_file;   // Exceedance Loss Table
    std::map<period_event, int> period_to_file;   // Period Loss Table

    if (tableName == OasisParquet::MPLT) {
      std::vector<OasisParquet::MomentPLTEntry> rows;
      DoKat(inFiles, osOut, rows, period_to_file);
    } else if (tableName == OasisParquet::QPLT) {
      std::vector<OasisParquet::QuantilePLTEntry> rows;
      DoKat(inFiles, osOut, rows, period_to_file);
    } else if (tableName == OasisParquet::SPLT) {
      std::vector<OasisParquet::SamplePLTEntry> rows;
      DoKat(inFiles, osOut, rows, period_to_file);
    } else if (tableName == OasisParquet::MELT) {
      std::vector<OasisParquet::MomentELTEntry> rows;
      DoKat(inFiles, osOut, rows, event_to_file);
    } else if (tableName == OasisParquet::QELT) {
      std::vector<OasisParquet::QuantileELTEntry> rows;
      DoKat(inFiles, osOut, rows, event_to_file);
    } else if (tableName == OasisParquet::SELT) {
      std::vector<OasisParquet::SampleELTEntry> rows;
      DoKat(inFiles, osOut, rows, event_to_file);
    } else if (tableName == OasisParquet::NONE) {
      fprintf(stderr, "FATAL: No table type selected - please select table "
		      "type for files to be concatenated.\n");
      exit(EXIT_FAILURE);
    } else {   // Should never get here
      fprintf(stderr, "FATAL: Unknown table selected.\n");
      exit(EXIT_FAILURE);
    }

  }

}
