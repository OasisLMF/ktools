#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../include/oasis.hpp"

typedef unsigned char byte;
int __exit(int status){ exit(status); return 1; }
int __perror(const char * m) { perror(m); return 1; }
int __close(int fd) { close(fd); return 1; }


// TO DO
// add TIV to TCVal for Functions 4 & 6


struct fmdata {
    int item_id;
    int agg_id;
    int prog_id;
    int level_id;
    int policytc_id;
    int layer_id;
    int calcrule_id;
    int allocrule_id;
    float deductible;
    float limits;
    float share_prop_of_lim;
    float deductible_prop_of_loss;
    float limit_prop_of_loss;
    float deductible_prop_of_tiv;
    float limit_prop_of_tiv;
    float deductible_prop_of_limit;
};


struct AggKey {
	int prog_id;
	int agg_id;
	int layer_id;
};
inline bool operator==(const AggKey& lhs, const AggKey& rhs){ return lhs.prog_id == rhs.prog_id && lhs.agg_id == rhs.agg_id && lhs.layer_id == rhs.layer_id; }
inline bool operator< (const AggKey& lhs, const AggKey& rhs){  if (lhs.prog_id < rhs.prog_id) return true; if( lhs.prog_id == rhs.prog_id && lhs.agg_id < rhs.agg_id) return true; if( lhs.prog_id == rhs.agg_id && lhs.agg_id == rhs.agg_id && lhs.layer_id < rhs.layer_id) return true; return false; }


struct TCVal {
	int calcrule_id;
	int allocrule_id;
	float deductible;
	float limit;
	float share_prop_of_lim;
    float deductible_prop_of_loss;
    float limit_prop_of_loss;
    float deductible_prop_of_tiv;
    float limit_prop_of_tiv;
    float deductible_prop_of_limit;
};

