#!/sbin/sh

# Include device-specific vars
source /sbin/bootrec-device

# Turn the LED off
echo 0 > ${BOOTREC_LED_RED}
echo 0 > ${BOOTREC_LED_GREEN}
echo 0 > ${BOOTREC_LED_BLUE}
