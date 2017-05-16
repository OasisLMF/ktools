/*
* Copyright (c)2015 - 2016 Oasis LMF Limited
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.
*
*   * Neither the original author of this software nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
* THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*/
/*
Convert old footprint format file to compressed format
Author: Ben Matharu  email: ben.matharu@oasislmf.org
*/



#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <zlib.h>
#include "../include/oasis.h"

void doit()
{
    FILE *finx = fopen("footprint.bin", "rb");
	FILE *finy = fopen("footprint.idx", "rb");
    if (finx == NULL) {
        fprintf(stderr,"File not found footprint.bin\n");
        exit(-1);
    }
    if (finy == NULL) {
        fprintf(stderr,"File not found footprint.idx\n");
        exit(-1);
    }


    FILE *foutx = fopen("footprint.bin.z", "wb");
    FILE *fouty = fopen("footprint.idx.z", "wb");
    int intensity_bins = 0;
    int hasIntensityUncertainty =0;
    fread(&intensity_bins, sizeof(intensity_bins), 1, finx);
    fread(&hasIntensityUncertainty, sizeof(hasIntensityUncertainty), 1, finx);
    fprintf(stderr,"intensity_bins: %d hasIntensityUncertainty: %d\n",intensity_bins,hasIntensityUncertainty);
    fwrite(&intensity_bins, sizeof(intensity_bins), 1, foutx);
    fwrite(&hasIntensityUncertainty, sizeof(hasIntensityUncertainty), 1, foutx);

    EventIndex in_idx;
    EventIndex out_idx;
    size_t i = fread(&in_idx, sizeof(in_idx), 1, finy);
    std::vector<EventRow> rv;
    std::vector<unsigned char> rvz;
    int total_insize = 0;
    int total_outsize = 0;
    out_idx.offset = 8;
    while (i != 0) {
		flseek(finx, in_idx.offset, SEEK_SET);                
        rv.resize(in_idx.size);
        total_insize += in_idx.size;
        fread(&rv[0], sizeof(1), in_idx.size,finx);
        rvz.clear();
        rvz.resize(rv.size() * sizeof(EventRow) + 1024);
        unsigned long len = rvz.size();
        compress(&rvz[0], &len, (unsigned char *)&rv[0], in_idx.size);
        total_outsize += len;
        fwrite((unsigned char *)&rvz[0], len, 1, foutx);
        out_idx.event_id = in_idx.event_id;        
        out_idx.size = len;
        fwrite(&out_idx, sizeof(out_idx), 1, fouty);
        // fprintf(stderr,"%d, %ld, %d\n", in_idx.event_id, in_idx.offset, in_idx.size);
        // fprintf(stderr,"%d, %ld, %d\n", out_idx.event_id, out_idx.offset, out_idx.size);
    /*  
        long long j=0;  
	    while (j < in_idx.size) {
            EventRow row;
            fread(&row, sizeof(row), 1,finx);
            // printf("%d, %d, %d, %.10e\n", in_idx.event_id,row.areaperil_id,row.intensity_bin_id, row.probability);

            j += sizeof(row);
	    }
*/
        out_idx.offset += len;;                
        rv.clear();
        i = fread(&in_idx, sizeof(in_idx), 1, finy);
    }

    printf("TODO: expected input size  %d\n",total_insize+8);
    printf("TODO: expected output size  %d\n",total_outsize+8);
    fclose(foutx);
    fclose(fouty);
    fclose(finx);
    fclose(finy);
}
int main(int argc, char *argv[]) {
    initstreams();
    doit();    
}