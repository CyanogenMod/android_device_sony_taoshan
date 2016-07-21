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
  # Return the build date in timestamp
  if [ -e /default.prop ]; then
    build_date=$(${toybox} cat /default.prop \
               | ${toybox} grep 'ro.build.date=' \
               | ${toybox} sed 's/.*=//' \
               | ${toybox} sed 's/[A-Z]* \(20[0-9][0-9]\)/\1/');
    ${toybox} date -d "${build_date}" -D "%A %B %d %T %Y" +'%s';
  else
    ${toybox} echo '0';
  fi;
}
