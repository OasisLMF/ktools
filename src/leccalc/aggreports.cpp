#include <algorithm>
#include <iostream>
#include <map>
#include <string.h>
#include <vector>

#include "aggreports.h"
#include "../include/oasis.h"


inline OASIS_FLOAT linear_interpolate(line_points lp, OASIS_FLOAT xpos) {
  return ((xpos - lp.to_x) * (lp.from_y - lp.to_y) / (lp.from_x - lp.to_x)) + lp.to_y;
}


bool operator<(const summary_id_period_key &lhs,
	       const summary_id_period_key &rhs) {

  if (lhs.summary_id != rhs.summary_id) {
    return lhs.summary_id < rhs.summary_id;
  } else {
    return lhs.period_no < rhs.period_no;
  }

}


bool operator<(const lossval& lhs, const lossval& rhs) {

  return lhs.value < rhs.value;

}


bool operator<(const wheatkey &lhs, const wheatkey &rhs) {

  if (lhs.summary_id != rhs.summary_id) {
    return lhs.summary_id < rhs.summary_id;
  } else {
    return lhs.sidx < rhs.sidx;
  }

}


aggreports::aggreports(const int totalperiods, const int maxsummaryid,
		       std::vector<std::map<outkey2, OutLosses>> &out_loss,
		       FILE **fout, const bool useReturnPeriodFile,
		       const int samplesize, const bool skipheader,
		       const bool *outputFlags, const bool ordFlag) :
  totalperiods_(totalperiods), maxsummaryid_(maxsummaryid),
  out_loss_(out_loss), fout_(fout), useReturnPeriodFile_(useReturnPeriodFile),
  samplesize_(samplesize), skipheader_(skipheader), outputFlags_(outputFlags),
  ordFlag_(ordFlag)
{

  LoadReturnPeriods();
  LoadPeriodsToWeighting();
  LoadEnsembleMapping();

}


void aggreports::LoadReturnPeriods() {

  if (useReturnPeriodFile_ == false) return;

  FILE *fin = fopen(RETURNPERIODS_FILE, "rb");
  if (fin == nullptr) {
    fprintf(stderr, "FATAL: Error opening file %s\n", RETURNPERIODS_FILE);
    exit(EXIT_FAILURE);
  }

  int return_period;
  while (fread(&return_period, sizeof(return_period), 1, fin) == 1) {
    returnperiods_.push_back(return_period);
  }

  fclose(fin);

  if (returnperiods_.size() == 0) {
    useReturnPeriodFile_ = false;
    fprintf(stderr, "WARNING: Empty return periods file. Running without "
		    "defined return periods option\n");
  }

}


void aggreports::LoadPeriodsToWeighting() {

  FILE *fin = fopen(PERIODS_FILE, "rb");
  if (fin == nullptr) return;

  Periods p;
  OASIS_FLOAT total_weighting = 0;
  while (fread(&p, sizeof(Periods), 1, fin) != 0) {
    total_weighting += (OASIS_FLOAT)p.weighting;
    periodstoweighting_[p.period_no] = p.weighting;
  }

  // Normalise weighting
  if (samplesize_) {
    for (auto iter = periodstoweighting_.begin();
	 iter != periodstoweighting_.end(); iter++) {
      iter->second /= samplesize_;
    }
  }

}


void aggreports::LoadEnsembleMapping() {

  FILE *fin = fopen(ENSEMBLE_FILE, "rb");
  if (fin == NULL) return;

  Ensemble e;
  std::vector<int> sidxtoensemble(samplesize_+1, 0);
  size_t i = fread(&e, sizeof(Ensemble), 1, fin);
  while (i != 0) {
    sidxtoensemble[e.sidx] = e.ensemble_id;
    ensembletosidx_[e.ensemble_id].push_back(e.sidx);
    i = fread(&e, sizeof(Ensemble), 1, fin);
  }

  // Check all sidx have ensemble IDs
  for (std::vector<int>::const_iterator it = std::next(sidxtoensemble.begin());
       it != sidxtoensemble.end(); ++it) {
    if (*it == 0) {
      fprintf(stderr, "FATAL: All sidx must have associated ensemble IDs\n");
      exit(EXIT_FAILURE);
    }
  }

}


OASIS_FLOAT aggreports::GetLoss(const OASIS_FLOAT next_return_period,
				const OASIS_FLOAT last_return_period,
				const OASIS_FLOAT last_loss,
				const OASIS_FLOAT current_return_period,
				const OASIS_FLOAT current_loss) const {

  if (current_return_period == 0.0) return 0.0;

  if (current_return_period == next_return_period) return current_loss;

  if (current_return_period < next_return_period) {
    line_points lpt;
    lpt.from_x = last_return_period;
    lpt.from_y = last_loss;
    lpt.to_x = current_return_period;
    lpt.to_y = current_loss;
    OASIS_FLOAT zz = linear_interpolate(lpt, next_return_period);
    return zz;
  }

  return -1;   // should not get here

}


