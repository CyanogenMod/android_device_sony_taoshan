#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Core script of the UserData Unify Tools
#

#------------------------------------------------------------
# Execution variables
dir_bin=/tmp/install/bin;
dir_script=/tmp/install/unify_userdata;
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
source ${dir_script}/prop_tools.sh;

#------------------------------------------------------------
# Abort if a partition change was performed before
if [ -e ${flag_partitions_changed} ]; then
  ui_print ' ';
  ui_print 'You already performed a partition change';
  ui_print 'Reboot to recovery before continuing...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Detect a MultiROM installation and ignore the unification
if [ -e /tmp/mrom_fakebootpart ]; then
  ui_print ' ';
  ui_print 'Installation as a MultiROM detected';
  ui_print 'Unified partitions layout validated';
  ui_print ' ';
  exit ${exec_success};
fi;

#------------------------------------------------------------
# Handle the usage of an old dangerous TWRP version
if [ -e /twres/twrp ] || [ -e /res/twrp ]; then
  if [ $(prop_default_date_timestamp) -lt ${twrp_timestamp_safe} ]; then
    ui_print ' ';
    ui_print 'The TWRP version you are using is outdated,';
    ui_print 'and may introduce a lot of problems...';
    ui_print 'Please update to latest TWRP Recovery';
    exit ${exec_failed};
  fi;
fi;

#------------------------------------------------------------
# If UserData resizing was programmed by the user
if [ -e ${cache_flag_resize_userdata} ]; then
  ${toybox} rm -f ${cache_flag_resize_userdata};

  # Extract the resize_userdata.zip and reboot on it
  ui_print ' ';
  ui_print 'Prepare the UserData resizing...';
  ${toybox} cp -f ${tmp_resize_userdata_zip} ${cache_resize_userdata_zip};
  ${toybox} echo '--update_package='${cache_resize_userdata_zip} > ${cache_recovery_command};

  # If the preparation of the recovery command failed
  if [ ! -e ${cache_recovery_command} ] || \
     [ ! -e ${cache_resize_userdata_zip} ]; then
    ui_print ' ';
    ui_print 'Failed starting a recovery script';
    ui_print 'Search for help on the ROM forums...';
    ${toybox} rm -f ${cache_recovery_command};
    ${toybox} rm -f ${cache_resize_userdata_zip};
    exit ${exec_failed};
  fi;

  # End of the modifications
  ui_print 'Rebooting to recovery automatically';
  ui_print ' ';

  # Reboot to Recovery
  /sbin/sleep 10;
  /sbin/reboot recovery;

  # Return a failure if we came here
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# If the SDCard partition does not exists, ROM flashable
if [ ! -e ${parts_by_name}/${part_sdcard_name} ]; then

  # If the unification process left a resize_userdata zip in cache
  if [ -e "${cache_resize_userdata_zip}" ]; then
    rm -f ${cache_resize_userdata_zip};
  fi;

  # If the UserData partition is not mountable, abort
  part_unmount '/data';
  part_unmount '/sdcard';
  if ! part_mount '/data' ${mmc_block}p${part_data_num} 'ext4 f2fs'; then
    ui_print ' ';
    ui_print 'UserData could not be mounted (ext4/f2fs ?)';
    ui_print 'Please do a Factory Reset before continuing';
    exit ${exec_failed};

  # If the UserData is mountable, check the storage.xml file
  elif ${toybox} grep -q 'primaryStorageUuid="primary_physical"' ${data_storage_xml}; then
    ui_print ' ';
    ui_print 'Finalize the migration, reset storages';
    ui_print 'Deleting '${data_storage_xml};
    ui_print ' ';
    ${toybox} rm -f ${data_storage_xml};
  fi;

  # If everything checks out for a ROM flash, return a success
  exit ${exec_success};
fi;

#------------------------------------------------------------
# Script introduction
ui_print ' ';
ui_print '       =========================';
ui_print '       | UserData Unifier Tool |';
ui_print '       =========================';
ui_print ' ';

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
   part_exists ${part_cache_name} ${part_cache_num}; then
  ui_print 'Device has issues with Cache partition';
  ui_print 'Please search for help on forums...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# If identifiers of the SDCard partition are missing, abort
if [ ! -e ${parts_by_name}/${part_sdcard_name} ] || \
   [ ! -e ${mmc_block}p${part_sdcard_num} ] || \
   part_exists ${part_sdcard_name} ${part_sdcard_num}; then
  ui_print 'Device has issues with SDCard partition';
  ui_print 'Please search for help on forums...';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# If identifiers of the UserData partition are missing, abort
if [ ! -e ${parts_by_name}/${part_data_name} ] || \
   [ ! -e ${mmc_block}p${part_data_num} ] || \
   part_exists ${part_data_name} ${part_data_num}; then
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
# Try to mount the SDCard partition
if ! part_mount '/sdcard_legacy' ${mmc_block}p${part_sdcard_num} 'ext4 exfat'; then
  ui_print 'SDCard could not be mounted (ext4/exfat ?)';

  # On first flash, abort & warn about a second flash
  if [ ! -e ${flag_wipe_sdcard_attempt} ]; then
    ui_print 'Please verify your partition file system';
    ui_print ' ';
    ui_print 'If you are sure you want to format SDCard, ';
    ui_print 'and accept the losses, flash this zip again';
    ${toybox} echo 1 > ${flag_wipe_sdcard_attempt};
    exit ${exec_failed};
  fi;

  # On second flash, allow continuing & losing UserData
  ui_print 'You are agreeing to format your SDCard...';

