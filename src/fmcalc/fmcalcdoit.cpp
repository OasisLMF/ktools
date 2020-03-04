#include "fmcalc.h"

void fmcalc::doit()
{
	// fmcalc fc(maxLevel, allocrule, inputpath, netvalue, oldFMProfile);

	isGULStreamType_ = true;
	// write_stream_type()
	// >>>>
	unsigned int fmstream_type = 1 | fmstream_id;

	fwrite(&fmstream_type, sizeof(fmstream_type), 1, stdout);
	// <<<<

	// read_gul_stream_type()
	// >>>>
	int gulstream_type = 0;
	size_t n_read = fread(&gulstream_type, sizeof(gulstream_type), 1, stdin);
	if (n_read == 0)
	{
		fprintf(stderr, "%s: Read error on input stream\n", __func__);
		exit(-1);
	}
	// <<<<

	// get_input_stream_type()
	// >>>>
	int stream_type = gulstream_type & gulstream_id;
	int stream_type2 = gulstream_type & fmstream_id;

	if (stream_type != gulstream_id && stream_type2 != fmstream_id)
	{
		fprintf(stderr, "%s: Not a gul stream type or fm stream type\n", __func__);
		exit(-1);
	}
	if (stream_type == gulstream_id)
	{
		stream_type = streamno_mask & gulstream_type;
	}
	if (stream_type2 == fmstream_id)
	{
		isGULStreamType_ = false;
		stream_type = streamno_mask & fmstream_type;
	}
	if (isGULStreamType_ == true)
		init_itemtotiv();
	if (stream_type != 1)
	{
		fprintf(stderr, "%s: Unsupported gul stream type %d\n", __func__, stream_type);
		exit(-1);
	}
	// <<<<

	int samplesize = 0;
	n_read = fread(&samplesize, sizeof(samplesize), 1, stdin);
	if (n_read != 1)
	{
		fprintf(stderr, "%s: fail to read number of samples\n", __func__);
		exit(-1);
	}

	fwrite(&samplesize, sizeof(samplesize), 1, stdout);
	std::vector<std::vector<OASIS_FLOAT>> event_guls(samplesize + 2); // one additional for mean and tiv
	std::vector<int> items;											  // item IDs for the current event

	int current_item_index = 0;

	int last_event_id = -1;
	bool end_of_stream = false;
	while (!end_of_stream) // for each event
	{
		// read_header()
		// >>>>
		gulSampleslevelHeader header;
		size_t i = fread(&header, sizeof(header), 1, stdin);
		if (i != 1 && ferror(stdin) != 0)
		{
			fprintf(stderr, "%s: fail to read samples level header\n", __func__);
			exit(-1);
		}
		// <<<<

		// compute latest event before end
		if (i != 1 && feof(stdin) != 0)
		{
			end_of_stream = true;
			if (last_event_id != -1)
			{
				dofm(last_event_id, items, event_guls);
			}
			break;
		}

		// next event
		if (header.event_id != last_event_id)
		{
			if (last_event_id != -1)
			{
				dofm(last_event_id, items, event_guls);
			}

			items.clear();
			for (unsigned int idx = 0; idx < event_guls.size(); idx++)
			{
				event_guls[idx].clear();
			}

			last_event_id = header.event_id;
		}

		while (!end_of_stream) // for each item of current event
		{
			// read_sample_loss()
			// >>>>
			gulSampleslevelRec record;
			i = fread(&record, sizeof(record), 1, stdin);
			if (i != 1 && ferror(stdin) != 0)
			{
				fprintf(stderr, "%s: fail to read samples level record\n", __func__);
				exit(-1);
			}
			// <<<<

			if (i != 1 && feof(stdin) != 0)
			{
				end_of_stream = true;
				dofm(last_event_id, items, event_guls);
				break;
			}

			// sample idx == 0 => marker for and of sample stream; dummy sample not used => next item follows
			if (record.sidx == 0)
			{
				break;
			}

			if (isGULStreamType_ == false)
			{
				if (record.sidx == tiv_idx)
				{
					items.push_back(header.item_id);
					for (unsigned int idx = 0; idx < event_guls.size(); idx++)
					{
						event_guls[idx].resize(items.size());
					}
					current_item_index = static_cast<int>(items.size() - 1);

					int sidx = 0;
					//event_guls[sidx].resize(items.size());
					event_guls[sidx][current_item_index] = record.loss;
				}

				if (record.sidx >= mean_idx)
				{
					int sidx = record.sidx + 1;
					if (record.sidx == mean_idx)
					{
						sidx = 1;
					}
					event_guls[sidx][current_item_index] = record.loss;
				}
			}
			else // isGULStreamType_ == true
			{
				if (record.sidx >= mean_idx)
				{
					if (record.sidx == mean_idx)
					{
						items.push_back(header.item_id);
						for (unsigned int idx = 0; idx < event_guls.size(); idx++)
						{
							event_guls[idx].resize(items.size());
						}
						current_item_index = static_cast<int>(items.size() - 1);
					}

					int sidx = record.sidx + 1;
					if (record.sidx == mean_idx)
					{
						sidx = 1;
					}
					event_guls[sidx][current_item_index] = record.loss;

					if (record.sidx == mean_idx)
					{ // add additional row for tiv
						sidx = 0;
						record.loss = gettiv(header.item_id);
						event_guls[sidx][current_item_index] = record.loss;
					}
				}
			}
		}
	}
}