int doFM(int event_id_, std::vector<gulSampleslevel> &event_guls_, byte *fmd, int numFMDs, int numSamples)
{
	std::map<AggKey, std::vector<int> > aggToOffsetVector;
	std::map<AggKey, int > levelMap;
	std::map<AggKey, int> aggMap;
	int myLevel=1;

	std::map<int, int> itemToOffsetMap;
	std::map<int, int> offsetToItemMap;
	
	int numGuls = event_guls_.size();

	
	// set up the item to offset map and offset to item map - we need to do this for each event, as a small fraction of the items will be impacted by some of the event
	int k=0;
	for(int i=0; i< numGuls; i++)
	{
		if ( event_guls_[i].sidx > 0 )
			continue;
		bool found = itemToOffsetMap.count(event_guls_[i].item_id);
		if ( not found ) 
		{ 
			itemToOffsetMap[event_guls_[i].item_id]=k; 
			offsetToItemMap[k] = event_guls_[i].item_id;
			k+=sizeof(float);
		}
	}
	int numItems = itemToOffsetMap.size();

	// allocate space for the levelLossWork table and initialise to zero
	byte *levelLossWork = (byte *)calloc( 1, itemToOffsetMap.size() * (numSamples+1) * sizeof(float) );		
	{	levelLossWork || __perror("Error allocating levelLossWork table :") && __exit(EXIT_FAILURE);	}
		
	// populate the levelLossWork table
	for(int i=0; i<numGuls; i++)
		*(float *)(levelLossWork + sizeof(float) * ( itemToOffsetMap.size() * (event_guls_[i].sidx)  ) + itemToOffsetMap[event_guls_[i].item_id] ) = event_guls_[i].gul;
	
	// allocate space for the groundUpLossWork table and initialise to zero
	byte *groundUpLossWork = (byte *)calloc( 1, itemToOffsetMap.size() * (numSamples+1) * sizeof(float) );		
	{	groundUpLossWork || __perror("Error allocating levelLossWork table :") && __exit(EXIT_FAILURE);	}
		
	// populate the groundUpLossWork table
	for(int i=0; i<numGuls; i++)
		*(float *)(groundUpLossWork + sizeof(float) * ( itemToOffsetMap.size() * (event_guls_[i].sidx)  ) + itemToOffsetMap[event_guls_[i].item_id] ) = event_guls_[i].gul;

	// allocate space for the outputLossWork table and initialise to zero
	byte *outputLossWork = (byte *)calloc( 1, itemToOffsetMap.size() * (numSamples+1) * sizeof(float) );		
	{	levelLossWork || __perror("Error allocating outputLossWork table :") && __exit(EXIT_FAILURE);	}

	// determine the maximum level
	int maxLevel = 1;
	while (1) 
	{
		bool foundLevel=false;
		for( byte *p=(byte *)fmd; p< fmd + numFMDs*sizeof(fmdata); p+=sizeof(fmdata))
		{
			fmdata *pfmd = (fmdata *)p;
			if (pfmd->level_id==maxLevel)
			{
				foundLevel=true;
				break;
			}
		}
		if (foundLevel)
			maxLevel++; 
		else
		{
			maxLevel--; // we didn't find that value of maxlevel, but we did find the one below it
			break;
		}
	}

	// determine max layers in the final level
	int maxLayer = 1;
	for( byte *p=(byte *)fmd; p < (byte *)fmd + numFMDs*sizeof(fmdata); p+=sizeof(fmdata))
	{
		fmdata *pfmd = (fmdata *)p;
		if( pfmd->level_id == maxLevel && pfmd->layer_id > maxLayer )
			maxLayer = pfmd->layer_id;
	}

	for ( myLevel=1; myLevel<=maxLevel; myLevel++)
	{
		// set up item Index vector for each agg_id - a list of item indices to sum over for that agg_id
		// also set up the aggregation index table and the T&C Map for the level
		// note that not all Items aggregated need necessarily have an item for a particular event
		std::map<AggKey, TCVal> TCMap;

		int aggCount=0;
		for( byte *p=(byte *)fmd; p< fmd + numFMDs*sizeof(fmdata); p+=sizeof(fmdata))
		{
			fmdata *pfmd = (fmdata *)p;
			if( pfmd->level_id==myLevel )
			{
				if ( itemToOffsetMap.count(pfmd->item_id) ==1 )
				{
					AggKey aggKey = { pfmd->prog_id, pfmd->agg_id, pfmd->layer_id };
					TCVal tcv = { pfmd->calcrule_id, pfmd->allocrule_id, pfmd->deductible, pfmd->limits, pfmd->share_prop_of_lim, pfmd->deductible_prop_of_loss, pfmd->limit_prop_of_loss, pfmd->deductible_prop_of_tiv, pfmd->limit_prop_of_tiv, pfmd->deductible_prop_of_limit};
					TCMap[ aggKey ] = TCVal(tcv);

					if (pfmd->allocrule_id == -1)
					{
						levelMap[aggKey] = itemToOffsetMap[pfmd->item_id];
					}
					else
					{
						if ( aggToOffsetVector.count(aggKey)==0  )
						{
							aggMap[aggKey] = aggCount;
							aggCount+=sizeof(float);
						}
						aggToOffsetVector[aggKey].push_back( itemToOffsetMap[pfmd->item_id] );
					}
				}
			}
		}

		// set up aggWork table to hold the aggregations and initialise to zero
		byte *aggLevelLossWork = (byte *)calloc(	1, sizeof(float) * aggMap.size() * (numSamples+1) );	
		{ fmd || __perror("Error opening fm data file:" ) && __exit(EXIT_FAILURE); }
		byte *aggGroundUpLossWork = (byte *)calloc(	1, sizeof(float) * aggMap.size() * (numSamples+1) );	
		{ fmd || __perror("Error opening fm data file:" ) && __exit(EXIT_FAILURE); }
		

		// Calculate the Aggregations
		for ( std::map<AggKey, std::vector<int> >::iterator j = aggToOffsetVector.begin(); j!=aggToOffsetVector.end(); j++)
		{
			if ( TCMap.count(j->first)!=1)
				exit(EXIT_FAILURE);
			TCVal tc = TCMap[j->first];

			if ( tc.allocrule_id == -1 ) // only aggregate losses if there is more than one item in the aggregation
				continue;

			for(int i=0; i<= numSamples; i++)
			{
				byte *levelLossWorkRow = levelLossWork + sizeof(float) * ( i* itemToOffsetMap.size() );
				byte *aggLevelLossWorkRow = aggLevelLossWork +  sizeof(float) * ( aggMap.size() * i );

				std::vector<int> indexVector = j->second;

				// aggregate the level losses
				float levelLossSum=0.0;
				for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
				{
					levelLossSum += *(float *)(levelLossWorkRow + *k );
				}
				*(float *)( aggLevelLossWorkRow + aggMap[j->first]  ) = levelLossSum;

				// aggregate the GULs if needed
				if ( tc.allocrule_id == 1 or tc.allocrule_id ==3 or tc.calcrule_id == 8 or tc.calcrule_id == 10  or tc.calcrule_id == 11 ) 
				{
					float groundUpLossSum=0.0;
					byte *groundUpLossWorkRow = groundUpLossWork + sizeof(float) * ( i* itemToOffsetMap.size() );
					byte *aggGroundUpLossWorkRow = aggGroundUpLossWork +  sizeof(float) * ( aggMap.size() * i );
					for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
					{
						groundUpLossSum += *(float *)(groundUpLossWorkRow + *k );
					}
					*(float *)( aggGroundUpLossWorkRow + aggMap[j->first]  ) = groundUpLossSum;
				}
			}
		}
	
		// calculate losses from the aggregations and back allocate
		for(int layer=1; layer<=( myLevel==maxLevel ? maxLayer : 1); layer++) // one level only unless we are at the final level - ie maxLevel
		{
			for(int i=0; i<=numSamples; i++)
			{
				byte *levelLossWorkRow = levelLossWork + sizeof(float) * i * itemToOffsetMap.size();
				byte *groundUpLossWorkRow = groundUpLossWork + sizeof(float) * i* itemToOffsetMap.size();
				byte *aggLevelLossWorkRow = aggLevelLossWork +  sizeof(float) * aggMap.size() * i;
				byte *aggGroundUpLossWorkRow = aggGroundUpLossWork +  sizeof(float) * aggMap.size() * i;
				byte *outputLossWorkRow = outputLossWork +  sizeof(float) * itemToOffsetMap.size() * i;

				for(   std::map<AggKey, TCVal>::iterator j=TCMap.begin(); j!=TCMap.end(); j++)
				{
					// only work on the items for this layer
					if ( (j->first).layer_id != layer)
						continue;

					TCVal tc = j->second; // TCMap[j->first];

					float totalLevelLoss = 0.0;
					float totalGroundUpLoss = 0.0;
					float totalRetainedLoss = 0.0;
					std::vector<int> indexVector = aggToOffsetVector[j->first];
					
					float loss, gul;
					if ( tc.allocrule_id==-1 )
					{
						int itemOffset = levelMap[j->first];
						loss = *(float *)(levelLossWorkRow + itemOffset);
						gul = *(float *)(groundUpLossWorkRow + itemOffset );
					}
					else							// otherwise, take it from the aggregation table
					{
						loss = *(float *)(aggLevelLossWorkRow + aggMap[j->first] );
						gul = *(float *)(aggGroundUpLossWorkRow + aggMap[j->first] );
					}
					

					// only do a financial calculation if we have a single T&C term for this aggregation for this layer
					// note that TCMap is set up for each specific level - see above

					// Financial Calculation
					if (tc.calcrule_id == 1)
					{
						//Function1 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded))
						if ( loss < tc.deductible )
							loss = 0.0;
						else
							if ( loss > tc.deductible + tc.limit )
								loss = tc.limit;
							else
								loss -= tc.deductible;
					}
					else if (tc.calcrule_id == 2)
					{
						//Function2 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss - Ded)) * Share
						if ( loss < tc.deductible )
							loss = 0.0;
						else
							if ( loss > tc.deductible + tc.limit )
								loss = tc.limit;
							else
								loss -= tc.deductible;
						loss *= tc.share_prop_of_lim;
					}
					else if (tc.calcrule_id == 3)
					{
						//Function3 = IIf(Loss < Ded, 0, IIf(Loss > Ded + Lim, Lim, Loss))
						if ( loss < tc.deductible )
							loss = 0.0;
						else
							if ( loss > tc.deductible + tc.limit )
								loss = tc.limit;
							//else loss is unchanged
					}
					/*
					if (tc.calcrule_id == 4)
					{
						//Function4 = IIf(Loss < (Ded * TIV), 0, IIf(Loss > TIV * (Ded + Lim), TIV * Lim, Loss - (TIV * Ded)))
						if ( loss < tc.deductible * tc.tiv )
							loss = 0.0;
						else
							if ( loss > ( tc.deductible + tc.limit) * tc.tiv )
								loss = tc.limit * tc.tiv;
							else
								loss -= ( tc.deductible * tc.tiv );
					}
					*/
					else if (tc.calcrule_id == 5)
					{
						//Function5 = Loss * (Lim - Ded)
						loss = loss * (tc.limit - tc.deductible);
					}
					/*
					if (tc.calcrule_id == 6)
					{
						//Function6 = IIf(Loss < (Ded * TIV), 0, Loss - (TIV * Ded))
						if ( loss < tc.deductible * tc.tiv )
							loss = 0.0;
						else
							loss -= tc.deductible * tc.tiv;
					}
					*/
					else if (tc.calcrule_id == 8)
					{
						// Takes the difference between Ground up loss and input loss to calculate the retained loss from the previous calculation, and then applies a floor (Ded) on retained loss
						// ED = IIf(GUL - Loss < 0, 0, IIf(GUL - Loss < Ded, Ded, GUL - Loss))
						float retainedLoss = gul - loss;
						float ED = retainedLoss < 0.0 ? 0.0 : ( retainedLoss < tc.deductible ? tc.deductible : retainedLoss );
						// Takes the effective deductible from the GUL
						// NetED = IIf(GUL - ED < 0, 0, GUL - ED)
						float lossNetOfED = gul - ED < 0.0 ? 0.0 : gul - ED;
						// Applies the limit
						// Function8 = IIf(NetED > Lim, Lim, NetED)
						loss = lossNetOfED > tc.limit ? tc.limit : lossNetOfED;
					}
					else if (tc.calcrule_id == 10)
					{
						// Takes the difference between Ground up loss and input loss to calculate the retained loss from the previous calculation, and then applies a cap (Ded) on retained loss
						// ED = IIf(GUL - Loss < 0, 0, IIf(GUL - Loss < Ded, Ded, GUL - Loss))
						float retainedLoss = gul - loss;
						float ED = retainedLoss < 0.0 ? 0.0 : ( retainedLoss < tc.deductible ? tc.deductible : retainedLoss );
						// Takes the effective deductible from the GUL
						// Function10 = IIf(GUL - ED < 0, 0, GUL - ED)
						loss = gul - ED < 0.0 ? 0.0 : gul - ED;
					}
					else if (tc.calcrule_id == 11)
					{
						// Takes the difference between Ground up loss and input loss to calculate the retained loss from the previous calculation, and then applies a floor (Ded) on retained loss
						// ED = IIf(GUL - Loss < 0, 0, IIf(GUL - Loss > Ded, Ded, GUL - Loss))
						float retainedLoss = gul - loss;
						float ED = retainedLoss < 0.0 ? 0.0 : ( retainedLoss > tc.deductible ? tc.deductible : retainedLoss );
						// Takes the effective deductible from the GUL
						// Function11 = IIf(GUL - ED < 0, 0, GUL - ED)
						loss = gul - ED < 0.0 ? 0.0 : gul - ED;
					}
					else if (tc.calcrule_id == 12)
					{
						// Function12 = IIf(Loss < Ded, 0, Loss - Ded)
						loss -= tc.deductible;
						loss = loss < 0.0 ? 0.0 : loss;
					}
					/*
					if (tc.calcrule_id == 13)
					{
						// Function13 = IIf(Loss < UndCovAmt, 0, IIf(Loss > UndCovAmt + Partof, Partof, Loss - UndCovAmt)) * LimAmt
						loss = ( loss < tc.undCovAmt ? 0.0 : ( loss > tc.undCovAmt + tc.partOf ? tvc.partOf : loss - tc.undCovAmt ) ) * tc.limAmt;
					}
					*/
					else
						__perror("unrecognised calcrule_id") || __exit(EXIT_FAILURE);	
					
					
					// Back Allocation
					// if there is only one element in the aggregation, put the result straight back into levelLossWork - no need for aggregation and back allocation
					if ( tc.allocrule_id==-1 )
					{
						int itemOffset = levelMap[j->first];
						*(float *)(levelLossWorkRow + itemOffset ) = loss;
					}
					// no back allocation - write the result to the allocation table to await being output
					// *(float *)( aggLevelLossWorkRow + aggMap[std::pair<int, int>(aggKey.prog_id, aggKey.agg_id)] ) = loss;
					else if ( tc.allocrule_id==0 )								
					{
						*(float *)( aggLevelLossWorkRow + aggMap[j->first] ) = loss;
					}
					// back-allocate the losses in proportion to the GULs ie proportion = ground up loss / sum of ground up losses
					else if ( tc.allocrule_id==1 ) 
					{
						totalGroundUpLoss = *(float *)( aggGroundUpLossWorkRow + aggMap[j->first]  );
						for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
						{
							float prop = totalGroundUpLoss > 0.0 ? *(float *)(groundUpLossWorkRow + *k) / totalGroundUpLoss : 0.0;
							float allocatedLoss = loss * prop;
							// use the outputLossWork table if we are at the max level, so that one layer's losses doesn't over-write the previous level's losses needed to compute the other layers
							if (myLevel==maxLevel)
								*(float *)( outputLossWorkRow + *k ) = allocatedLoss;
							else
								*(float *)( levelLossWorkRow + *k ) = allocatedLoss;
						}
					}
					// back allocate in proportion to the loss in the previous level 
					// ie proportion = loss in previous level / sum of the losses in the previous level
					// NB 
					else if ( tc.allocrule_id==2 ) 
					{
							totalLevelLoss = *(float *)(aggLevelLossWorkRow + aggMap[j->first] );
							for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
							{
								float prop = totalLevelLoss > 0.0 ? *(float *)(levelLossWorkRow + *k) / totalLevelLoss : 0.0;
								float allocatedLoss = loss * prop;
								// use the outputLossWork table if we are at the max level, so that one layer's losses doesn't over-write the previous level's losses needed to compute the other layers
								if (myLevel==maxLevel)
									*(float *)( outputLossWorkRow + *k ) = allocatedLoss;
								else
									*(float *)( levelLossWorkRow + *k ) = allocatedLoss;
							}
					}
					// back allocate in proportion to the retained loss in the previous level
					// ie proportion = retained loss in previous level / sum of the retained losses in the previous level 
					// However, if the totalRetainedLoss is zero (eg because it's the first level), allocate back according to the GULs.
					else if ( tc.allocrule_id==3 )
					{
						totalRetainedLoss = *(float *)( aggGroundUpLossWorkRow + aggMap[j->first]  ) - *(float *)(aggLevelLossWorkRow + aggMap[j->first] );
						if ( totalRetainedLoss == 0.0 )
						{
							totalGroundUpLoss = *(float *)( aggGroundUpLossWorkRow + aggMap[j->first]  );
							for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
							{
								float prop = totalGroundUpLoss > 0.0 ? *(float *)(groundUpLossWorkRow + *k) / totalGroundUpLoss : 0.0;
								float allocatedLoss = loss * prop;
								// use the outputLossWork table if we are at the max level, so that one layer's losses doesn't over-write the previous level's losses needed to compute the other layers
								if (myLevel==maxLevel)
									*(float *)( outputLossWorkRow + *k ) = allocatedLoss;
								else
									*(float *)( levelLossWorkRow + *k ) = allocatedLoss;
							}
						}
						else
						{
							for ( std::vector<int>::iterator k = indexVector.begin(); k!=indexVector.end(); k++)
							{
								float prop = totalRetainedLoss > 0.0 ? ( *(float *)(groundUpLossWorkRow + *k) - *(float *)(levelLossWorkRow + *k) ) / totalRetainedLoss : 0.0;
								float allocatedLoss = loss * prop;
								if (myLevel==maxLevel)
									*(float *)( outputLossWorkRow + *k ) = allocatedLoss;
								else
									*(float *)( levelLossWorkRow + *k ) = allocatedLoss;
							}
						}
					}
					else
						__perror("unrecognised allocrule_id") || __exit(EXIT_FAILURE);	
				}
			}
			// fprintf(stderr, "doFM 7.4.2\n");
			// we now have the data in the losswork and/or agg table for all samples for this layer, so we can do output
			if (myLevel == maxLevel)
			{
				fmlevelhdr fmhd;
				fmlevelrec fmrec;
				for(   std::map<AggKey, TCVal>::iterator j=TCMap.begin(); j!=TCMap.end(); j++)
				{
					AggKey aggKey = j->first;
					if ( aggKey.layer_id!=layer)
						continue;
					if ( TCMap.count(aggKey)!=1)
						continue;
					TCVal tc = TCMap[aggKey];

					if ( tc.allocrule_id == 0 ) // output from the aggregation table
					{
						fmhd.event_id = event_id_;
						fmhd.prog_id = aggKey.prog_id;
						fmhd.layer_id = aggKey.layer_id;
						fmhd.output_id = aggKey.agg_id;
						fwrite(&fmhd, sizeof(fmlevelhdr), 1, stdout);
						for(int i=0; i<=numSamples; i++)
						{
							byte *aggLevelLossWorkRow = aggLevelLossWork +  sizeof(float) * aggMap.size() * i;

							if (i==0) 
								fmrec.sidx = mean_idx;
							else
								fmrec.sidx = i;
							fmrec.loss= *(float *)( aggLevelLossWorkRow + aggMap[aggKey] );
							fwrite( &fmrec, sizeof(fmrec), 1, stdout);
						}
						fmrec.sidx = 0;
						fmrec.loss = 0.0;
						fwrite( &fmrec, sizeof(fmrec), 1, stdout);
					}
					else if ( tc.allocrule_id == 1 || tc.allocrule_id == 2 || tc.allocrule_id == 3 ) // output from the levelLoss table
					{
						for( std::vector<int>::iterator k=aggToOffsetVector[aggKey].begin(); k!=aggToOffsetVector[aggKey].end(); k++)
						{
							fmhd.event_id = event_id_;
							fmhd.prog_id = aggKey.prog_id;
							fmhd.layer_id = aggKey.layer_id;
							fmhd.output_id = offsetToItemMap[*k];
							fwrite(&fmhd, sizeof(fmlevelhdr), 1, stdout);
							
							// *k is the offset of the item
							for(int i=0; i<=numSamples; i++)
							{
								byte *outputLossWorkRow = outputLossWork + sizeof(float) * i * itemToOffsetMap.size();

								if (i==0) 
									fmrec.sidx = mean_idx;
								else
									fmrec.sidx = i;
								fmrec.loss= *(float *)(outputLossWorkRow + *k);
								fwrite( &fmrec, sizeof(fmrec), 1, stdout);
							}
							fmrec.sidx = 0;
							fmrec.loss = 0.0;
							fwrite( &fmrec, sizeof(fmrec), 1, stdout);
						}
					}  // 
					else if ( tc.allocrule_id == -1 )
					{
						int itemOffset = levelMap[j->first];

						fmhd.event_id = event_id_;
						fmhd.prog_id = aggKey.prog_id;
						fmhd.layer_id = aggKey.layer_id;
						fmhd.output_id = offsetToItemMap[itemOffset];
						fwrite(&fmhd, sizeof(fmlevelhdr), 1, stdout);

						for(int i=0; i<=numSamples; i++)
						{
							byte *levelLossWorkRow = levelLossWork + sizeof(float) * i * itemToOffsetMap.size();
							if (i==0) 
								fmrec.sidx = mean_idx;
							else
								fmrec.sidx = i;
							fmrec.loss = *(float *)(levelLossWorkRow + itemOffset);
							fwrite( &fmrec, sizeof(fmrec), 1, stdout);
						}
						fmrec.sidx = 0;
						fmrec.loss = 0.0;
						fwrite( &fmrec, sizeof(fmrec), 1, stdout);
					}
				}
			}
		}	
		free(aggLevelLossWork);
		free(aggGroundUpLossWork);
		aggMap.clear();
		levelMap.clear();
		aggToOffsetVector.clear();
	}
	free(levelLossWork);
	free(outputLossWork);
	free(groundUpLossWork);
}


