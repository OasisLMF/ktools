#include <stdio.h>
#include <stdlib.h>
#include <set>

#include "../include/oasis.h"

// TODO: see https://docs.microsoft.com/en-us/cognitive-toolkit/setup-buildzlib-vs17

namespace fptofpcache {

	
	void getusedareaperils(std::set<int> &ap)
	{
		FILE *fin = fopen("input/items.bin", "rb");
		item q;
		int i = fread(&q, sizeof(q), 1, fin);
		while (i != 0) {
			ap.insert(q.areaperil_id);
			i = fread(&q, sizeof(q), 1, fin);
		}		
		fclose(fin);
	}
	
	void processrows(int event_id, FILE *finx, long long size, const std::set<int> &ap, FILE *foutx, int &out_size)
	{
		long long i = 0;
		out_size = 0;
		EventRow row;
		
		while (i < size) {			
			fread(&row, sizeof(row), 1, finx);
			const bool is_in = ap.find(row.areaperil_id) != ap.end();
			if (is_in == true) {
				fwrite(&row, sizeof(row), 1, foutx);
				out_size += sizeof(row);
			}

			i += sizeof(row);
		}

	}

	void doit()
	{

		FILE *finx = fopen("static/footprint.bin", "rb");
		if (finx == nullptr) {
			fprintf(stderr, "footprint.bin open failed\n");
			exit(EXIT_FAILURE);
		}
		FILE *finy = fopen("static/footprint.idx", "rb");
		if (finy == nullptr) {
			fprintf(stderr, "footprint.idx open failed\n");
			exit(EXIT_FAILURE);
		}

		int num_intensity_bins;
		int has_intensity_uncertainty;
		fread(&num_intensity_bins, sizeof(num_intensity_bins), 1, finx);
		fread(&has_intensity_uncertainty, sizeof(has_intensity_uncertainty), 1,finx);

		std::set<int> ap;
		getusedareaperils(ap);


		FILE *foutx = fopen("input/footprintcache.bin", "wb");
		FILE *fouty = fopen("input/footprintcache.idx", "wb");
		EventIndex idx_out;
		idx_out.event_id = 0;
		idx_out.size = 0;
		idx_out.offset = 0;

		fwrite(&num_intensity_bins, sizeof(num_intensity_bins), 1, foutx);
		idx_out.offset += sizeof(num_intensity_bins);
		fwrite(&has_intensity_uncertainty, sizeof(has_intensity_uncertainty), 1, foutx);
		idx_out.offset += sizeof(has_intensity_uncertainty);

		EventIndex idx_in;
		

		size_t i = fread(&idx_in, sizeof(idx_in), 1, finy);
		while (i != 0) {			
			idx_out.event_id = idx_in.event_id;
			int outsize;
			processrows(idx_in.event_id, finx, idx_in.size, ap,foutx,outsize);
			idx_out.size = outsize;
			fwrite(&idx_out, sizeof(idx_out), 1, fouty);	
			idx_out.offset = idx_out.offset + outsize;
			i = fread(&idx_in, sizeof(idx_in), 1, finy);
			flseek(finx, idx_in.offset, SEEK_SET);
		}

		fclose(foutx);
		fclose(fouty);
		fclose(finx);
		fclose(finy);		
	}
}
