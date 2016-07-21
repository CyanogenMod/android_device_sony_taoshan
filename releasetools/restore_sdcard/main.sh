#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Core script of the Restore SDCard Tools
#

#------------------------------------------------------------
# Execution variables
dir_bin=/tmp/install/bin;
dir_script=/tmp/install/restore_sdcard;
exec_failed=78;
exec_success=89;
source ${dir_script}/variables.sh;
if [ -z "${variables_set}" ]; then
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Shared helper tools
source ${dir_script}/edify_tools.sh;
source ${dir_script}/part_tools.sh;

#------------------------------------------------------------
# Abort if a partition change was performed before
if [ -e ${flag_partitions_changed} ]; then
  ui_print ' ';
  ui_print 'You already performed a partition change';
  ui_print 'Reboot to recovery before continuing...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Script introduction
ui_print ' ';
ui_print '        =======================';
ui_print '        | SDCard Restore Tool |';
ui_print '        =======================';
ui_print ' ';

#------------------------------------------------------------
# If UserData and SDCard formats were programmed by the zip
if [ -e ${cache_flag_format_userdata_sdcard} ]; then

  # Format the UserData and SDCard partitions
  echo 1 > ${flag_partitions_changed};
  ui_print 'Preparing the partitions';
  part_unmount ${mmc_block}p${part_data_num};
  part_unmount ${mmc_block}p${part_sdcard_num};
  ui_print 'Formatting the UserData partition';
  ${make_ext4fs} -b 4096 -g 32768 -i 8192 -I 256 -l -${part_data_length_sub} ${parts_by_name}/${part_data_name};
  ui_print 'Formatting the SDCard partition';
  ${make_ext4fs} -b 4096 -g 32768 -i 8192 -I 256 ${parts_by_name}/${part_sdcard_name};

  # End of the modifications
  ${toybox} rm -f ${cache_flag_format_userdata_sdcard};
  ${toybox} rm -f ${cache_recovery_command};
  ${toybox} rm -f ${cache_restore_sdcard_zip};
  ui_print 'Done with the partitions modifications';
  ui_print ' ';
  ui_print 'Rebooting to recovery automatically';
  ui_print ' ';

  # Reboot to Recovery
  /sbin/sleep 10;
  /sbin/reboot recovery;

  # Return a failure if we came here
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# If the SDCard partition does exist, return a success result
if [ -e ${parts_by_name}/${part_sdcard_name} ] || \
   [ -e ${mmc_block}p${part_sdcard_num} ] ||
   [ part_exists ${part_sdcard_name} ${part_sdcard_num} ]; then
  exit ${exec_success};
fi;

#------------------------------------------------------------
# If the flashable zip is stored on a partition going to be formatted
if [ ! -z "$(${toybox} echo ${flashable_zip} | grep -E '/data/|/sdcard/|/sdcard_legacy/')" ]; then
  ui_print 'The flashable zip is stored on a partition';
  ui_print 'that will be lost by the operation...';

  # On first flash, abort & warn about a second flash
  if [ ! -e ${flag_flashable_internal} ]; then
    ui_print ' ';
    ui_print 'You could save the zip on a MicroSD';
    ui_print 'or use "adb sideload pathtofile.zip"';
    ui_print ' ';
    ui_print 'Otherwise, if you really want to use';
    ui_print 'the current zip, flash it again';
    ${toybox} echo 1 > ${flag_flashable_internal};
    exit ${exec_failed};
  fi;

  # On second flash, allow continuing & losing the zip
  ui_print 'You are agreeing to use the zip';
  ui_print ' ';
fi;

#------------------------------------------------------------
# Announce the partitions verifications
ui_print 'Starting the partitions verifications...';
ui_print ' ';

# Unmount the data related partitions
part_unmount '/data';
part_unmount '/sdcard_legacy';
part_unmount '/sdcard';

#------------------------------------------------------------
# If identifiers of the Cache partition are missing, abort
if [ ! -e ${parts_by_name}/${part_cache_name} ] || \
   [ ! -e ${mmc_block}p${part_cache_num} ] || \
   [ ! part_exists ${part_cache_name} ${part_cache_num} ]; then
  ui_print 'Device has issues with Cache partition';
  ui_print 'Please search for help on forums...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# If identifiers of the UserData partition are missing, abort
if [ ! -e ${parts_by_name}/${part_data_name} ] || \
   [ ! -e ${mmc_block}p${part_data_num} ] || \
   [ ! part_exists ${part_data_name} ${part_data_num} ]; then
  ui_print 'Device has issues with UserData partition';
  ui_print 'Please search for help on forums...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Mount the Cache partition
if ! part_mount '/cache' ${mmc_block}p${part_cache_num} 'ext4 f2fs'; then
  ui_print 'Cache could not be mounted (ext4/f2fs ?)';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Try to mount the UserData partition
