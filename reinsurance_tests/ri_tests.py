"""
Test wrapper on FMCals with friendlier data structures.
"""
import argparse
import os
import itertools
import pandas as pd
import subprocess
import json
import shutil
from collections import namedtuple
from tabulate import tabulate

DEDUCTIBLE_AND_LIMIT_CALCRULE_ID = 1
FRANCHISE_DEDUCTIBLE_AND_LIMIT_CALCRULE_ID = 3
DEDUCTIBLE_ONLY_CALCRULE_ID = 12
DEDUCTIBLE_AS_A_CAP_ON_THE_RETENTION_OF_INPUT_LOSSES_CALCRULE_ID = 10
DEDUCTIBLE_AS_A_FLOOR_ON_THE_RETENTION_OF_INPUT_LOSSES_CALCRULE_ID = 11
DEDUCTIBLE_LIMIT_AND_SHARE_CALCRULE_ID = 2
DEDUCTIBLE_AND_LIMIT_AS_A_PROPORTION_OF_LOSS_CALCRUKE_ID = 5
DEDUCTIBLE_WITH_LIMIT_AS_A_PROPORTION_OF_LOSS_CALCRUKE_ID = 9
LIMIT_ONLY_CALCRULE_ID = 14
LIMIT_AS_A_PROPORTION_OF_LOSS_CALCRULE_ID = 15
DEDUCTIBLE_AS_A_PROPORTION_OF_LOSS_CALCRULE_ID = 16

NO_ALLOCATION_ALLOC_ID = 0
ALLOCATE_TO_ITEMS_BY_GUL_ALLOC_ID = 1
ALLOCATE_TO_ITEMS_BY_PREVIOUS_LEVEL_ALLOC_ID = 2

BUILDINGS_COVERAGE_TYPE_ID = 1
CONTENTS_COVERAGE_TYPE_ID = 2
TIME_COVERAGE_TYPE_ID = 3

PERIL_WIND = 1

GUL_INPUTS_FILES = [
    'coverages',
    'gulsummaryxref',
    'items']

IL_INPUTS_FILES = [
    'fm_policytc',
    'fm_profile',
    'fm_programme',
    'fm_xref',
    'fmsummaryxref']

OPTIONAL_INPUTS_FILES = [
    'events']

CONVERSION_TOOLS = {
    'coverages': 'coveragetobin',
    'events': 'evetobin',
    'fm_policytc': 'fmpolicytctobin',
    'fm_profile': 'fmprofiletobin',
    'fm_programme': 'fmprogrammetobin',
    'fm_xref': 'fmxreftobin',
    'fmsummaryxref': 'fmsummaryxreftobin',
    'gulsummaryxref': 'gulsummaryxreftobin',
    'items': "itemtobin"}

COVERAGE_TYPES = [
    BUILDINGS_COVERAGE_TYPE_ID,
    CONTENTS_COVERAGE_TYPE_ID,
    TIME_COVERAGE_TYPE_ID]

PERILS = [PERIL_WIND]


