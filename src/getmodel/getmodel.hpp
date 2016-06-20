#include <map>
#include <set>
#include <list>
#include <vector>

struct Result;
struct EventIndex;

class getmodel {

public:

    getmodel();
    ~getmodel();
    void init();
	void doCdf(std::list<int> event_ids);

private:
	
	std::map<int, std::vector<float>> _vulnerabilities;	
	std::map<int, std::set<int>> _vulnerability_ids_by_area_peril;
	std::set<int> _area_perils;
	std::vector<float> _mean_damage_bins;
    int _num_intensity_bins = -1;
    int _num_damage_bins = -1;
    int _has_intensity_uncertainty = false;
    Result* _temp_results;

    void getVulnerabilities();
    void getExposures();
    void getDamageBinDictionary();
    void getItems();
	void getIntensityInfo();
    void doCdfInner(std::list<int> &event_ids, std::map<int, EventIndex> &event_index_by_event_id);
    void doCdfInnerNoIntensityUncertainty(std::list<int> &event_ids, std::map<int, EventIndex> &event_index_by_event_id);

    void  doResults(
        int &event_id,
        int &areaperil_id,
        std::map<int, std::set<int>> &vulnerabilities_by_area_peril,
        std::map<int, std::vector<float>> &vulnerabilities,
        std::vector<float> intensity) const;

	void  doResultsNoIntensityUncertainty(
		int &event_id,
		int &areaperil_id,
		std::map<int, std::set<int>> &vulnerabilities_by_area_peril,
		std::map<int, std::vector<float>> &vulnerabilities,
		int intensity_bin_index) const;
	
	static void initOutputStream();
    int getVulnerabilityIndex(int intensity_bin_index, int damage_bin_index) const;
};
