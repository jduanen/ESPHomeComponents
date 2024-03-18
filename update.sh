#!/bin/bash
rm -rf .esphome/build
cp ~/Code/ESPHomeComponents/components/Si1151/Si1151.{h,cpp} .esphome/external_components/254f630e/components/Si1151/
esphome compile ~/Code/ESPHomeComponents/Si1151.yaml

