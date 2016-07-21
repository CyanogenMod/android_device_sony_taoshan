#!/sbin/sh
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2016 Adrian DC
#
# Partitions shared tools
#

# Function part_mount(point,path,type)
part_mount()
{
  # Partition already mounted
  if ${toybox} grep -q "${1} " /proc/mounts; then
    return 0;
  fi;

  # Prepare the partition mounts
  ${toybox} mkdir -p ${1};
  if ${toybox} grep -q "${2} " /proc/mounts; then
    ${toybox} umount -fl "${2}";
  fi;

  # Try mounting the partition with each of the file systems
  for filesystem in ${3}; do
    if ${toybox} mount -t $filesystem ${2} ${1}; then
      return 0;
    fi;
  done;

  # If the mounts failed, return an error
  ${toybox} echo "part_mount: Failed on ${1}";
  return 1;
}

# Function part_unmount(point)
part_unmount()
{
  # Mount the partition if required
  if ${toybox} grep -q "${1} " /proc/mounts; then
    if ! ${toybox} umount -fl "${1}"; then
      ${toybox} echo "part_umount: Failed on ${1}";
      return 1;
    fi;
  fi;

  return 0;
}

# Function part_exists(name,num)
part_exists()
{
  # Verify if a partition exists and matches a number
  local part_entry=$(${sgdisk} --print ${mmc_block} \
                   | ${toybox} grep "${1}" \
                   | ${toybox} grep " ${2} ");
  if [ ! -z "${part_entry}" ]; then
    return 1;
  fi;

  return 0;
}

# Function part_get_start(name)
part_get_start()
{
  # Return the partition start value
  ${toybox} echo $(${sgdisk} --print ${mmc_block} \
                 | ${toybox} grep "${1}" \
                 | ${toybox} tr -s ' ' \
                 | ${toybox} cut -d ' ' -f 3);
}

# Function part_get_end(name)
part_get_end()
{
  # Return the partition start value
  ${toybox} echo $(${sgdisk} --print ${mmc_block} \
                 | ${toybox} grep ${1} \
                 | ${toybox} tr -s ' ' \
                 | ${toybox} cut -d ' ' -f 4);
}
