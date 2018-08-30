

ROWNO=2

initdata()
{
python GenerateTestData.py -disaggregation -num_location=$1 -num_area_perils=$2 -num_vulnerabilities=$3 -num_agg_area_perils=$4 -num_agg_vulnerabilities=$5 -num_detail_per_aggregate=$6 -num_events=100
}

doit()
{
initdata $1 $2 $2 $3 $5 $6
getmodel -d < input/events.bin > cdf.bin
getmodel2 -d < input/events.bin > cdf2.bin
((ROWNO++))
}

doit 100 100 100 10 10 10
doit 100 1000 100 10 10 10
doit 100 100 1000 10 10 10

doit 100 1000 1000 100 100 100

doit 1000 1000 1000 100 100 100