void aggreports::FillTVaR(std::map<int, std::vector<TVaR>> &tail,
			  const int summary_id, const int epcalc,
			  const OASIS_FLOAT nextreturnperiod_value,
			  const OASIS_FLOAT tvar) {

  tail[summary_id].push_back({nextreturnperiod_value, tvar});

}


void aggreports::FillTVaR(std::map<wheatkey, std::vector<TVaR>> &tail,
			  const int summary_id, const int sidx,
			  const OASIS_FLOAT nextreturnperiod_value,
			  const OASIS_FLOAT tvar) {

  wheatkey wk;
  wk.summary_id = summary_id;
  wk.sidx = sidx;
  tail[wk].push_back({nextreturnperiod_value, tvar});

}


// epcalc = sidx for Wheatsheaf (Per Sample Exceedance Probability Table)
template<typename T>
void aggreports::WriteReturnPeriodOut(const std::vector<int> fileIDs,
	size_t &nextreturnperiod_index, OASIS_FLOAT &last_return_period,
	OASIS_FLOAT &last_loss, const OASIS_FLOAT current_return_period,
	const OASIS_FLOAT current_loss, const int summary_id, const int eptype,
	const int epcalc, const OASIS_FLOAT max_retperiod, int counter,
	OASIS_FLOAT tvar, T &tail,
	void (aggreports::*WriteOutput)(const std::vector<int>, const int,
					const int, const int,
					const OASIS_FLOAT, const OASIS_FLOAT))
{

  OASIS_FLOAT nextreturnperiod_value = 0;

  do {

    if (nextreturnperiod_index >= returnperiods_.size()) return;

    nextreturnperiod_value = returnperiods_[nextreturnperiod_index];

    if (current_return_period > nextreturnperiod_value) break;

    if (max_retperiod < nextreturnperiod_value) {
      nextreturnperiod_index++;
      continue;
    }

    OASIS_FLOAT loss = GetLoss(nextreturnperiod_value, last_return_period,
			       last_loss, current_return_period, current_loss);
    (this->*WriteOutput)(fileIDs, summary_id, epcalc, eptype,
			 nextreturnperiod_value, loss);

    tvar = tvar - ((tvar - loss) / counter);
    FillTVaR(tail, summary_id, epcalc, nextreturnperiod_value, tvar);

    nextreturnperiod_index++;
    counter++;

  } while (current_return_period <= nextreturnperiod_value);

  if (current_return_period > 0) {
    last_return_period = current_return_period;
    last_loss = current_loss;
  }

}


inline void aggreports::OutputRows(const std::vector<int> fileIDs,
				   const char * buffer, int strLen) {

  for (std::vector<int>::const_iterator it = fileIDs.begin();
       it != fileIDs.end(); ++it) {

    const char * bufPtr = buffer;
    int num;
    int counter = 0;
    do {

      num = fprintf(fout_[*it], "%s", bufPtr);
      if (num < 0) {   // Write error
	fprintf(stderr, "FATAL: Error writing %s: %s\n",
		buffer, strerror(errno));
	exit(EXIT_FAILURE);
      } else if (num < strLen) {   // Incomplete write
	bufPtr += num;
	strLen -= num;
      } else break;   // Success

      fprintf(stderr, "INFO: Attempt %d to write %s\n", ++counter, buffer);

      if (counter == 10) {
	fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n",
		buffer);
	exit(EXIT_FAILURE);
      }

    } while (true);

  }

}


void aggreports::WriteLegacyOutput(const std::vector<int> fileIDs,
				   const int summary_id, const int type,
				   const int ensemble_id,
				   const OASIS_FLOAT retperiod,
				   const OASIS_FLOAT loss) {

  const int bufferSize = 4096;
  char buffer[bufferSize];
  int strLen;
  strLen = snprintf(buffer, bufferSize, "%d,%d,%f,%f", summary_id, type,
		    retperiod, loss);
  if (ensembletosidx_.size() > 0) {
    strLen += snprintf(buffer+strLen, bufferSize-strLen, ",%d", ensemble_id);
  }
  strLen += snprintf(buffer+strLen, bufferSize-strLen, "\n");
  OutputRows(fileIDs, buffer, strLen);

}


void aggreports::WriteORDOutput(const std::vector<int> fileIDs,
				const int summary_id, const int epcalc,
				const int eptype, const OASIS_FLOAT retperiod,
				const OASIS_FLOAT loss) {

  const int bufferSize = 4096;
  char buffer[bufferSize];
  int strLen;
  strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", summary_id, epcalc,
		    eptype, retperiod, loss);
  OutputRows(fileIDs, buffer, strLen);

}


void aggreports::WriteTVaR(const std::vector<int> fileIDs, const int epcalc,
			   const int eptype_tvar,
			   const std::map<int, std::vector<TVaR>> &tail) {

  const int bufferSize = 4096;
  char buffer[bufferSize];
  int strLen;

  for (auto s : tail) {
    for (auto t : s.second) {
      buffer[0] = 0;
      strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n", s.first,
			epcalc, eptype_tvar, t.retperiod, t.tvar);
      OutputRows(fileIDs, buffer, strLen);
    }
  }

}