int main()
{
	// read in the FM data
	FILE *fin = (FILE *)fopen("fm/fm_data.bin", "rb");	
	{ fin || __perror("Error opening fm data file:" ) && __exit(EXIT_FAILURE); }
    fseek (fin, 0, SEEK_END);   // non-portable ???
    size_t fmdFileSize=ftell (fin);
	fclose(fin);

    int fd = open("fm/fm_data.bin", O_RDONLY);
    { fd == -1 && __perror("Error opening file for reading") && __exit(EXIT_FAILURE); }

    byte *fmd = (byte *)mmap(0, fmdFileSize, PROT_READ, MAP_SHARED, fd, 0);
    { ( fmd == MAP_FAILED ) && __perror("Error mmapping the file") && __close(fd) && __exit(EXIT_FAILURE); }

	freopen(NULL, "rb", stdin);
	freopen(NULL, "wb", stdout);
	
	unsigned int fmstream_type = 1 | fmstream_id;
	fwrite(&fmstream_type, sizeof(fmstream_type), 1, stdout);

	int gulstream_type = 0;
	int i = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);
	int stream_type = gulstream_type & gulstream_id ;
	{ stream_type != gulstream_id && fprintf(stderr, "Not a gul stream type\n") && __exit(EXIT_FAILURE); }
	stream_type = streamno_mask &gulstream_type;
	{ stream_type != 1 && fprintf(stderr, "Unsupported gul stream type\n") && __exit(EXIT_FAILURE); }

	int last_event_id = -1;
	std::vector<gulSampleslevel> event_guls;
	if (stream_type == 1) {
		int samplesize = 0;
		fread(&samplesize, sizeof(samplesize), 1, stdin);
		fwrite(&samplesize, sizeof(samplesize), 1, stdout);		
		while (i != 0){
			gulSampleslevelHeader gh;
			i = fread(&gh, sizeof(gh), 1, stdin);
			if (gh.event_id != last_event_id ) 
			{
				if (last_event_id != -1) 
				{
					doFM(last_event_id, event_guls, fmd, fmdFileSize/sizeof(fmdata), samplesize);
				}
				event_guls.clear();
				last_event_id = gh.event_id;
			}
			while (i != 0)
			{
				gulSampleslevelRec gr;
				i = fread(&gr, sizeof(gr), 1, stdin);
				if (i == 0) 
				{
					// reached the end of the file, so process the last event before we break from this loop and terminate the higher level loopm(i==0)
					doFM(last_event_id, event_guls, fmd, fmdFileSize/sizeof(fmdata), samplesize);
					break;
				}
				if (gr.sidx == 0) break; // this marks  the start of a new event, so process the one we have just read
				if (gr.sidx == mean_idx) gr.sidx = 0;
				if (gr.sidx < 0) continue;
				gulSampleslevel gs;
				gs.event_id = gh.event_id;
				gs.item_id = gh.item_id;
				gs.sidx = gr.sidx;
				gs.gul = gr.gul;
				
				event_guls.push_back(gs);			
			}
		}
	}

	if (munmap(fmd, fmdFileSize) == -1) 
		perror("Error un-mmapping the file");
    close(fd);
}
