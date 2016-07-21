#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Properties functions helper tools
#

# Function prop_default_date_timestamp()
prop_default_date_timestamp()
{
  # Variables
  local build_date;
  local build_timestamp=0;

  # Build date to timestamp
  if [ -e /default.prop ]; then
    build_date=$(${toybox} cat /default.prop \
               | ${toybox} grep 'ro.build.date=' \
               | ${toybox} sed 's/.*=//' \
               | ${toybox} sed 's/[A-Z]* \(20[0-9][0-9]\)/\1/');

    # Convert to timestamp
    build_timestamp=$(${toybox} date -d "${build_date}" -D "%A %B %d %T %Y" +'%s' || \
                      ${toybox} date -d "${build_date}" -D "%A %d %B %T %Y" +'%s' || \
                      echo 0);
  fi;

  # Result output
  ${toybox} echo "${build_timestamp}";
}
