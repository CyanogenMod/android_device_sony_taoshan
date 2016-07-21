#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
#

# Device name
device="${1#*_}";
if [ -z "${device}" ]; then
  device='mint';
fi;

# Device variables
path=$(cd ${0%/*} && pwd -P);
name=$(basename ${0%.sh})
input=${path}/${name};
reporoot=${path%%/device*};
out="${reporoot}/out/target/product/${device}";
targetdir="${out}/${name}_temp";
targetzip="${out}/${name}-$(date +%Y%m%d)-${device}.zip";
targettmpzip="${targetzip}.unsigned.zip";
binary_updater="${out}/obj/EXECUTABLES/updater_intermediates/updater";
binary_files="${out}/recovery/root/sbin/make_ext4fs ${out}/recovery/root/sbin/sgdisk ${out}/utilities/toybox";

# Script introduction
echo "";
echo "";
echo "++++ ${name} ++++";
echo "";

# Verify if output files exist
for file in ${binary_files} ${binary_updater}; do
  if [ ! -f ${file} ]; then
    echo " Full '${device}' build required for the package (${file#${out}/} not found)";
    echo "";
    exit 1;
  fi;
done;

# Delete output file and dir if exist
rm -rf ${targetdir};
rm -f ${targettmpzip};
rm -f ${targetzip};

# Create a temporary work folder
mkdir "${targetdir}";
mkdir -p "${targetdir}/install/bin";
mkdir -p "${targetdir}/install/${name}";
mkdir -p "${targetdir}/META-INF/com/google/android";
cd ${targetdir};

# Copy the output files
cp ${input}/updater-script ./META-INF/com/google/android/;
cp ${binary_updater} ./META-INF/com/google/android/update-binary;
cp ${input}/*.sh ./install/${name}/;
for file in ${binary_files}; do
  cp ${file} ./install/bin/;
done;

# Package the zip output
zip ${targettmpzip} * -r;

# Sign the zip output
java -jar ${reporoot}/prebuilts/sdk/tools/lib/signapk.jar \
     -w ${reporoot}/build/target/product/security/testkey.x509.pem \
     ${reporoot}/build/target/product/security/testkey.pk8 \
     ${targettmpzip} \
     ${targetzip};

# Restore the original path
cd ${path};

# Delete the temporary work folder and zip output
rm -rf ${targetdir};
rm -f ${targettmpzip};

# Result flashable zip
echo "";
echo -e "\e[0;33mPackage ${name}:\e[0m ${targetzip}";
echo "";
