#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Shared variables
#

# Executables paths
sgdisk=${dir_bin}/sgdisk;
toybox=${dir_bin}/toybox;

# Recovery paths
cache_recovery=/cache/recovery;
cache_recovery_command=${cache_recovery}/command;
cache_flag_format_userdata_sdcard=${cache_recovery}/flag_format_userdata_sdcard;
cache_flag_resize_userdata=${cache_recovery}/flag_resize_userdata;
cache_resize_userdata_name=resize_userdata.zip
cache_resize_userdata_zip=${cache_recovery}/${cache_resize_userdata_name};
tmp_resize_userdata_zip=${dir_script}/${cache_resize_userdata_name};

# Identifier tags
flag_flashable_internal=/tmp/flag_flashable_internal;
flag_partitions_changed=/tmp/flag_partitions_changed;
flag_wipe_data_attempt=/tmp/flag_wipe_data_attempt;
flag_wipe_sdcard_attempt=/tmp/flag_wipe_sdcard_attempt;

# UserData paths
data_storage_xml=/data/system/storage.xml;

# Partitions path constants
mmc_block=/dev/block/mmcblk0;
parts_by_name=/dev/block/platform/msm_sdcc.1/by-name;

# Partitions items constants
part_cache_name='cache';
part_data_name='userdata';
part_sdcard_name='sdcard';
part_cache_num=30;
part_data_num=31;
part_sdcard_num=32;

# Partitions dimension constants
part_data_length_sub=16384;

# TWRP updated timestamp (2016/08/04)
twrp_timestamp_safe=1470268800;

# Variables are defined
variables_set='true';
