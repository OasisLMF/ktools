add_subdirectory(aalcalc)
add_subdirectory(aalcalctocsv)
add_subdirectory(cdftocsv)
add_subdirectory(coveragetobin)
add_subdirectory(coveragetocsv)
add_subdirectory(damagebintobin)
add_subdirectory(damagebintocsv)
add_subdirectory(eltcalc)
add_subdirectory(eve)
add_subdirectory(evetobin)
add_subdirectory(evetocsv)
add_subdirectory(fmcalc)
add_subdirectory(fmpolicytctobin)
add_subdirectory(fmpolicytctocsv)
add_subdirectory(fmprofilebinconv)
add_subdirectory(fmprofiletobin)
add_subdirectory(fmprofiletocsv)
add_subdirectory(fmprogrammetobin)
add_subdirectory(fmprogrammetocsv)
add_subdirectory(fmsummaryxreftobin)
add_subdirectory(fmsummaryxreftocsv)
add_subdirectory(fmtocsv)
add_subdirectory(fmxreftobin)
add_subdirectory(fmxreftocsv)
add_subdirectory(footprintconv)
add_subdirectory(footprinttobin)
add_subdirectory(footprinttocsv)
add_subdirectory(getmodel)
add_subdirectory(gulcalc)
add_subdirectory(gulsummaryxreftobin)
add_subdirectory(gulsummaryxreftocsv)
add_subdirectory(gultobin)
add_subdirectory(gultocsv)
add_subdirectory(itemtobin)
add_subdirectory(itemtocsv)
add_subdirectory(kat)
if (BUILD_TESTING)
	add_subdirectory(kunit)
endif()
add_subdirectory(leccalc)
add_subdirectory(ordleccalc)
add_subdirectory(occurrencetobin)
add_subdirectory(occurrencetocsv)
add_subdirectory(periodstobin)
add_subdirectory(periodstocsv)
add_subdirectory(pltcalc)
add_subdirectory(randtobin)
add_subdirectory(randtocsv)
add_subdirectory(returnperiodtobin)
add_subdirectory(returnperiodtocsv)
add_subdirectory(summarycalc)
add_subdirectory(summarycalctobin)
add_subdirectory(summarycalctocsv)
add_subdirectory(summaryindex)
add_subdirectory(validateitems)
add_subdirectory(vulnerabilitytobin)
add_subdirectory(vulnerabilitytocsv)
if (WIN32)
	add_subdirectory(wingetopt)
endif()
