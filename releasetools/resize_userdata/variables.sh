#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Shared variables
#

# Executables paths
e2fsck=${dir_bin}/e2fsck;
resize2fs=${dir_bin}/resize2fs_static;
toybox=${dir_bin}/toybox;

# Recovery paths
cache_recovery=/cache/recovery;
cache_recovery_command=${cache_recovery}/command;

# Identifier tags
flag_partitions_changed=/tmp/flag_partitions_changed;

# UserData paths
data_storage_xml=/data/system/storage.xml;

# Partitions path constants
mmc_block=/dev/block/mmcblk0;

# Partitions items constants
part_data_num=14;

# Partitions dimension constants
part_data_length_sub=16384;

# Variables are defined
variables_set='true';
