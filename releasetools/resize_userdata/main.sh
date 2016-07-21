#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Core script of the Resize UserData Tools
#

#------------------------------------------------------------
# Execution variables
dir_bin=/tmp/install/bin;
dir_script=/tmp/install/resize_userdata;
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
ui_print '       =========================';
ui_print '       | UserData Resizer Tool |';
ui_print '       =========================';
ui_print ' ';

#------------------------------------------------------------
# Mount the UserData partition
if ! part_mount '/data' ${mmc_block}p${part_data_num} 'ext4'; then
  ui_print 'UserData could not be mounted (ext4 ?)';
  exit ${exec_failed};
fi;

#------------------------------------------------------------
# Start the UserData resizing process
echo 1 > ${flag_partitions_changed};
ui_print 'Fixing UserData partition size';
part_data_size=$(part_get_resize2fs ${part_data_num} ${part_data_length_sub});

# If the partition size is valid
if [ ! -z "${part_data_size}" ]; then

  # Delete the storage.xml from UserData
  ui_print 'Deleting '${data_storage_xml};
  ${toybox} rm -f ${data_storage_xml};
  ui_print ' ';

  # Unmount the UserData partition twice to avoid issues
  part_unmount ${mmc_block}p${part_data_num};
  if ! part_unmount ${mmc_block}p${part_data_num}; then
    ui_print 'UserData could not be unmounted';
    exit ${exec_failed};
  fi;

  # Start the partition modifications
  ui_print 'Checking UserData partition structure';
  ${e2fsck} -fp ${mmc_block}p${part_data_num};
  ${resize2fs} ${mmc_block}p${part_data_num} ${part_data_size};

  # Abort if resize2fs failed
  if [ $? -ne 0 ]; then
    ui_print 'Partition resizing failed...';
    exit ${exec_failed};
  fi;

# Abandon the resizing if the size is incorrect
else
  ui_print 'Partition dimensioning failed...';
  part_unmount ${mmc_block}p${part_data_num};
  exit ${exec_failed};
fi;

# End of the modifications
${toybox} rm -f ${cache_recovery_command};
ui_print ' ';
ui_print 'Rebooting to recovery automatically';
ui_print 'You can then flash the ROM zip';
ui_print ' ';

# Reboot to Recovery
/sbin/sleep 10;
/sbin/reboot recovery;

# Return a success
exit ${exec_success};
