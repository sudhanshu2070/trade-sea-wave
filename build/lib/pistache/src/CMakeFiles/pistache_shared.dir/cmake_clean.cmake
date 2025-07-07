file(REMOVE_RECURSE
  "libpistache.pdb"
  "libpistache.so"
  "libpistache.so.0.5"
  "libpistache.so.0.5.9.20250518"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/pistache_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