void aggreports::WriteTVaR(const std::vector<int> fileIDs,
			   const int eptype_tvar,
			   const std::map<wheatkey, std::vector<TVaR>> &tail) {

  const int bufferSize = 4096;
  char buffer[bufferSize];
  int strLen;

  for (auto s : tail) {
    for (auto t : s.second) {
      buffer[0] = 0;
      strLen = snprintf(buffer, bufferSize, "%d,%d,%d,%f,%f\n",
			s.first.summary_id, s.first.sidx, eptype_tvar,
			t.retperiod, t.tvar);
      OutputRows(fileIDs, buffer, strLen);
    }
  }

}


inline void aggreports::DoSetUp(int &eptype, int &eptype_tvar, int &epcalc,
	const int ensemble_id, const std::vector<int> fileIDs,
	void (aggreports::*&WriteOutput)(const std::vector<int>, const int,
					 const int, const int,
					 const OASIS_FLOAT, const OASIS_FLOAT))
{

  if (eptype == AEP) eptype_tvar = AEPTVAR;
  else if (eptype == OEP) eptype_tvar = OEPTVAR;

  if (epcalc == MEANDR && eptHeader_ == true) {
    std::string fileHeader;
    if (ordFlag_) {
      fileHeader = "SummaryID,EPCalc,EPType,ReturnPeriod,Loss\n";
      eptHeader_ = false;
    } else {
      fileHeader = "summary_id,type,return_period,loss";
      if (ensembletosidx_.size() > 0) fileHeader += ",ensemble_id";
      fileHeader += "\n";
    }
    for (std::vector<int>::const_iterator it = fileIDs.begin();
	 it != fileIDs.end(); ++it) {
      fprintf(fout_[*it], "%s", fileHeader.c_str());
    }
  }

  if (ordFlag_) {
    WriteOutput = &aggreports::WriteORDOutput;
  } else {
    WriteOutput = &aggreports::WriteLegacyOutput;
    // epcalc = type in legacy output
    // eptype = ensemble ID in legacy output
    if (epcalc == MEANDR) {
      epcalc = 1;
    } else {
      epcalc = 2;
    }
    eptype = ensemble_id;

  }

}


// No weighting
void aggreports::WriteExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<int, lossvec> &items,
	const OASIS_FLOAT max_retperiod, int epcalc, int eptype,
	int samplesize, int ensemble_id) {

  if (items.size() == 0) return;
  if (samplesize == 0) return;

  int eptype_tvar = 0;
  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
  DoSetUp(eptype, eptype_tvar, epcalc, ensemble_id, fileIDs, WriteOutput);

  std::map<int, std::vector<TVaR>> tail;

  for (auto s : items) {
    lossvec &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    size_t nextreturnperiodindex = 0;
    OASIS_FLOAT last_computed_rp = 0;
    OASIS_FLOAT last_computed_loss = 0;
    OASIS_FLOAT tvar = 0;
    int i = 1;

    for (auto lp : lpv) {
      OASIS_FLOAT retperiod = max_retperiod / i;

      if (useReturnPeriodFile_) {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, retperiod, lp / samplesize,
			     s.first, eptype, epcalc, max_retperiod, i, tvar,
			     tail, WriteOutput);
	tvar = tvar - ((tvar - (lp / samplesize)) / i);
      } else {
	tvar = tvar - ((tvar - (lp / samplesize)) / i);
	tail[s.first].push_back({retperiod, tvar});
	(this->*WriteOutput)(fileIDs, s.first, epcalc, eptype, retperiod,
			     lp / samplesize);
      }
      i++;
    }
    if (useReturnPeriodFile_) {
      do {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, 0, 0, s.first, eptype, epcalc,
			     max_retperiod, i, tvar, tail, WriteOutput);
      } while (nextreturnperiodindex < returnperiods_.size());
    }
  }

  // Only write Tail Value at Risk (TVaR) for ORD output
  if (!ordFlag_) return;
  WriteTVaR(fileIDs, epcalc, eptype_tvar, tail);

}


