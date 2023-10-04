#if defined(_MSC_VER)
#include "../include/dirent.h"
#else
#include <dirent.h>
#endif

#include <array>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/oasis.h"

#ifdef HAVE_PARQUET
#include "../include/oasisparquet.h"
#endif

namespace aalcalcmeanonly {

  int nPeriods_ = 0;
  int maxPeriodNo_ = 0;
  int sampleSize_ = 0;
  std::unordered_map<int, double> eventToWeighting_;
  std::unordered_map<int, double> periodToWeighting_;
  std::map<int, std::array<double, 2>> means_;

  void LoadPeriodToWeighting() {

    FILE* fin = fopen(PERIODS_FILE, "rb");
    if (fin == nullptr) return;

    Periods p;
    size_t i = fread(&p, sizeof(Periods), 1, fin);
    while (i != 0) {
      periodToWeighting_[p.period_no] = p.weighting;
      i = fread(&p, sizeof(Periods), 1, fin);
    }

  }

  template<typename T>
  void LoadOccurrence(T &occ, FILE *fin) {

    size_t i = fread(&nPeriods_, sizeof(nPeriods_), 1, fin);
    double factor = (double)periodToWeighting_.size();
    double defaultWeighting = 1.0;
    if (periodToWeighting_.size() > 0) defaultWeighting = 0.0;

    i = fread(&occ, sizeof(occ), 1, fin);
    while (i != 0) {
      double weighting = defaultWeighting;
      if (periodToWeighting_.find(occ.period_no) != periodToWeighting_.end()) {
	weighting = periodToWeighting_[occ.period_no] * factor;
      }
      eventToWeighting_[occ.event_id] += weighting;
      if (maxPeriodNo_ < occ.period_no) maxPeriodNo_ = occ.period_no;
      i = fread(&occ, sizeof(occ), 1, fin);
    }

    if (maxPeriodNo_ > nPeriods_) {
      fprintf(stderr,
	      "FATAL: Maximum period number exceeds total number of periods in header of %s\n",
	      OCCURRENCE_FILE);
      exit(EXIT_FAILURE);
    }

    fclose(fin);

  }

  void LoadOccurrence() {

    int dateOpts;
    int granularDate = 0;
    FILE *fin = fopen(OCCURRENCE_FILE, "rb");
    if (fin == nullptr) {
      fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, OCCURRENCE_FILE);
      exit(EXIT_FAILURE);
    }

    fread(&dateOpts, sizeof(dateOpts), 1, fin);
    granularDate = dateOpts >> 1;
    if (granularDate) {
      occurrence_granular occ;
      LoadOccurrence(occ, fin);
    } else {
      occurrence occ;
      LoadOccurrence(occ, fin);
    }

  }

  inline void ReadInputStream(FILE *fin) {

    summarySampleslevelHeader sh;
    size_t i = fread(&sh, sizeof(sh), 1, fin);
    while (i != 0) {

      double weight = eventToWeighting_[sh.event_id];

      while (i != 0) {

        sampleslevelRec sr;
        i = fread(&sr, sizeof(sr), 1, fin);

        if (sr.sidx == chance_of_loss_idx || sr.sidx == max_loss_idx) continue;
        if (i == 0 || sr.sidx == 0) break;

        means_[sh.summary_id][sr.sidx != -1] += (sr.loss * weight / (1 + (sampleSize_ - 1) * (sr.sidx != -1)));

      }

      i = fread(&sh, sizeof(sh), 1, fin);

    }

  }

  void ReadFilesInDirectory(const std::string& path) {

    DIR *dir = nullptr;
    struct dirent *ent = nullptr;
    if ((dir = opendir(path.c_str())) != nullptr) {
      while ((ent = readdir(dir)) != nullptr) {
        std::string s = ent->d_name;
	if (s.length() > 4 && s.substr(s.length() - 4, 4) == ".bin") {
	  s = path + ent->d_name;
	  FILE *fin = fopen(s.c_str(), "rb");
	  if (fin == nullptr) {
	    fprintf(stderr, "FATAL: %s: cannot open %s\n", __func__, s.c_str());
	    exit(EXIT_FAILURE);
	  }
	  int summaryCalcStreamType = 0;
	  fread(&summaryCalcStreamType, sizeof(summaryCalcStreamType), 1, fin);
	  if (summaryCalcStreamType != (summarycalc_id | 1)) {
	    fprintf(stderr, "FATAL: %s: invalid stream type %d\n", __func__,
		    summaryCalcStreamType);
	    exit(EXIT_FAILURE);
	  }
	  fread(&sampleSize_, sizeof(sampleSize_), 1, fin);
	  int summarySet = 0;
	  fread(&summarySet, sizeof(summarySet), 1, fin);

	  ReadInputStream(fin);

	  fclose(fin);

	}

      }

    }

  }

  inline void WriteCsvOutputFile(const bool skipHeader, const bool ordOutput) {

    if (!skipHeader) {
      if (ordOutput) {
        printf("SummaryId,SampleType,MeanLoss\n");
      } else {
        printf("summary_id,type,mean\n");
      }
    }

    for (auto sumIter = means_.begin(); sumIter != means_.end(); ++sumIter) {
      for (auto typeIter = sumIter->second.begin();
	   typeIter != sumIter->second.end(); ++typeIter) {

        printf("%d,%ld,%f\n",
	       sumIter->first,
	       typeIter - sumIter->second.begin() + 1,
	       (*typeIter) / nPeriods_);

      }
    }

  }

#ifdef HAVE_PARQUET
  inline void WriteParquetOutputFile(const std::string& parquetOutFile) {

    std::vector<OasisParquet::ParquetFields> parquetFields;
    parquetFields.push_back({"SummaryId", parquet::Type::INT32,
                            parquet::ConvertedType::INT_32});
    parquetFields.push_back({"SampleType", parquet::Type::INT32,
                            parquet::ConvertedType::INT_32});
    parquetFields.push_back({"MeanLoss", parquet::Type::FLOAT,
                            parquet::ConvertedType::NONE});

    parquet::StreamWriter os =
      OasisParquet::SetupParquetOutputStream(parquetOutFile, parquetFields);

    for (auto sumIter = means_.begin(); sumIter != means_.end(); ++sumIter) {
      for (auto typeIter = sumIter->second.begin();
           typeIter != sumIter->second.end(); ++typeIter) {

        os << sumIter->first << typeIter - sumIter->second.begin() + 1
           << (*typeIter) / nPeriods_ << parquet::EndRow();

      }
    }

  }
#endif

  void OutputRows(const bool skipHeader, const bool ordOutput,
		  const std::string& parquetOutFile) {
  /* !parquetOutfile.empty() | ordOutput | output files
  * ---------------------------------------------------
  *            1             |     1     | ORD parquet and csv
  *            1             |     0     | ORD parquet
  *            0             |     1     | ORD csv
  *            0             |     0     | legacy csv
  */

    if (!(!parquetOutFile.empty() && !ordOutput)) {
      WriteCsvOutputFile(skipHeader, ordOutput);
    }

#ifdef HAVE_PARQUET
    if (!parquetOutFile.empty()) WriteParquetOutputFile(parquetOutFile);
#endif
	
  void DoIt(const std::string& subFolder, const bool skipHeader,
	    const bool ordOutput, const std::string& parquetOutFile) {

    LoadPeriodToWeighting();
    LoadOccurrence();

    std::string path = "work/" + subFolder;
    if (path.substr(path.length() - 1, 1) != "/") {
      path = path + "/";
    }
    ReadFilesInDirectory(path);
    OutputRows(skipHeader, ordOutput, parquetOutFile);

  }

}
