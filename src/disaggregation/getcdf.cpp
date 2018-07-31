void getareaperilcdf(
	AREAPERIL_INT &agg_areaperil_id,
	std::map<AREAPERIL_INT, std::set<int>> &aggregate_to_ap,
	std::vector<OASIS_FLOAT> intensity) const {
	for (int area_peril_id : aggregate_to_ap[agg_areaperil_id]) {
		//Result *results = new Result[_num_damage_bins];
		std::vector<Result> results;
		results.resize(_num_damage_bins);
		int result_index = 0;

		int vulnerability_index = 0;
		double cumulative_prob = 0;
		for (int damage_bin_index = 0; damage_bin_index < _num_damage_bins;
			damage_bin_index++) {
			double prob = 0.0f;
			for (int intensity_bin_index = 0;
				intensity_bin_index < _num_intensity_bins; intensity_bin_index++) {
				prob += vulnerability[vulnerability_index++] *
					intensity[intensity_bin_index];
			}

			// if (prob > 0 || damage_bin_index == 0)
			//{
			cumulative_prob += prob;
			results[result_index++] = Result(static_cast<OASIS_FLOAT>(cumulative_prob),
				_mean_damage_bins[damage_bin_index]);
			//}
			if (cumulative_prob > 0.999999940)
				break; // single precision value approx 1
		}

		int num_results = result_index;
		fwrite(&event_id, SIZE_OF_INT, 1, stdout);
		fwrite(&areaperil_id, sizeof(areaperil_id), 1, stdout);
		fwrite(&vulnerability_id, SIZE_OF_INT, 1, stdout);
		fwrite(&num_results, SIZE_OF_INT, 1, stdout);
		fwrite(&results[0], SIZE_OF_RESULT, num_results, stdout);
		//delete[] results;
	}
}