"""
Ktools FM File Structure
---------------------------------------------------------------------------------------------------
items
item_id 	                int     Identifier of the exposure item
coverage_id 	            int     Identifier of the coverage
areaperil_id 	            int     Identifier of the locator and peril
vulnerability_id 	        int 	Identifier of the vulnerability distribution
group_id 	                int 	Identifier of the correlaton group
---------------------------------------------------------------------------------------------------
coverages
coverage_id 	            int 	Identifier of the coveragegenerate
tiv 	                    float 	The total insured value of the coverage
---------------------------------------------------------------------------------------------------
gulsummaryxref
coverage_id 	            int 	Identifier of the coverage
summary_id 	                int 	Identifier of the summary level grouping
summaryset_id 	            int 	Identifier of the summary set
---------------------------------------------------------------------------------------------------
fm_programme
from_agg_id 	            int 	Oasis Financial Module from_agg_id
level_id 	                int 	Oasis Financial Module level_id
to_agg_id 	                int 	Oasis Financial Module to_agg_id
---------------------------------------------------------------------------------------------------
fm_profile
policytc_id 	            int 	Oasis Financial Module policytc_id
calcrule_id 	            int 	Oasis Financial Module calcrule_id
allocrule_id 	            int 	Oasis Financial Module allocrule_id
ccy_id 	                    int 	Oasis Financial Module ccy_id
deductible 	                float 	Deductible
limit 	                    float 	Limit
share_prop_of_lim 	        float 	Share/participation as a proportion of limit
deductible_prop_of_loss 	float 	Deductible as a proportion of loss
limit_prop_of_loss 	        float 	Limit as a proportion of loss
deductible_prop_of_tiv 	    float 	Deductible as a proportion of TIV
limit_prop_of_tiv 	        float 	Limit as a proportion of TIV
deductible_prop_of_limit 	float 	Deductible as a proportion of limit
---------------------------------------------------------------------------------------------------
fm_policytc
layer_id 	                int 	Oasis Financial Module layer_id
level_id 	                int 	Oasis Financial Module level_id
agg_id 	                    int 	Oasis Financial Module agg_id
policytc_id 	            int 	Oasis Financial Module policytc_id
---------------------------------------------------------------------------------------------------
fmsummaryxref
output_id 	                int 	Identifier of the coverage
summary_id  	            int 	Identifier of the summary  level group for one or more output losses
summaryset_id 	            int 	Identifier of the summary set (0 to 9 inclusive)
---------------------------------------------------------------------------------------------------
fm_xref
output_id 	                int 	Identifier of the output group of losses
agg_id 	                    int 	Identifier of the agg_id to output
layer_id 	                int 	Identifier of the layer_id to output
---------------------------------------------------------------------------------------------------
"""

Item = namedtuple(
    "Item", "item_id coverage_id areaperil_id vulnerability_id group_id")
Coverage = namedtuple(
    "Coverage", "coverage_id tiv")
FmProgramme = namedtuple(
    "FmProgramme", "from_agg_id level_id to_agg_id")
FmProfile = namedtuple(
    "FmProfile", "policytc_id calcrule_id allocrule_id ccy_id deductible limit " +
    "share_prop_of_lim deductible_prop_of_loss limit_prop_of_loss deductible_prop_of_tiv " +
    "limit_prop_of_tiv deductible_prop_of_limit")
FmPolicyTc = namedtuple(
    "FmPolicyTc", "layer_id level_id agg_id policytc_id")
GulSummaryXref = namedtuple(
    "GulSummaryXref", "coverage_id summary_id summaryset_id")
FmSummaryXref = namedtuple(
    "FmSummaryXref", "output_id summary_id summaryset_id")
FmXref = namedtuple(
    "FmXref", "output_id agg_id layer_id")
XrefDescription = namedtuple(
    "Description", ("xref_id policy_id location_id coverage_type_id peril_id tiv"))
GulRecord = namedtuple(
    "GulRecord", "event_id item_id sidx loss")


class Location(object):
    def __init__(
            self,
            location_id, area_peril_id, vulnerability_id,
            buildings_value, contents_value, time_value):
        self.location_id = location_id
        self.area_peril_id = area_peril_id
        self.vulnerability_id = vulnerability_id
        self.buildings_value = buildings_value
        self.contents_value = contents_value
        self.time_value = time_value

    def get_tiv(self, coverage_type_id):
        switcher = {
            BUILDINGS_COVERAGE_TYPE_ID: self.buildings_value,
            CONTENTS_COVERAGE_TYPE_ID: self.contents_value,
            TIME_COVERAGE_TYPE_ID: self.time_value
        }
        return switcher.get(coverage_type_id, 0)


