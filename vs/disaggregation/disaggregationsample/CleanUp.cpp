/*

void getAggAPWeights(AREAPERIL_INT &aggregate_areaperil_id, int vulnerability_id,
	std::map<AggregateID, std::vector<Weight>> &_agg_ap_map_weights);
void getAggVulWeights(AREAPERIL_INT &areaperil_id, int aggregate_vulnerability_id,
	std::map<AggregateID, std::vector<Weight>> &_agg_vul_map_weights);
void getAggAPandVulWeights(AREAPERIL_INT &aggregate_areaperil_id, int aggregate_vulnerability_id,
	std::map<AggregateID, std::vector<Weight>> &_agg_map_weights);




void disaggregation::getAggAPWeights(AREAPERIL_INT &aggregate_areaperil_id, int vulnerability_id,
	map<AggregateID, vector<Weight>> &_agg_ap_map_weights) {
	vector<Weight> weights;
	auto ap_probs = _aggregate_areaperils[aggregate_areaperil_id];
	for (AREAPERIL_INT c = 0; c < ap_probs.size(); ++c) {
		if (ap_probs[c] != 0) {
			weights.push_back(Weight((c + 1), vulnerability_id, ap_probs[c]));
		}
	}
	AggregateID agg_id(aggregate_areaperil_id, vulnerability_id);
	_agg_ap_map_weights[agg_id] = weights;
}

void disaggregation::getAggVulWeights(AREAPERIL_INT &areaperil_id, int aggregate_vulnerability_id,
	map<AggregateID, vector<Weight>> &_agg_vul_map_weights) {
	vector<Weight> weights;
	auto vul_probs = _aggregate_vulnerabilities[aggregate_vulnerability_id][areaperil_id];
	for (auto it = vul_probs.begin(); it != vul_probs.end(); ++it) {
		if (it->second != 0) {
			weights.push_back(Weight(areaperil_id, it->first, it->second));
		}
	}
	AggregateID agg_id(areaperil_id, aggregate_vulnerability_id);
	_agg_vul_map_weights[agg_id] = weights;
}



void disaggregation::getAggAPandVulWeights(AREAPERIL_INT &aggregate_areaperil_id, int aggregate_vulnerability_id,
	map<AggregateID, vector<Weight>> &_agg_map_weights) {
	vector<Weight> weights;
	auto ap_probs = _aggregate_areaperils[aggregate_areaperil_id];
	for (AREAPERIL_INT c = 0; c < ap_probs.size(); ++c) {
		if (ap_probs[c] != 0) {
			auto vul_probs = _aggregate_vulnerabilities[aggregate_vulnerability_id][c + 1];
			for (auto it = vul_probs.begin(); it != vul_probs.end(); ++it) {
				if (it->second != 0) {
					weights.push_back(Weight((c + 1), it->first, it->second*ap_probs[c]));
				}
			}
		}
	}
	AggregateID agg_id(aggregate_areaperil_id, aggregate_vulnerability_id);
	_agg_map_weights[agg_id] = weights;
}

if (a.vulnerability_id) {
	if (a.areaperil_id) {
		AggregateID agg_id(a.areaperil_id, a.vulnerability_id);
		Weight weight(a.areaperil_id, a.vulnerability_id, 1);
		std::vector<Weight> weights;
		weights.push_back(weight);
		_agg_ids_map_weights[agg_id] = weights;
	}
	else {
		getAggAPWeights(a.aggregate_areaperil_id, a.vulnerability_id, _agg_ids_map_weights);
	}
}
else {
	if (a.areaperil_id) {
		getAggVulWeights(a.areaperil_id, a.aggregate_vulnerability_id, _agg_ids_map_weights);
	}
	else {
		getAggAPandVulWeights(a.aggregate_areaperil_id, a.aggregate_vulnerability_id, _agg_ids_map_weights);
	}
}
	}

	*/