#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Edify functions helper tools
#

# edify output, adapted from Chainfire
OUTFD=$(${toybox} ps -w -o ARGS \
      | ${toybox} grep -v 'grep' \
      | ${toybox} grep -oE 'update(.*)' \
      | ${toybox} cut -d ' ' -f 3);

# zip package path
flashable_zip=$(${toybox} ps -w -o ARGS \
              | ${toybox} grep -v 'grep' \
              | ${toybox} grep -oE 'update(.*)' \
              | ${toybox} cut -d ' ' -f 4-);

# ui_print, adapted from Chainfire
ui_print()
{
  if [ ! -z ${OUTFD} ]; then
    if [ ! -z "${1}" ]; then
      ${toybox} echo "ui_print ${1}" > /proc/self/fd/${OUTFD};
    fi;
    ${toybox} echo "ui_print " > /proc/self/fd/${OUTFD};
  else
    ${toybox} echo ${1};
  fi;
}