class Policy(object):
    def __init__(
            self,
            policy_id, site_limit, blanket_deductible, blanket_limit, locations):
        self.policy_id = policy_id
        self.site_limit = site_limit
        self.blanket_deductible = blanket_deductible
        self.blanket_limit = blanket_limit
        self.locations = locations

class Treaty(object):
    def __init__(
            self):
        pass

# TODO
#
# class LocationPerRiskTreaty(Treaty):
#     def __init__(
#             self,
#             treaty_id, location_ids, per_risk_attachment, per_risk_limit, occurrence_limit,  share):
#         self.treaty_id = treaty_id
#         self.location_ids = location_ids
#         self.per_risk_attachment = per_risk_attachment
#         self.per_risk_limit = per_risk_limit
#         self.occurrence_limit = occurrence_limit
#         self.share = share


# class PolicyPerRiskTreaty(Treaty):
#     def __init__(
#             self,
#             treaty_id, policy_ids, per_risk_attachment, per_risk_limit, occurrence_limit,  share):
#         self.treaty_id = treaty_id
#         self.policy_ids = policy_ids
#         self.per_risk_attachment = per_risk_attachment
#         self.per_risk_limit = per_risk_limit
#         self.occurrence_limit = occurrence_limit
#         self.share = share


# class PolicyFacTreaty(Treaty):
#     def __init__(
#             self,
#             treaty_id, policy_id, attachment, limit, share):
#         self.treaty_id = treaty_id
#         self.policy_id = policy_id
#         self.attachment = attachment
#         self.limit = limitmapping = {frozenset(('House_Number', 
#                       'Street_Number', 
#                       'State')): AddressClass,
#            frozenset(('Name', 
#                       'Address')): EmployeeClass}

# class SurplusShareTreaty(Treaty):
#     def __init__(
#             self,
#             treaty_id, line_size, number_of_lines, occurrence_limit, share):
#         self.treaty_id = treaty_id
#         self.line_size = line_size
#         self.number_of_lines = number_of_lines
#         self.occurrence_limit = occurrence_limit
#         self.share = share

class CatXlTreaty(Treaty):
    def __init__(
            self,
            treaty_id, attachment, occurrence_limit, share):
        self.treaty_id = treaty_id
        self.attachment = attachment
        self.occurrence_limit = occurrence_limit
        self.share = share


class PolicyFacTreaty(Treaty):
    def __init__(
            self,
            treaty_id, policy_id, attachment, limit, share):
        self.treaty_id = treaty_id
        self.policy_id = policy_id
        self.attachment = attachment
        self.limit = limit
        self.share = share


class LocationFacTreaty(Treaty):
    def __init__(
            self,
            treaty_id, location_id, attachment, limit, share):
        self.treaty_id = treaty_id
        self.location_id = location_id
        self.attachment = attachment
        self.limit = limit
        self.share = share


