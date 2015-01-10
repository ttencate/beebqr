#!/bin/bash
set +e
file=$(gphoto2 --camera='USB PTP Class Camera' -L | grep 'IMG_.*\.JPG' | tail -n1 | cut -c8-19)
gphoto2 --camera='USB PTP Class Camera' --folder='/store_00010001/DCIM/107___01' --get-file="$file"