if ! part_mount '/data' ${mmc_block}p${part_data_num} 'ext4'; then
  ui_print 'UserData could not be mounted (ext4 ?)';

  # On first flash, abort & warn about a second flash
  if [ ! -e ${flag_wipe_data_attempt} ]; then
    ui_print 'Please verify your partition file system';
    ui_print ' ';
    ui_print 'If you are sure you want to format UserData, ';
    ui_print 'and accept the losses, flash this zip again';
    ${toybox} echo 1 > ${flag_wipe_data_attempt};
    exit ${exec_failed};
  fi;

  # On second flash, allow continuing & losing UserData
  ui_print 'You are agreeing to format your UserData...';

# If the UserData partition could be mounted
else
  # Check if UserData is empty
  if [ ! -d /data ] || [ -n "$(${toybox} find /data/ -type f)" ]; then
    ui_print 'UserData partition still contains files';

    # On first flash, abort & warn about a second flash
    if [ ! -e ${flag_wipe_data_attempt} ]; then
      ui_print 'Please backup your data and Factory Reset';
      ui_print ' ';
      ui_print 'If you are sure you want to format UserData, ';
      ui_print 'and accept the losses, flash this zip again';
      ${toybox} echo 1 > ${flag_wipe_data_attempt};
      exit ${exec_failed};
    fi;

    # On second flash, allow continuing & losing UserData
    ui_print 'You are agreeing to format your UserData...';

  # If UserData is empty, continue
  else
    ui_print 'UserData partition is empty';
  fi;
fi;

# Unmount the UserData partition
part_unmount '/data';

#------------------------------------------------------------
# Announce the process start
ui_print ' ';
ui_print 'Starting the SDCard restore...';

#------------------------------------------------------------
# Read the UserData partition start and end
part_data_start=$(part_get_start ${part_data_name});
part_sdcard_end=$(part_get_end ${part_data_name});

# If the partitions start and end are correct, use them
if [ ! -z "${part_data_start}" ] && \
   [ ! -z "${part_sdcard_end}" ] && \
   [ ${part_data_start} -lt ${part_data_end} ] && \
   [ ${part_data_end} -lt ${part_sdcard_end} ]; then

  # Delete the partition and create Userdata & SDCard
  ${toybox} echo 1 > ${flag_partitions_changed};
  part_unmount ${mmc_block}p${part_data_num};
  part_unify_state='Deleting UserData' && \
    ${sgdisk} --delete=${part_data_num} ${mmc_block};
  test $? -eq 0 && part_unify_state='Creating new UserData' && \
    ${sgdisk} --new=${part_data_num}:${part_data_start}:${part_data_end} ${mmc_block};
  test $? -eq 0 && part_unify_state='Creating new SDCard' && \
    ${sgdisk} --new=${part_sdcard_num}:${part_data_end}:${part_sdcard_end} ${mmc_block};
  test $? -eq 0 && part_unify_state='Renaming new UserData' && \
    ${sgdisk} --change-name=${part_data_num}:${part_data_name} ${mmc_block};
  test $? -eq 0 && part_unify_state='Renaming new SDCard' && \
    ${sgdisk} --change-name=${part_sdcard_num}:${part_sdcard_name} ${mmc_block};

  # If the partitions modifications were successful
  if [ $? -eq 0 ]; then
    ui_print 'Partitions modifications done';

  # If the partitions modifications failed
  else
    ui_print 'Failure during the modifications';
    ui_print 'Error on: '$part_unify_state;
    ui_print 'Search for help on the ROM forums...';
    exit ${exec_failed};
  fi;

# If the partitions dimensions are incorrect
else
  ui_print 'Failure during the dimensioning';
  ui_print 'Search for help on the ROM forums...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# End of the procedure
ui_print 'SDCard restored successfully';
ui_print ' ';

#------------------------------------------------------------
# Initialize a recovery command script
${toybox} mkdir -p ${cache_recovery};
${toybox} echo '' > ${cache_recovery_command};
${toybox} echo '1' > ${cache_flag_format_userdata_sdcard};
${toybox} cp -f ${flashable_zip} ${cache_restore_sdcard_zip};
${toybox} rm -f ${cache_flag_resize_userdata};

# If the preparation of the recovery command failed
if [ ! -e ${cache_recovery_command} ] || \
   [ ! -e ${cache_flag_format_userdata_sdcard} ] || \
   [ ! -e ${cache_restore_sdcard_zip} ]; then
  ui_print 'Failed starting a recovery script';
  ui_print 'Search for help on the ROM forums...';
  ${toybox} rm -f ${cache_recovery_command};
  ${toybox} rm -f ${cache_flag_format_userdata_sdcard};
  ${toybox} rm -f ${cache_restore_sdcard_zip};
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Automatic rebooting instructions
ui_print 'Rebooting to recovery automatically,';
ui_print 'then resizing UserData partition...';
ui_print ' ';

# Reboot to recovery and resize the UserData
${toybox} echo '--update_package='${cache_restore_sdcard_zip} > ${cache_recovery_command};

#------------------------------------------------------------
# Reboot to Recovery
/sbin/sleep 10;
/sbin/reboot recovery;

# Return a failure if we came here
exit ${exec_failed};