class DirectLayer(object):

    def __init__(self, policies):
        self.policies = policies
        self.item_ids = list()
        self.item_tivs = list()
        self.coverages = pd.DataFrame()
        self.items = pd.DataFrame()
        self.fmprogrammes = pd.DataFrame()
        self.fmprofiles = pd.DataFrame()
        self.fm_policytcs = pd.DataFrame()
        self.fm_xrefs = pd.DataFrame()
        self.xref_descriptions = pd.DataFrame()

    def generate_oasis_structures(self):
        coverage_id = 0
        item_id = 0
        group_id = 0
        policy_agg_id = 0
        policytc_id = 0

        coverages_list = list()
        items_list = list()
        fmprogrammes_list = list()
        fmprofiles_list = list()
        fm_policytcs_list = list()
        fm_xrefs_list = list()
        xref_descriptions_list = list()

        site_agg_id = 0
        for policy in self.policies:
            policy_agg_id = policy_agg_id + 1
            policytc_id = policytc_id + 1
            fmprofiles_list.append(FmProfile(
                policytc_id=policytc_id,
                calcrule_id=DEDUCTIBLE_AND_LIMIT_CALCRULE_ID,
                ccy_id=-1,
                allocrule_id=ALLOCATE_TO_ITEMS_BY_GUL_ALLOC_ID,
                deductible=policy.blanket_deductible,
                limit=policy.blanket_limit,
                share_prop_of_lim=0.0,          # Not used
                deductible_prop_of_loss=0.0,    # Not used
                limit_prop_of_loss=0.0,         # Not used
                deductible_prop_of_tiv=0.0,     # Not used
                limit_prop_of_tiv=0.0,          # Not used
                deductible_prop_of_limit=0.0    # Not used
            ))
            fm_policytcs_list.append(FmPolicyTc(
                layer_id=1,
                level_id=2,
                agg_id=policy_agg_id,
                policytc_id=policytc_id
            ))
            for location in policy.locations:
                group_id = group_id + 1
                site_agg_id = site_agg_id + 1
                policytc_id = policytc_id + 1
                fmprofiles_list.append(FmProfile(
                    policytc_id=policytc_id,
                    calcrule_id=DEDUCTIBLE_AND_LIMIT_CALCRULE_ID,
                    ccy_id=-1,
                    allocrule_id=ALLOCATE_TO_ITEMS_BY_GUL_ALLOC_ID,
                    deductible=0,
                    limit=policy.site_limit,
                    share_prop_of_lim=0.0,          # Not used
                    deductible_prop_of_loss=0.0,    # Not used
                    limit_prop_of_loss=0.0,         # Not used
                    deductible_prop_of_tiv=0.0,     # Not used
                    limit_prop_of_tiv=0.0,          # Not used
                    deductible_prop_of_limit=0.0    # Not used
                ))
                fm_policytcs_list.append(FmPolicyTc(
                    layer_id=1,
                    level_id=1,
                    agg_id=site_agg_id,
                    policytc_id=policytc_id
                ))
                fmprogrammes_list.append(
                    FmProgramme(
                        from_agg_id=site_agg_id,
                        level_id=2,
                        to_agg_id=policy_agg_id
                    )
                )

                for coverage_type_id in COVERAGE_TYPES:
                    tiv = location.get_tiv(coverage_type_id)
                    if tiv > 0:
                        coverage_id = coverage_id + 1
                        coverages_list.append(
                            Coverage(
                                coverage_id=coverage_id,
                                tiv=tiv
                            ))
                        for peril in PERILS:
                            item_id = item_id + 1
                            self.item_ids.append(item_id)
                            self.item_tivs.append(tiv)
                            items_list.append(
                                Item(
                                    item_id=item_id,
                                    coverage_id=coverage_id,
                                    areaperil_id=location.area_peril_id,
                                    vulnerability_id=location.vulnerability_id,
                                    group_id=group_id
                                ))
                            fmprogrammes_list.append(
                                FmProgramme(
                                    from_agg_id=item_id,
                                    level_id=1,
                                    to_agg_id=site_agg_id
                                )
                            )
                            fm_xrefs_list.append(
                                FmXref(
                                    output_id=item_id,
                                    agg_id=item_id,
                                    layer_id=1
                                ))
                            xref_descriptions_list.append(XrefDescription(
                                xref_id=item_id,
                                location_id=location.location_id,
                                coverage_type_id=coverage_type_id,
                                peril_id=peril,
                                policy_id=policy.policy_id,
                                tiv=tiv
                            )
                            )

        self.coverages = pd.DataFrame(coverages_list)
        self.items = pd.DataFrame(items_list)
        self.fmprogrammes = pd.DataFrame(fmprogrammes_list)
        self.fmprofiles = pd.DataFrame(fmprofiles_list)
        self.fm_policytcs = pd.DataFrame(fm_policytcs_list)
        self.fm_xrefs = pd.DataFrame(fm_xrefs_list)
        self.xref_descriptions = pd.DataFrame(xref_descriptions_list)

    def write_oasis_files(self):
        
        self.coverages.to_csv("coverages.csv", index=False)
        self.items.to_csv("items.csv", index=False)
        self.fmprogrammes.to_csv("fm_programme.csv", index=False)
        self.fmprofiles.to_csv("fm_profile.csv", index=False)
        self.fm_policytcs.to_csv("fm_policytc.csv", index=False)
        self.fm_xrefs.to_csv("fm_xref.csv", index=False)

        directory = "direct"
        if os.path.exists(directory):
            shutil.rmtree(directory)
        os.mkdir(directory)
        input_files = GUL_INPUTS_FILES + IL_INPUTS_FILES

        for input_file in input_files:
            conversion_tool = CONVERSION_TOOLS[input_file]
            input_file_path = input_file + ".csv"
            if not os.path.exists(input_file_path):
                continue

            output_file_path = os.path.join(directory, input_file + ".bin")
            command = "{} < {} > {}".format(
                conversion_tool, input_file_path, output_file_path)
            proc = subprocess.Popen(command, shell=True)
            proc.wait()
            if proc.returncode != 0:
                raise Exception(
                    "Failed to convert {}: {}".format(input_file_path, command))

    def apply_fm(self, loss_percentage_of_tiv=1.0, net=False):
        guls_list = list()
        for item_id, tiv in itertools.izip(self.item_ids, self.item_tivs):
            event_loss = loss_percentage_of_tiv * tiv
            guls_list.append(
                GulRecord(event_id=1, item_id=item_id, sidx=-1, loss=event_loss))
            guls_list.append(
                GulRecord(event_id=1, item_id=item_id, sidx=1, loss=event_loss))
        guls_df = pd.DataFrame(guls_list)
        guls_df.to_csv("guls.csv", index=False)
        net_flag = ""
        if net:
            net_flag = "-n"
        command = "gultobin -S 1 < guls.csv | xfmcalc -p direct {} | tee ils.bin | fmtocsv > ils.csv".format(
            net_flag)
        proc = subprocess.Popen(command, shell=True)
        proc.wait()
        if proc.returncode != 0:
            raise Exception("Failed to run fm")
        losses_df = pd.read_csv("ils.csv")
        losses_df.drop(losses_df[losses_df.sidx != 1].index, inplace=True)
        del losses_df['sidx']
        guls_df.drop(guls_df[guls_df.sidx != 1].index, inplace=True)
        del guls_df['event_id']
        del guls_df['sidx']
        guls_df = pd.merge(self.xref_descriptions,
                           guls_df, left_on=['xref_id'], right_on=['item_id'])
        losses_df = pd.merge(guls_df,
                             losses_df, left_on='xref_id', right_on='output_id', suffixes=["_gul", "_net"])
        del losses_df['event_id']
        del losses_df['output_id']
        del losses_df['xref_id']
        del losses_df['item_id']

        return losses_df