// With weighting
void aggreports::WriteExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<int, lossvec2> &items,
	const OASIS_FLOAT cum_weight_constant,
	int epcalc, int eptype, int samplesize, int ensemble_id) {

  if (items.size() == 0) return;
  if (samplesize == 0) return;

  int eptype_tvar = 0;
  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
  DoSetUp(eptype, eptype_tvar, epcalc, ensemble_id, fileIDs, WriteOutput);

  std::map<int, std::vector<TVaR>> tail;

  for (auto s : items) {
    lossvec2 &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    size_t nextreturnperiodindex = 0;
    OASIS_FLOAT last_computed_rp = 0;
    OASIS_FLOAT last_computed_loss = 0;
    OASIS_FLOAT tvar = 0;
    int i = 1;
    OASIS_FLOAT cummulative_weighting = 0;
    OASIS_FLOAT max_retperiod = 0;
    bool largest_loss = false;

    for (auto lp : lpv) {
      cummulative_weighting += (lp.period_weighting * cum_weight_constant);

      if (lp.period_weighting) {
	OASIS_FLOAT retperiod = 1 / cummulative_weighting;

	if (!largest_loss) {
	  max_retperiod = retperiod + 0.0001;   // Add for floating point errors
	  largest_loss = true;
	}

	if (useReturnPeriodFile_) {
	  WriteReturnPeriodOut(fileIDs, nextreturnperiodindex,
			       last_computed_rp, last_computed_loss, retperiod,
			       lp.value / samplesize, s.first, eptype, epcalc,
			       max_retperiod, i, tvar, tail, WriteOutput);
	  tvar = tvar - ((tvar - (lp.value / samplesize)) / i);
	} else {
	  tvar = tvar - ((tvar - (lp.value / samplesize)) / i);
	  tail[s.first].push_back({retperiod, tvar});
	  (this->*WriteOutput)(fileIDs, s.first, epcalc, eptype, retperiod,
			       lp.value / samplesize);
	}
	i++;
      }
    }
    if (useReturnPeriodFile_) {
      do {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, 0, 0, s.first, eptype, epcalc,
			     max_retperiod, i, tvar, tail, WriteOutput);
      } while (nextreturnperiodindex < returnperiods_.size());
    }
  }

  // Only write Tail Value at Risk (TVaR) for ORD output
  if (!ordFlag_) return;
  WriteTVaR(fileIDs, epcalc, eptype_tvar, tail);

}


inline void aggreports::DoSetUpWheatsheaf(int &eptype, int &eptype_tvar,
	const int ensemble_id, const std::vector<int> fileIDs,
	void (aggreports::*&WriteOutput)(const std::vector<int>, const int,
					 const int, const int,
					 const OASIS_FLOAT, const OASIS_FLOAT))
{

  if (eptype == AEP) eptype_tvar = AEPTVAR;
  else if (eptype == OEP) eptype_tvar = OEPTVAR;

  std::string fileHeader;
  if (ordFlag_ && pseptHeader_) {
    fileHeader = "SummaryID,SampleID,EPType,ReturnPeriod,Loss\n";
    pseptHeader_ = false;
  } else if (!ordFlag_ && wheatSheaf_[eptype] == false) {
    fileHeader = "summary_id,sidx,return_period,loss";
    if (ensembletosidx_.size() > 0) fileHeader += ",ensemble_id";
    fileHeader += "\n";
    wheatSheaf_[eptype] = true;
  }
  for (std::vector<int>::const_iterator it = fileIDs.begin();
       it != fileIDs.end(); ++it) {
    fprintf(fout_[*it], "%s", fileHeader.c_str());
  }

  if (ordFlag_) {
    WriteOutput = &aggreports::WriteORDOutput;
  } else {
    WriteOutput = &aggreports::WriteLegacyOutput;
    eptype = ensemble_id;
  }

}


// No weighting
void aggreports::WritePerSampleExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<wheatkey, lossvec> &items,
	int eptype, int ensemble_id) {

  if (items.size() == 0) return;

  int eptype_tvar = 0;
  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
  DoSetUpWheatsheaf(eptype, eptype_tvar, ensemble_id, fileIDs, WriteOutput);

  std::map<wheatkey, std::vector<TVaR>> tail;
  const OASIS_FLOAT max_retperiod = totalperiods_;

  for (auto s : items) {
    lossvec &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    size_t nextreturnperiodindex = 0;
    OASIS_FLOAT last_computed_rp = 0;
    OASIS_FLOAT last_computed_loss = 0;
    OASIS_FLOAT tvar = 0;
    int i = 1;

    for (auto lp : lpv) {
      OASIS_FLOAT retperiod = max_retperiod / i;

      if (useReturnPeriodFile_) {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, retperiod, lp,
			     s.first.summary_id, eptype, s.first.sidx,
			     max_retperiod, i, tvar, tail, WriteOutput);
	tvar = tvar - ((tvar - lp) / i);
      } else {
	tvar = tvar - ((tvar - lp) / i);
	tail[s.first].push_back({retperiod, tvar});
	(this->*WriteOutput)(fileIDs, s.first.summary_id, s.first.sidx, eptype,
			     retperiod, lp);
      }
      i++;
    }
    if (useReturnPeriodFile_) {
      do {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, 0, 0, s.first.summary_id,
			     eptype, s.first.sidx, max_retperiod, i, tvar,
			     tail, WriteOutput);
      } while (nextreturnperiodindex < returnperiods_.size());
    }
  }

  // Only write Tail Value at Risk (TVaR) for ORD output
  if (!ordFlag_) return;
  WriteTVaR(fileIDs, eptype_tvar, tail);

}


