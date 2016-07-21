# Copyright (C) 2016 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import subprocess

ANDROID_BUILD_TOP = os.getenv('ANDROID_BUILD_TOP')
OUT_DIR = os.getenv('OUT')
TARGET_PRODUCT = os.getenv('TARGET_PRODUCT')

DEVICE_RELEASE_TOOLS = os.path.join(ANDROID_BUILD_TOP, 'device', 'sony', 'taoshan', 'releasetools')
INSTALL_BIN_PATH = os.path.join('install', 'bin')
INSTALL_UNIFY_USERDATA_PATH = os.path.join('install', 'unify_userdata')
RECOVERY_SBIN_DIR = os.path.join(OUT_DIR, 'recovery', 'root', 'sbin')
UTILITIES_DIR = os.path.join(OUT_DIR, 'utilities')

def FullOTA_Assertions(self):
  AddUnifiedUserDataAssertion(self, self.input_zip)

def IncrementalOTA_Assertions(self):
  AddUnifiedUserDataAssertion(self, self.target_zip)

def AddUnifiedUserDataAssertion(self, input_zip):
  """Execute the resize_data.sh package script."""
  subprocess.call([os.path.join(DEVICE_RELEASE_TOOLS, 'resize_userdata.sh'), str(TARGET_PRODUCT)])
  """Include the required binaries in the output zip."""
  self.output_zip.write(os.path.join(OUT_DIR, 'resize_userdata.zip'), os.path.join(INSTALL_UNIFY_USERDATA_PATH, 'resize_userdata.zip'))
  self.output_zip.write(os.path.join(RECOVERY_SBIN_DIR, 'sgdisk'), os.path.join(INSTALL_BIN_PATH, 'sgdisk'))
  self.output_zip.write(os.path.join(UTILITIES_DIR, 'toybox'), os.path.join(INSTALL_BIN_PATH, 'toybox'))
  """Core script of the UserData Unify Tool."""
  self.script.AppendExtra('package_extract_dir("install", "/tmp/install");')
  self.script.AppendExtra('set_metadata_recursive("/tmp/install/bin", "uid", 0, "gid", 0, "dmode", 0755, "fmode", 0755);')
  self.script.AppendExtra('set_metadata_recursive("/tmp/install/unify_userdata", "uid", 0, "gid", 0, "dmode", 0755, "fmode", 0755);')
  self.script.AppendExtra('if run_program("/tmp/install/unify_userdata/main.sh") == 22784 then')
  self.script.AppendExtra('ui_print("Unified UserData detected");')
  self.script.AppendExtra('else')
  self.script.AppendExtra('ui_print(" ");')
  self.script.AppendExtra('ui_print("==================================");')
  self.script.AppendExtra('ui_print(" The device needs to be converted");')
  self.script.AppendExtra('ui_print("  to Unified UserData partition");')
  self.script.AppendExtra('ui_print("==================================");')
  self.script.AppendExtra('ui_print(" ");')
  self.script.AppendExtra('sleep(5);')
  self.script.AppendExtra('assert(abort("Please read the informations above..."););')
  self.script.AppendExtra('endif;')
