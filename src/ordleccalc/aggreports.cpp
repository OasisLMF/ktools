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
      } else return;   // Success

      fprintf(stderr, "INFO: Attempt %d to write %s\n", ++counter, buffer);

    } while (counter < 10);

    fprintf(stderr, "FATAL: Maximum attempts to write %s exceeded\n", buffer);
    exit (EXIT_FAILURE);

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


void aggreports::WriteExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<int, lossvec> &items,
	const OASIS_FLOAT max_retperiod, int epcalc, int eptype,
	int samplesize, int ensemble_id) {

  if (items.size() == 0) return;

  int eptype_tvar = 0;
  if (eptype == AEP) eptype_tvar = AEPTVAR;
  else if (eptype == OEP) eptype_tvar = OEPTVAR;

  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
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

  }

  // Only write Tail Value at Risk (TVaR) for ORD output
  if (!ordFlag_) return;
  WriteTVaR(fileIDs, epcalc, eptype_tvar, tail);

}


void aggreports::WritePerSampleExceedanceProbabilityTable(
	const std::vector<int> fileIDs, std::map<wheatkey, lossvec> &items,
	int eptype, int ensemble_id) {

  if (items.size() == 0) return;

  int eptype_tvar = 0;
  if (eptype == AEP) eptype_tvar = AEPTVAR;
  else if (eptype == OEP) eptype_tvar = OEPTVAR;

  void (aggreports::*WriteOutput)(const std::vector<int>, const int, const int,
				  const int, const OASIS_FLOAT,
				  const OASIS_FLOAT);
  if (ordFlag_) {
    WriteOutput = &aggreports::WriteORDOutput;
  } else {
    WriteOutput = &aggreports::WriteLegacyOutput;
    eptype = ensemble_id;
  }

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

  MeanDamageRatio(fileIDs, GetAgg, AEP);

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

  MeanDamageRatio(fileIDs, GetMax, OEP);

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

}


void aggreports::OutputAggFullUncertainty() {

  const int handle = AGG_FULL_UNCERTAINTY;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  FullUncertainty(fileIDs, GetAgg, AEP);

}


void aggreports::OutputOccFullUncertainty() {

  const int handle = OCC_FULL_UNCERTAINTY;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  FullUncertainty(fileIDs, GetMax, OEP);

}


// Wheatsheaf (Per Sample Exceedance Probability Table)
// Wheatsheaf Mean = Per Sample Mean (Exceedance Probability Table)
void aggreports::WheatsheafAndWheatsheafMean(const std::vector<int> handles,
				OASIS_FLOAT (OutLosses::*GetOutLoss)(),
				const int eptype) {

  std::map<wheatkey, lossvec> items;

  for (auto x : out_loss_[SAMPLES]) {
    wheatkey wk;
    wk.sidx = x.first.sidx;
    wk.summary_id = x.first.summary_id;
    items[wk].push_back((x.second.*GetOutLoss)());
  }

  if (outputFlags_[handles[WHEATSHEAF]] == true) {
    std::vector<int> fileIDs = GetFileIDs(handles[WHEATSHEAF], PSEPT);
    WritePerSampleExceedanceProbabilityTable(fileIDs, items, eptype);
  }

  if (outputFlags_[handles[WHEATSHEAF_MEAN]] == false) return;

  std::vector<size_t> maxCount(maxsummaryid_, 0);
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
				  PERSAMPLEMEAN, eptype, samplesize_);

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

  WheatsheafAndWheatsheafMean(handles, GetAgg, AEP);

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

  WheatsheafAndWheatsheafMean(handles, GetMax, OEP);

}


void aggreports::SampleMean(const std::vector<int> fileIDs,
			    OASIS_FLOAT (OutLosses::*GetOutLoss)(),
			    const int eptype) {

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

}


void aggreports::OutputAggSampleMean() {

  const int handle = AGG_SAMPLE_MEAN;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  SampleMean(fileIDs, GetAgg, AEP);

}


void aggreports::OutputOccSampleMean() {

  const int handle = OCC_SAMPLE_MEAN;

  if (outputFlags_[handle] == false) return;

  std::vector<int> fileIDs = GetFileIDs(handle);

  SampleMean(fileIDs, GetMax, OEP);

}