// With weighting
void aggreports::WritePerSampleExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<wheatkey, lossvec2> &items,
	int eptype, int ensemble_id) {

  if (items.size() == 0) return;

  int eptype_tvar = 0;
  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
  DoSetUpWheatsheaf(eptype, eptype_tvar, ensemble_id, fileIDs, WriteOutput);

  std::map<wheatkey, std::vector<TVaR>> tail;

  for (auto s : items) {
    lossvec2 &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    size_t nextreturnperiodindex = 0;
    OASIS_FLOAT last_computed_rp = 0;
    OASIS_FLOAT last_computed_loss = 0;
    OASIS_FLOAT tvar = 0;
    int i = 1;
    OASIS_FLOAT cummulative_weighting = 0;
    OASIS_FLOAT max_retperiod = 0;
    bool largest_loss = false;

    for (auto lp : lpv) {
      cummulative_weighting += (OASIS_FLOAT)lp.period_weighting * samplesize_;

      if (lp.period_weighting) {
	OASIS_FLOAT retperiod = 1 / cummulative_weighting;

	if (!largest_loss) {
	  max_retperiod = retperiod + 0.0001;   // Add for floating point errors
	  largest_loss = true;
	}

	if (useReturnPeriodFile_) {
	  WriteReturnPeriodOut(fileIDs, nextreturnperiodindex,
			       last_computed_rp, last_computed_loss, retperiod,
			       lp.value, s.first.summary_id, eptype,
			       s.first.sidx, max_retperiod, i, tvar, tail,
			       WriteOutput);
	  tvar = tvar - ((tvar - lp.value) / i);
	} else {
	  tvar = tvar - ((tvar - lp.value) / i);
	  tail[s.first].push_back({retperiod, tvar});
	  (this->*WriteOutput)(fileIDs, s.first.summary_id, s.first.sidx,
			       eptype, retperiod, lp.value);
	}
	i++;
      }
    }
    if (useReturnPeriodFile_) {
      do {
	WriteReturnPeriodOut(fileIDs, nextreturnperiodindex, last_computed_rp,
			     last_computed_loss, 0, 0, s.first.summary_id,
			     eptype, s.first.sidx, max_retperiod, i, tvar,
			     tail, WriteOutput);
      } while (nextreturnperiodindex < returnperiods_.size());
    }
  }

  // Only write Tail Value at Risk (TVaR) for ORD output
  if (!ordFlag_) return;
  WriteTVaR(fileIDs, eptype_tvar, tail);

}


void aggreports::MeanDamageRatio(const std::vector<int> fileIDs,
				 OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				 const int eptype) {

  std::map<int, lossvec> items;

  for (auto x : out_loss_[MEANS]) {
    items[x.first.summary_id].push_back((x.second.*GetOutLoss)());
  }

  WriteExceedanceProbabilityTable(fileIDs, items, totalperiods_, MEANDR,
				  eptype);

}


void aggreports::MeanDamageRatioWithWeighting(const std::vector<int> fileIDs,
	OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype) {

  std::map<int, lossvec2> items;

  for (auto x : out_loss_[MEANS]) {
    lossval lv;
    lv.value = (x.second.*GetOutLoss)();
    auto iter = periodstoweighting_.find(x.first.period_no);
    if (iter != periodstoweighting_.end()) {
      lv.period_weighting = iter->second;
      lv.period_no = x.first.period_no;   // for debugging
      items[x.first.summary_id].push_back(lv);
    }
  }

  WriteExceedanceProbabilityTable(fileIDs, items, samplesize_, MEANDR, eptype);

}


void aggreports::OutputAggMeanDamageRatio() {

  // Determine which files to write to for legacy stream
  // Determine whether Mean Damage Ratio should be written for ORD stream
  std::vector<int> fileIDs;
  std::vector<int> handles = { AGG_FULL_UNCERTAINTY, AGG_SAMPLE_MEAN,
			       AGG_WHEATSHEAF_MEAN };
  for (std::vector<int>::iterator it = handles.begin(); it != handles.end();
       ++it) {
    if (outputFlags_[*it] == true) fileIDs.push_back(*it);
  }

  if (fileIDs.size() == 0) return;   // No suitable files

  if (ordFlag_) {
    fileIDs.clear();
    fileIDs.push_back(EPT);
  }

  if (periodstoweighting_.size() == 0) {
    MeanDamageRatio(fileIDs, GetAgg, AEP);
  } else {
    MeanDamageRatioWithWeighting(fileIDs, GetAgg, AEP);
  }

}


void aggreports::OutputOccMeanDamageRatio() {

  // Determine which files to write to for legacy stream
  // Determine whether Mean Damage Ratio should be written for ORD stream
  std::vector<int> fileIDs;
  std::vector<int> handles = { OCC_FULL_UNCERTAINTY, OCC_SAMPLE_MEAN,
			       OCC_WHEATSHEAF_MEAN };
  for (std::vector<int>::iterator it = handles.begin(); it != handles.end();
       ++it) {
    if (outputFlags_[*it] == true) fileIDs.push_back(*it);
  }

  if (fileIDs.size() == 0) return;   // No suitable files

  if (ordFlag_) {
    fileIDs.clear();
    fileIDs.push_back(EPT);
  }

  if (periodstoweighting_.size() == 0) {
    MeanDamageRatio(fileIDs, GetMax, OEP);
  } else {
    MeanDamageRatioWithWeighting(fileIDs, GetMax, OEP);
  }

}


