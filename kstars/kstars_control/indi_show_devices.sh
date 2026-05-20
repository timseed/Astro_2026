#!/bin/bash
echo "Showing INDI Registered device names "
indi_getprop | cut -d "." -f 1 | sort | uniq
echo "Ended."
