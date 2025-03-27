# ESPHomeComponents
External Components for ESPHome

## geigerkit

This is the driver for a Geiger-Kit based radiation sensor.
The GK device emits detection count information via its serial port to the
controlling ESP8266 device's serial port.

## ledSign

**TBD**

## max7219uart

**TBD**

## sens21231M

**TBD**

## Si1151

**TBD**

# Development Notes

* workon ESPHOME
* update virtualenv
  - `pip list --format=freeze | grep -v '^\-e' | cut -d = -f 1 | xargs -n1 pip install -U`
* update platformio
  - `python -m pip install -U platformio`
* compile and test locally
  - `esphome compile <fileName>.yaml`
  - N.B. 
    * need to use different include than the production version
      - local
```
- source:
      type: local
      path: /home/jdn/Code/ESPHomeComponents/components/
  components: [ geigerkit ]
  refresh: always
```
      - github
```
- source:
      type: git
      url: https://github.com/jduanen/ESPHomeComponents/components
  components: [ geigerkit ]
  refresh: always
```
    * needs included files to be local to the yaml file
      - `mkdir hold`
      - `cp <configFilePath> hold`
      - `ln -s <secretsFilePath> hold`
    * after finished testing: `rm -rf hold`
* can also flash to device
  - `esphome run <configFilePath>`
* and can check logs from the device
  - `esphome logs <configFilePath>`