inline std::vector<int> aggreports::GetFileIDs(const int handle, int table) {

  if (ordFlag_) {
    return std::vector<int>(1, table);
  } else {
    return std::vector<int>(1, handle);
  }

}


void aggreports::FullUncertainty(const std::vector<int> fileIDs,
				 OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				 const int eptype) {

  std::map<int, lossvec> items;

  for (auto x : out_loss_[SAMPLES]) {
    items[x.first.summary_id].push_back((x.second.*GetOutLoss)());
  }

  WriteExceedanceProbabilityTable(fileIDs, items, totalperiods_ * samplesize_,
				  FULL, eptype);

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      items.clear();
      for (auto x : out_loss_[SAMPLES]) {
	for (auto sidx : ensemble.second) {
	  if (x.first.sidx == sidx) {
	    items[x.first.summary_id].push_back((x.second.*GetOutLoss)());
	  }
	}
      }
      WriteExceedanceProbabilityTable(fileIDs, items,
				      totalperiods_ * ensemble.second.size(),
				      FULL, eptype, 1, ensemble.first);
    }
  }

}


void aggreports::FullUncertaintyWithWeighting(const std::vector<int> fileIDs,
	OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype) {

  std::map<int, lossvec2> items;

  for (auto x : out_loss_[SAMPLES]) {
    lossval lv;
    lv.value = (x.second.*GetOutLoss)();
    auto iter = periodstoweighting_.find(x.first.period_no);
    if (iter != periodstoweighting_.end()) {
      lv.period_weighting = iter->second;
      lv.period_no = x.first.period_no;   // for debugging
      items[x.first.summary_id].push_back(lv);
    }
  }

  WriteExceedanceProbabilityTable(fileIDs, items, 1, FULL, eptype);

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      items.clear();
      for (auto x : out_loss_[SAMPLES]) {
	for (auto sidx : ensemble.second) {
	  if (x.first.sidx == sidx) {
	    lossval lv;
	    lv.value = (x.second.*GetOutLoss)();
	    auto iter = periodstoweighting_.find(x.first.period_no);
	    if (iter != periodstoweighting_.end()) {
	      lv.period_weighting = iter->second;
	      lv.period_no = x.first.period_no;   // for debugging
	      items[x.first.summary_id].push_back(lv);
	    }
	  }
	}
      }
      WriteExceedanceProbabilityTable(fileIDs, items, 1, FULL, eptype, 1,
				      ensemble.first);
    }
  }

}


void aggreports::OutputAggFullUncertainty() {

  const int handle = AGG_FULL_UNCERTAINTY;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  if (periodstoweighting_.size() == 0) {
    FullUncertainty(fileIDs, GetAgg, AEP);
  } else {
    FullUncertaintyWithWeighting(fileIDs, GetAgg, AEP);
  }

}


void aggreports::OutputOccFullUncertainty() {

  const int handle = OCC_FULL_UNCERTAINTY;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  if (periodstoweighting_.size() == 0) {
    FullUncertainty(fileIDs, GetMax, OEP);
  } else {
    FullUncertaintyWithWeighting(fileIDs, GetMax, OEP);
  }

}


inline void aggreports::FillWheatsheafItems(const outkey2 key,
					    std::map<wheatkey, lossvec> &items,
					    const OASIS_FLOAT loss) {

  wheatkey wk;
  wk.sidx = key.sidx;
  wk.summary_id = key.summary_id;
  items[wk].push_back(loss);

}


inline void aggreports::FillWheatsheafItems(const outkey2 key,
	std::map<wheatkey, lossvec2> &items, const OASIS_FLOAT loss,
	std::vector<int> &maxPeriodNo) {

  wheatkey wk;
  wk.sidx = key.sidx;
  wk.summary_id = key.summary_id;
  lossval lv;
  lv.value = loss;
  auto iter = periodstoweighting_.find(key.period_no);
  if (iter != periodstoweighting_.end()) {
    lv.period_weighting = iter->second;
    lv.period_no = key.period_no;
    if (lv.period_no > maxPeriodNo[key.summary_id]) {
	maxPeriodNo[key.summary_id] = lv.period_no;
    }
    items[wk].push_back(lv);
  }

}


