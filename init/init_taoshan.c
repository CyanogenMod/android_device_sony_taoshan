/*
   Copyright (c) 2014, CyanogenMod. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include "init_msm.h"


#define HW_MODEL_ID     "/proc/cci_hw_model_name"
#define BUF_SIZE         64
static char tmp[BUF_SIZE];

static int read_file2(const char *fname, char *data, int max_size)
{
    int fd, rc;

    if (max_size < 1)
        return 0;

    fd = open(fname, O_RDONLY);
    if (fd < 0) {
        ERROR("failed to open '%s'\n", fname);
        return 0;
    }

    rc = read(fd, data, max_size - 1);
    if ((rc > 0) && (rc < max_size))
        data[rc] = '\0';
    else
        data[0] = '\0';
    close(fd);

    return 1;
}

void init_msm_properties(unsigned long msm_id, unsigned long msm_ver, char *board_type)
{
    char platform[PROP_VALUE_MAX];
    int rc;
    unsigned long hw_id = -1;

    UNUSED(msm_id);
    UNUSED(msm_ver);
    UNUSED(board_type);

    rc = property_get("ro.board.platform", platform);
    if (!rc || !ISMATCH(platform, ANDROID_TARGET))
        return;

    /* Obtain model ID */
    rc = read_file2(HW_MODEL_ID, tmp, sizeof(tmp));
    if (rc) {
        hw_id = strtoul(tmp, NULL, 0);
    }

    /* Set common model number */
    property_set("ro.product.model", "Xperia L");

    /* C2104 */
    if (hw_id==87) {
        property_set("ro.build.description", "C2104-user 4.2.2 JDQ39 Android.1016 test-keys");
        property_set("ro.build.fingerprint", "Sony/C2104/C2104:4.2.2/15.3.A.1.17/Android.1016:user/release-keys");
        property_set("ro.build.product", "C2104");
        property_set("ro.product.device", "C2104");
    }

    /* C2105 otherwise */
    else {
        property_set("ro.build.description", "C2105-user 4.2.2 JDQ39 Android.1016 test-keys");
        property_set("ro.build.fingerprint", "Sony/C2105/C2105:4.2.2/15.3.A.1.17/Android.1016:user/release-keys");
        property_set("ro.build.product", "C2105");
        property_set("ro.product.device", "C2105");
    }

}