class ReinsuranceLayer(object):

    def __init__(self, name, treaties, items, coverages, xref_descriptions):
        self.name = name
        self.treaties = treaties
        self.coverages = items
        self.items = coverages
        self.xref_descriptions = xref_descriptions

        self.item_ids = list()
        self.item_tivs = list()
        self.fmprogrammes = pd.DataFrame()
        self.fmprofiles = pd.DataFrame()
        self.fm_policytcs = pd.DataFrame()
        self.fm_xrefs = pd.DataFrame()

    def generate_oasis_structures(self):
        treaty_agg_id = 0
        treatytc_id = 0

        fmprogrammes_list = list()
        fmprofiles_list = list()
        fm_policytcs_list = list()
        fm_xrefs_list = list()

        for treaty in self.treaties:
            treaty_agg_id = treaty_agg_id + 1
            treatytc_id = treatytc_id + 1
            fmprofiles_list.append(FmProfile(
                policytc_id=treatytc_id,
                calcrule_id=DEDUCTIBLE_LIMIT_AND_SHARE_CALCRULE_ID,
                ccy_id=-1,
                allocrule_id=ALLOCATE_TO_ITEMS_BY_GUL_ALLOC_ID,
                deductible=treaty.attachment,
                limit=treaty.occurrence_limit,
                share_prop_of_lim=treaty.share,
                deductible_prop_of_loss=0.0,    # Not used
                limit_prop_of_loss=0.0,         # Not used
                deductible_prop_of_tiv=0.0,     # Not used
                limit_prop_of_tiv=0.0,          # Not used
                deductible_prop_of_limit=0.0    # Not used
            ))
            fm_policytcs_list.append(FmPolicyTc(
                layer_id=1,
                level_id=1,
                agg_id=treaty_agg_id,
                policytc_id=treatytc_id
            ))
            for __, xref_description in self.xref_descriptions.iterrows():
                fmprogrammes_list.append(
                    FmProgramme(
                        from_agg_id=xref_description.xref_id,
                        level_id=1,
                        to_agg_id=treaty_agg_id
                    )
                )
                fm_xrefs_list.append(
                    FmXref(
                        output_id=xref_description.xref_id,
                        agg_id=xref_description.xref_id,
                        layer_id=1
                    ))

        self.fmprogrammes = pd.DataFrame(fmprogrammes_list)
        self.fmprofiles = pd.DataFrame(fmprofiles_list)
        self.fm_policytcs = pd.DataFrame(fm_policytcs_list)
        self.fm_xrefs = pd.DataFrame(fm_xrefs_list)

    def write_oasis_files(self):

        self.fmprogrammes.to_csv("fm_programme.csv", index=False)
        self.fmprofiles.to_csv("fm_profile.csv", index=False)
        self.fm_policytcs.to_csv("fm_policytc.csv", index=False)
        self.fm_xrefs.to_csv("fm_xref.csv", index=False)

        directory = self.name
        if os.path.exists(directory):
            shutil.rmtree(directory)
        os.mkdir(directory)

        input_files = GUL_INPUTS_FILES + IL_INPUTS_FILES

        for input_file in input_files:
            conversion_tool = CONVERSION_TOOLS[input_file]
            input_file_path = input_file + ".csv"
            if not os.path.exists(input_file_path):
                continue

            output_file_path = os.path.join(directory, input_file + ".bin")
            command = "{} < {} > {}".format(
                conversion_tool, input_file_path, output_file_path)
            proc = subprocess.Popen(command, shell=True)
            proc.wait()
            if proc.returncode != 0:
                raise Exception(
                    "Failed to convert {}: {}".format(input_file_path, command))

    def apply_fm(self, input):
        command = \
            "xfmcalc -p {0} < {1}.bin | tee {0}.bin | fmtocsv > {0}.csv".format(
                self.name, input)
        proc = subprocess.Popen(command, shell=True)
        proc.wait()
        if proc.returncode != 0:
            raise Exception("Failed to run fm")
        losses_df = pd.read_csv("{}.csv".format(self.name))
        losses_df.drop(losses_df[losses_df.sidx != 1].index, inplace=True)
        losses_df = pd.merge(self.xref_descriptions,
                             losses_df, left_on='xref_id', right_on='output_id')
        del losses_df['event_id']
        del losses_df['sidx']
        del losses_df['output_id']
        del losses_df['xref_id']
        return losses_df