// Wheatsheaf (Per Sample Exceedance Probability Table)
// Wheatsheaf Mean = Per Sample Mean (Exceedance Probability Table)
void aggreports::WheatsheafAndWheatsheafMean(const std::vector<int> handles,
				OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				const int eptype, const int ensemble_id) {

  std::map<wheatkey, lossvec> items;
  int samplesize;

  if (ensemble_id != 0) {
    samplesize = (int)ensembletosidx_[ensemble_id].size();
    for (auto x : out_loss_[SAMPLES]) {
      for (auto sidx : ensembletosidx_[ensemble_id]) {
	if (x.first.sidx == sidx) {
	  FillWheatsheafItems(x.first, items, (x.second.*GetOutLoss)());
	}
      }
    }
  } else {
    samplesize = samplesize_;
    for (auto x : out_loss_[SAMPLES]) {
      FillWheatsheafItems(x.first, items, (x.second.*GetOutLoss)());
    }
  }

  if (outputFlags_[handles[WHEATSHEAF]] == true) {
    std::vector<int> fileIDs = GetFileIDs(handles[WHEATSHEAF], PSEPT);
    WritePerSampleExceedanceProbabilityTable(fileIDs, items, eptype,
					     ensemble_id);
  }

  if (outputFlags_[handles[WHEATSHEAF_MEAN]] == false) return;

  std::vector<size_t> maxCount(maxsummaryid_+1, 0);
  for (auto x : items) {
    if (x.second.size() > maxCount[x.first.summary_id]) {
      maxCount[x.first.summary_id] = x.second.size();
    }
  }

  std::map<int, lossvec> mean_map;
  for (int i = 1; i <= maxsummaryid_; i++) {
    mean_map[i] = lossvec(maxCount[i], 0);
  }

  for (auto s : items) {
    lossvec &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    int i = 0;
    for (auto lp : lpv) {
      mean_map[s.first.summary_id][i] += lp;
      i++;
    }
  }

  std::vector<int> fileIDs = GetFileIDs(handles[WHEATSHEAF_MEAN]);
  WriteExceedanceProbabilityTable(fileIDs, mean_map, totalperiods_,
				  PERSAMPLEMEAN, eptype, samplesize,
				  ensemble_id);

}


// Wheatsheaf (Per Sample Exceedance Probability Table)
// Wheatsheaf Mean = Per Sample Mean (Exceedance Probability Table)
void aggreports::WheatsheafAndWheatsheafMeanWithWeighting(
	const std::vector<int> handles, OASIS_FLOAT (OutLosses::*GetOutLoss)(),
	const int eptype, const int ensemble_id) {

  std::map<wheatkey, lossvec2> items;
  int samplesize;
  std::vector<int> maxPeriodNo(maxsummaryid_+1, 0);

  if (ensemble_id != 0) {
    samplesize = (int)ensembletosidx_[ensemble_id].size();
    for (auto x : out_loss_[SAMPLES]) {
      for (auto sidx : ensembletosidx_[ensemble_id]) {
	if (x.first.sidx == sidx) {
	  FillWheatsheafItems(x.first, items, (x.second.*GetOutLoss)(),
			      maxPeriodNo);
	}
      }
    }
  } else {
    samplesize = samplesize_;
    for (auto x : out_loss_[SAMPLES]) {
      FillWheatsheafItems(x.first, items, (x.second.*GetOutLoss)(),
			  maxPeriodNo);
    }
  }

  if (outputFlags_[handles[WHEATSHEAF]] == true) {
    std::vector<int> fileIDs = GetFileIDs(handles[WHEATSHEAF], PSEPT);
    WritePerSampleExceedanceProbabilityTable(fileIDs, items, eptype,
					     ensemble_id);
  }

  if (outputFlags_[handles[WHEATSHEAF_MEAN]] == false) return;

  std::map<int, lossvec2> mean_map;
  for (int i = 1; i <= maxsummaryid_; i++) {
    mean_map[i] = lossvec2(maxPeriodNo[i], lossval());
  }

  for (auto s : items) {
    lossvec2 &lpv = s.second;
    std::sort(lpv.rbegin(), lpv.rend());
    for (auto lp : lpv) {
      lossval &l = mean_map[s.first.summary_id][lp.period_no-1];
      l.period_no = lp.period_no;
      l.period_weighting = lp.period_weighting;
      l.value += lp.value;
    }
  }

  std::vector<int> fileIDs = GetFileIDs(handles[WHEATSHEAF_MEAN]);
  WriteExceedanceProbabilityTable(fileIDs, mean_map, samplesize,
				  PERSAMPLEMEAN, eptype, samplesize,
				  ensemble_id);

}


// Wheatsheaf Mean = Per Sample Mean
void aggreports::OutputAggWheatsheafAndWheatsheafMean() {

  std::vector<int> handles = { AGG_WHEATSHEAF, AGG_WHEATSHEAF_MEAN };
  int falseCount = 0;
  for (std::vector<int>::iterator it = handles.begin(); it != handles.end();
       ++it) {
    if (outputFlags_[*it] == false) falseCount++;
  }
  if (falseCount == 2) return;

  if (periodstoweighting_.size() == 0) {
    WheatsheafAndWheatsheafMean(handles, GetAgg, AEP);
  } else {
    WheatsheafAndWheatsheafMeanWithWeighting(handles, GetAgg, AEP);
  }

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      if (periodstoweighting_.size() == 0) {
	WheatsheafAndWheatsheafMean(handles, GetAgg, AEP, ensemble.first);
      } else {
	WheatsheafAndWheatsheafMeanWithWeighting(handles, GetAgg, AEP,
						 ensemble.first);
      }
    }
  }

}


