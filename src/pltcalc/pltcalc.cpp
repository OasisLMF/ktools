#include "../include/oasis.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <math.h>
using namespace std;
struct period_occ {
	int period_no;
	int occ_date_id;
};

std::map<int, std::vector<period_occ> > m_occ;
int date_algorithm_=0;
int samplesize_ = 0;

void d(long long g, int &y, int &mm, int &dd)
{
	y = (10000 * g + 14780) / 3652425;
	int ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
	if (ddd < 0) {
		y = y - 1;
		ddd = g - (365 * y + y / 4 - y / 100 + y / 400);
	}
	int mi = (100 * ddd + 52) / 3060;
	mm = (mi + 2) % 12 + 1;
	y = y + (mi + 2) / 12;
	dd = ddd - (mi * 306 + 5) / 10 + 1;
	return;
}

void loadoccurrence()
{
	FILE *fin = fopen(OCCURRENCE_FILE, "rb");
	if (fin == NULL) {
		cerr << "loadoccurrence: Unable to open " << OCCURRENCE_FILE << "\n";
		exit(-1);
	}
	
	int no_of_periods = 0;
	occurrence occ;
	int i = fread(&date_algorithm_, sizeof(date_algorithm_), 1, fin);
	i = fread(&no_of_periods, sizeof(no_of_periods), 1, fin);
	i = fread(&occ, sizeof(occ), 1, fin);
	while (i != 0) {
		period_occ p;
		p.occ_date_id = occ.occ_date_id;
		p.period_no = occ.period_no;
		m_occ[occ.event_id].push_back(p);
		//event_to_periods[occ.event_id].push_back(occ.period_no);
		i = fread(&occ, sizeof(occ), 1, fin);
	}

	fclose(fin);

}


struct outrec {
	int summary_id;
	int period_no;
	int event_id;
	float mean;
	float standard_deviation;
	float exp_value;
	int occ_date_id;
};
void dopltcalc(const summarySampleslevelHeader &sh, const std::vector<sampleslevelRec> &vrec)
{
	static int j = 0;
	std::vector<period_occ> &vp = m_occ[sh.event_id];	
	bool hasrec = false;
	bool firsttime = true;
	outrec o;
	o.event_id = sh.event_id;
	o.summary_id = sh.summary_id;
	o.exp_value = sh.expval;
	o.mean = 0;
	o.standard_deviation = 0;

	float squared_loss_sum = 0;
	float loss_sum = 0;
	for (auto p : vp) {
		o.period_no = p.period_no;
		o.occ_date_id = p.occ_date_id;
		if (firsttime == true) { // only do this once
			for (auto v : vrec) {
				if (v.sidx > 0) {
					hasrec = true;
					loss_sum += v.loss;
					squared_loss_sum += (v.loss*v.loss);
				}
			}
			firsttime = false;
		}
		if (hasrec) {
			o.mean = loss_sum / samplesize_;
			//o.standard_deviation = ((squared_loss_sum - loss_sum)/ samplesize_)/(samplesize_ -1);
			float sd = (squared_loss_sum - ((loss_sum*loss_sum) / samplesize_)) / (samplesize_ - 1);
			float x = sd / squared_loss_sum;
			if (x < 0.0000001) sd = 0;   // fix floating point precision problems caused by using large numbers
			o.standard_deviation = sqrt(sd);
			if (date_algorithm_) {
				int occ_year, occ_month, occ_day;
				d(o.occ_date_id, occ_year, occ_month, occ_day);
				printf("%d,%d,%d,%0.2f,%0.2f,%0.2f,%d,%d,%d\n", o.summary_id, o.period_no, o.event_id, o.mean, o.standard_deviation, o.exp_value, occ_year, occ_month,occ_day);
			}
			else {
				printf("%d,%d,%d,%0.2f,%0.2f,%0.2f,%d\n", o.summary_id, o.period_no, o.event_id, o.mean, o.standard_deviation, o.exp_value, o.occ_date_id);
			}
			j++;
		}
	}

}

void doit()
{
	loadoccurrence();
	int summarycalcstream_type = 0;
	int i = fread(&summarycalcstream_type, sizeof(summarycalcstream_type), 1, stdin);
	int stream_type = summarycalcstream_type & summarycalc_id;

	if (stream_type != summarycalc_id) {
		std::cerr << "Not a summarycalc stream type\n";
		exit(-1);
	}
	stream_type = streamno_mask & summarycalcstream_type;
	if (date_algorithm_) {
		printf("summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_year,occ_month,occ_day\n");
	}else {
		printf("summary_id,period_no,event_id,mean,standard_deviation,exposure_value,occ_date_id\n");
	}
	if (stream_type == 1) {
		int summary_set = 0;
		int j=0;
		i = fread(&samplesize_, sizeof(samplesize_), 1, stdin);
		if (i != 0) i = fread(&summary_set, sizeof(summary_set), 1, stdin);
		std::vector<sampleslevelRec> vrec;
		summarySampleslevelHeader sh;
		while (i != 0) {			
			i = fread(&sh, sizeof(sh), 1, stdin);			
			while (i != 0) {
				sampleslevelRec sr;
				i = fread(&sr, sizeof(sr), 1, stdin);
				if (i == 0 || sr.sidx == 0) {
					dopltcalc(sh,vrec);
					vrec.clear();
					break;
				}
				vrec.push_back(sr);
			}
			
			j++;
		}
				
	}
}

int main(int argc, char* argv[])
{
	initstreams();
	doit();
	
}