class CustomJsonEncoder(json.JSONEncoder):
    def default(self, o):
        # Here you can serialize your object depending of its type
        # or you can define a method in your class which serializes the object           
        if isinstance(o, (
            Policy, Location, )):
            return o.__dict__  # Or another method to serialize it
        else:
            return json.JSONEncoder.encode(self, o)

mapping = {
    frozenset((
        'policy_id',
        'site_limit',
        'blanket_deductible',
        'blanket_limit',
        'locations')): Policy,
    frozenset((
        'location_id',
        'area_peril_id',
        'vulnerability_id',
        'buildings_value',
        'contents_value',
        'time_value')): Location,
    frozenset((
        'treaty_id',
        'attachment',
        'occurrence_limit',
        'share')): CatXlTreaty,        
    frozenset((
        'treaty_id',
        'policy_id',
        'attachment',
        'limit',
        'share')): PolicyFacTreaty,        
    frozenset((
        'treaty_id',
        'attachment',
        'limit',
        'share')): CatXlTreaty
        }

def class_mapper(d):
    return mapping[frozenset(d.keys())](**d)

parser = argparse.ArgumentParser(description='Run Oasis FM examples with reinsurance.')
parser.add_argument(
    '-n', '--name', metavar='N', type=str, required=True,
    help='The analysis name. All intermediate files will be "+ \
            "saved in a labelled directory.')