# If the SDCard partition could be mounted
else
  # Check if SDCard is empty
  if [ ! -d /sdcard_legacy ] || [ -n "$(${toybox} find /sdcard_legacy/ -type f)" ]; then
    ui_print 'SDCard partition still contains files';

    # On first flash, abort & warn about a second flash
    if [ ! -e ${flag_wipe_sdcard_attempt} ]; then
      ui_print 'Please copy your data from internal SDCard';
      ui_print 'Do not format the partition yourself';
      ui_print ' ';
      ui_print 'If you are sure you want to remove SDCard,';
      ui_print 'and accept the losses, flash this zip again';
      ${toybox} echo 1 > ${flag_wipe_sdcard_attempt};
      exit ${exec_failed};
    fi;

    # On second flash, allow continuing & losing SDCard
    ui_print 'You are agreeing to remove internal SDCard...';
    ui_print ' ';

  # If SDCard is empty, continue
  else
    ui_print 'SDCard partition is empty';
  fi;
  part_unmount '/sdcard_legacy';
fi;

#------------------------------------------------------------
# Try to mount the UserData partition
if ! part_mount '/data' ${mmc_block}p${part_data_num} 'ext4'; then
  ui_print ' ';
  ui_print 'UserData could not be mounted (ext4 ?)';
  ui_print 'Please verify your partition file system';
  ui_print ' ';
  ui_print 'UserData needs to be using ext4,';
  ui_print 'so please perform a Factory Reset';
  exit ${exec_failed};

# If the UserData partition could be mounted
else
  # Check if UserData is empty
  if [ ! -d /data ] || [ -n "$(${toybox} find /data/ -type f)" ]; then
    ui_print 'UserData partition still contains files';

    # On first flash, abort & warn about a second flash
    if [ ! -e ${flag_wipe_data_attempt} ]; then
      ui_print ' ';
      ui_print 'Please backup your data to avoid losses';
      ui_print 'UserData will try to be kept but be safe';
      ui_print ' ';
      ui_print 'If you are sure you want to keep UserData,';
      ui_print 'and accept the risks, flash this zip again';
      ${toybox} echo 1 > ${flag_wipe_data_attempt};
      exit ${exec_failed};
    fi;

    # On second flash, allow continuing & risking UserData
    ui_print 'You are agreeing to risk your UserData...';

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
ui_print 'Starting the UserData unification...';

#------------------------------------------------------------
# Read the new UserData partition start and end
part_data_start=$(part_get_start ${part_data_name});
part_data_end=$(part_get_end ${part_sdcard_name});

# If the partition start and end are correct
if [ ! -z "${part_data_start}" ] && \
   [ ! -z "${part_data_end}" ] && \
   [ ${part_data_start} -lt ${part_data_end} ]; then

  # Delete partitions and create Userdata
  ${toybox} echo 1 > ${flag_partitions_changed};
  part_unmount ${mmc_block}p${part_data_num};
  part_unmount ${mmc_block}p${part_sdcard_num};
  part_unify_state='Deleting SDCard' && \
    ${sgdisk} --delete=${part_sdcard_num} ${mmc_block};
  test $? -eq 0 && part_unify_state='Deleting UserData' && \
    ${sgdisk} --delete=${part_data_num} ${mmc_block};
  test $? -eq 0 && part_unify_state='Creating new UserData' && \
    ${sgdisk} --new=${part_data_num}:${part_data_start}:${part_data_end} ${mmc_block};
  test $? -eq 0 && part_unify_state='Renaming new UserData' && \
    ${sgdisk} --change-name=${part_data_num}:${part_data_name} ${mmc_block};

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
ui_print 'UserData unified successfully';
ui_print ' ';

#------------------------------------------------------------
# Initialize a recovery command script
${toybox} mkdir -p ${cache_recovery};
${toybox} echo '' > ${cache_recovery_command};
${toybox} cp -f ${tmp_resize_userdata_zip} ${cache_resize_userdata_zip};
${toybox} rm -f ${cache_flag_format_userdata_sdcard};

# If the preparation of the recovery command failed
if [ ! -e ${cache_recovery_command} ] || \
   [ ! -e ${cache_resize_userdata_zip} ]; then
  ui_print 'Failed starting a recovery script';
  ui_print 'Search for help on the ROM forums...';
  ${toybox} rm -f ${cache_recovery_command};
  ${toybox} rm -f ${cache_resize_userdata_zip};
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Automatic rebooting instructions
ui_print 'Rebooting to recovery automatically,';
ui_print 'then resizing UserData partition...';
ui_print ' ';

# Reboot to recovery and resize the UserData
${toybox} echo '--update_package='${cache_resize_userdata_zip} > ${cache_recovery_command};

#------------------------------------------------------------
# Reboot to Recovery
/sbin/sleep 10;
/sbin/reboot recovery;

# Return a failure if we came here
exit ${exec_failed};