// Wheatsheaf Mean = Per Sample Mean
void aggreports::OutputOccWheatsheafAndWheatsheafMean() {

  std::vector<int> handles = { OCC_WHEATSHEAF, OCC_WHEATSHEAF_MEAN };
  int falseCount = 0;
  for (std::vector<int>::iterator it = handles.begin(); it != handles.end();
       ++it) {
    if (outputFlags_[*it] == false) falseCount++;
  }
  if (falseCount == 2) return;

  if (periodstoweighting_.size() == 0) {
    WheatsheafAndWheatsheafMean(handles, GetMax, OEP);
  } else {
    WheatsheafAndWheatsheafMeanWithWeighting(handles, GetMax, OEP);
  }

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      if (periodstoweighting_.size() == 0) {
	WheatsheafAndWheatsheafMean(handles, GetMax, OEP, ensemble.first);
      } else {
	WheatsheafAndWheatsheafMeanWithWeighting(handles, GetMax, OEP,
						 ensemble.first);
      }
    }
  }

}


void aggreports::SampleMean(const std::vector<int> fileIDs,
			    OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			    const int eptype) {

  if (samplesize_ == 0) return;   // Prevent division by zero error

  std::map<summary_id_period_key, OASIS_FLOAT> items;
  for (auto x : out_loss_[SAMPLES]) {
    summary_id_period_key sk;
    sk.summary_id = x.first.summary_id;
    sk.period_no = x.first.period_no;
    items[sk] += ((x.second.*GetOutLoss)() / samplesize_);
  }

  std::map<int, lossvec> mean_map;
  for (auto s : items) {
    mean_map[s.first.summary_id].push_back(s.second);
  }

  WriteExceedanceProbabilityTable(fileIDs, mean_map, totalperiods_, MEANSAMPLE,
				  eptype);

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      items.clear();
      for (auto x : out_loss_[SAMPLES]) {
	for (auto sidx : ensemble.second) {
	  if (x.first.sidx == sidx) {
	    summary_id_period_key sk;
	    sk.summary_id = x.first.summary_id;
	    sk.period_no = x.first.period_no;
	    items[sk] += ((x.second.*GetOutLoss)() / ensemble.second.size());
	  }
	}
      }
      mean_map.clear();
      for (auto s : items) {
	mean_map[s.first.summary_id].push_back(s.second);
      }
      WriteExceedanceProbabilityTable(fileIDs, mean_map, totalperiods_,
				      MEANSAMPLE, eptype, 1, ensemble.first);
    }
  }

}


void aggreports::SampleMeanWithWeighting(const std::vector<int> fileIDs,
	OASIS_FLOAT (OutLosses::*GetOutLoss)(), const int eptype) {

  if (samplesize_ == 0) return;   // Prevent division by zero error

  std::map<summary_id_period_key, lossval> items;
  for (auto x : out_loss_[SAMPLES]) {
    summary_id_period_key sk;
    sk.summary_id = x.first.summary_id;
    sk.period_no = x.first.period_no;
    auto iter = periodstoweighting_.find(x.first.period_no);
    if (iter != periodstoweighting_.end()) {
      items[sk].period_weighting = iter->second;
      items[sk].period_no = x.first.period_no;   // for debugging
      items[sk].value += ((x.second.*GetOutLoss)() / samplesize_);
    }
  }

  std::map<int, lossvec2> mean_map;
  for (auto s : items) {
    mean_map[s.first.summary_id].push_back(s.second);
  }

  WriteExceedanceProbabilityTable(fileIDs, mean_map, samplesize_, MEANSAMPLE,
				  eptype);

  // By ensemble ID
  if (ordFlag_) return;   // Ensemble IDs not supported for ORD output
  if (ensembletosidx_.size() > 0) {
    for (auto ensemble : ensembletosidx_) {
      items.clear();
      for (auto x : out_loss_[SAMPLES]) {
	for (auto sidx : ensemble.second) {
	  if (x.first.sidx == sidx) {
	    summary_id_period_key sk;
	    sk.summary_id = x.first.summary_id;
	    sk.period_no = x.first.period_no;
	    auto iter = periodstoweighting_.find(x.first.period_no);
	    if (iter != periodstoweighting_.end()) {
	      items[sk].period_weighting = iter->second;
	      items[sk].period_no = x.first.period_no;   // for debugging
	      items[sk].value += ((x.second.*GetOutLoss)() / ensemble.second.size());
	    }
	  }
	}
      }
      mean_map.clear();
      for (auto s : items) {
	mean_map[s.first.summary_id].push_back(s.second);
      }
      WriteExceedanceProbabilityTable(fileIDs, mean_map, samplesize_,
				      MEANSAMPLE, eptype, 1, ensemble.first);
    }
  }
}


void aggreports::OutputAggSampleMean() {

  const int handle = AGG_SAMPLE_MEAN;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  if (periodstoweighting_.size() == 0) {
    SampleMean(fileIDs, GetAgg, AEP);
  } else {
    SampleMeanWithWeighting(fileIDs, GetAgg, AEP);
  }

}


void aggreports::OutputOccSampleMean() {

  const int handle = OCC_SAMPLE_MEAN;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  if (periodstoweighting_.size() == 0) {
    SampleMean(fileIDs, GetMax, OEP);
  } else {
    SampleMeanWithWeighting(fileIDs, GetMax, OEP);
  }

}