parser.add_argument(
    '-p', '--policy_json', metavar='N', type=str, required=True,
    help='The JSON file containing the direct policies.')
parser.add_argument(
    '-r1', '--ri_1_json', metavar='N', type=str, required=False,
    help='The JSON file containing the first reinsurance inuring layer.')
parser.add_argument(
    '-r2', '--ri_2_json', metavar='N', type=str, required=False,
    help='The JSON file containing the second reinsurance inuring layer.')

args = parser.parse_args()

run_name = args.name
policy_json_file = args.policy_json
ri_1_json_file = args.ri_1_json
ri_2_json_file = args.ri_2_json

for filepath in [policy_json_file, ri_1_json_file, ri_2_json_file]:
    if filepath is None: 
        continue
    if not os.path.exists(policy_json_file):
        print "Path does not exist: {}".format(policy_json_file)
        exit(1)

if os.path.exists(run_name):
    shutil.rmtree(run_name)
os.mkdir(run_name)

do_ri_1 = ri_1_json_file is not None
do_ri_2 = ri_2_json_file is not None

with open(policy_json_file) as json_data:
    policies = json.load(json_data, object_hook=class_mapper)

if do_ri_1:
    with open(ri_1_json_file) as json_data:
        treaties1 = json.load(json_data, object_hook=class_mapper)

if do_ri_2:
    with open(ri_2_json_file) as json_data:
        treaties2 = json.load(json_data, object_hook=class_mapper)

cwd = os.getcwd()
try:
    os.chdir(run_name)

    direct_layer = DirectLayer(policies=policies)
    direct_layer.generate_oasis_structures()
    direct_layer.write_oasis_files()
    losses_df = direct_layer.apply_fm(loss_percentage_of_tiv=0.1, net=True)
    print "Direct layer loss"
    print tabulate(losses_df, headers='keys', tablefmt='psql', floatfmt=".2f")
    print ""
    print ""

    if do_ri_1:
        reinsurance_layer = ReinsuranceLayer(
            name = "ri1",
            treaties=treaties1,
            items=direct_layer.items,
            coverages=direct_layer.coverages,
            xref_descriptions=direct_layer.xref_descriptions
        )

        reinsurance_layer.generate_oasis_structures()
        reinsurance_layer.write_oasis_files()
        treaty_losses_df = reinsurance_layer.apply_fm("ils")
        print "Reinsurance - first inuring layer"
        print tabulate(treaty_losses_df, headers='keys', tablefmt='psql', floatfmt=".2f")
        print ""
        print ""

    if do_ri_2:

        reinsurance_layer = ReinsuranceLayer(
            name="ri2",
            treaties=treaties2,
            items=direct_layer.items,
            coverages=direct_layer.coverages,
            xref_descriptions=direct_layer.xref_descriptions
        )

        reinsurance_layer.generate_oasis_structures()
        reinsurance_layer.write_oasis_files()
        treaty_losses_df = reinsurance_layer.apply_fm("ri1")
        print "Reinsurance - second inuring layer"
        print tabulate(treaty_losses_df, headers='keys', tablefmt='psql', floatfmt=".2f")
        print ""
        print ""
        
finally:
    os.chdir(cwd)
