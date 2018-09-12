import pandas as pd

vulnerabilities = pd.read_csv("./data2/vulnerability2.csv", sep='\s*,\s*', engine='python')
footprints = pd.read_csv("./data2/footprint2.csv", sep='\s*,\s*', engine='python')
events_df = pd.read_csv("./data2/events2.csv", sep='\s*,\s*', engine='python')
damage_bin_dict = pd.read_csv("./data2/damage_bin_dict2.csv", sep='\s*,\s*', engine='python')

events = list(events_df['event_id'])

bin_means = list(damage_bin_dict['interpolation'])

areaperil_ids = [1000001, 2]
vulnerability_ids = [100009, 3000001]

disagg_areaperils=[1,2,100001,200001,400001,400002,600001,700001]
disagg_vulnerabilities=[6, 8, 9, 10, 100009, 300007, 500009, 500010, 500011, 600009]

num_damage_bins = 102
num_intensity_bins = 121

def getWeights(areaperil_id, vulnerability_id):
    weights = pd.read_csv("./data2/weights2.csv", sep='\s*,\s*', engine='python')
    weight_dict = {}
    total = 0
    if areaperil_id == 1000001 and vulnerability_id == 100009:
        rel_weights = weights[weights['vulnerability_id'].isin([100009])]
        rel_weights = rel_weights[rel_weights['areaperil_id'].isin(disagg_areaperils)]
        for i, row in rel_weights.iterrows():
            weight_dict[(int(row['areaperil_id']), 100009)] = row['count']
            total += row['count']

    elif areaperil_id == 2 and vulnerability_id == 3000001:
        rel_weights = weights.loc[weights['areaperil_id'].isin([2])]
        rel_weights = rel_weights[rel_weights['vulnerability_id'].isin(disagg_vulnerabilities)]
        for i, row in rel_weights.iterrows():
            weight_dict[(2, int(row['vulnerability_id']))] = row['count']
            total += row['count']

    elif areaperil_id == 1000001 and vulnerability_id == 3000001:
        rel_weights = weights.loc[weights['areaperil_id'].isin(disagg_areaperils)]
        rel_weights = rel_weights[rel_weights['vulnerability_id'].isin(disagg_vulnerabilities)]
        for i, row in rel_weights.iterrows():
            weight_dict[(int(row['areaperil_id']), int(row['vulnerability_id']))] = row['count']
            total += row['count']
    else:
        weight_dict[(areaperil_id, vulnerability_id)] = 1
        total = 1
    
    for weight in weight_dict:
        weight_dict[weight] /= total
    
    return weight_dict

def getVulnerability(vulnerability_id):
    rel_vul = vulnerabilities[vulnerabilities['vulnerability_id'].isin([vulnerability_id])]
    rel_vul.sort_values(by=['damage_bin_id', 'intensity_bin_id'],inplace=True)
    vulnerability = {}
    current_damage_bin = -1
    for i, row in rel_vul.iterrows():
        if row['damage_bin_id'] != current_damage_bin:
            vulnerability[row['damage_bin_id']] = []
            current_damage_bin = row['damage_bin_id']
        vulnerability[row['damage_bin_id']].append(row['probability'])
    return vulnerability

def getFootprint (event_id, areaperil_id):
    r_footprint = footprints[footprints['areaperil_id'].isin([areaperil_id])]
    rel_footprint = r_footprint[r_footprint['event_id'].isin([event_id])]
    footprint = []
    for i, row in rel_footprint.iterrows():
        footprint.append(row['probability'])
    return footprint

def getCdf (vulnerability, footprint):
    if footprint:
        cdf = []
        for damage_bin_id in range(num_damage_bins):
            cumulative_prob = 0.0
            for intensity_bin_id in range(num_intensity_bins):
                cumulative_prob += (vulnerability[damage_bin_id + 1][intensity_bin_id] * footprint[intensity_bin_id])
            cdf.append(cumulative_prob)
        return cdf
    else:
        return None

def getDisaggregateCdfs(event_id):
    attribute_cdfs = {}
    for vulnerability_id in disagg_vulnerabilities:
        vulnerability=getVulnerability(vulnerability_id)
        for areaperil_id in disagg_areaperils:
            footprint=getFootprint(event_id, areaperil_id)
            cdf = getCdf(vulnerability, footprint)
            if cdf:
                attribute_cdfs[(areaperil_id, vulnerability_id)] = cdf
    return attribute_cdfs


def getAggregateCdf(attribute_cdfs, areaperil_id, vulnerability_id):
    weight_dict = getWeights(areaperil_id, vulnerability_id)
    cumulative_prob = 0
    cdf = []
    for damage_bin_id in range(num_damage_bins):
        for attributes in weight_dict:
            try: 
                cumulative_prob += (attribute_cdfs[attributes][damage_bin_id] * weight_dict[attributes])
            except KeyError:
                if damage_bin_id == 0:
                    cumulative_prob += weight_dict[attributes]
        cdf.append(cumulative_prob)
        if cumulative_prob > 0.999999955:
            break
    return cdf


def main():
    main = pd.DataFrame(columns=['event_id', 'areaperil_id', 'vulnerability_id', 'bin_index', 'prob_to', 'bin_mean'])
    for event_id in events:
        attribute_cdfs = getDisaggregateCdfs(event_id)
        for areaperil_id in areaperil_ids:
            for vulnerability_id in vulnerability_ids:
                cdf = getAggregateCdf(attribute_cdfs, areaperil_id, vulnerability_id)
                part = pd.DataFrame({
                    "event_id" : [event_id]*len(cdf),
                    'areaperil_id' : [areaperil_id]*len(cdf),
                    'vulnerability_id' : [vulnerability_id]*len(cdf),
                    'bin_index' : list(range(1, len(cdf)+1)),
                    'prob_to' : cdf,
                    'bin_mean' : bin_means[:len(cdf)]
                })
                main = main.append(part)
    main.to_csv('./cdf_result.csv')

    return True 

print (main())
                
