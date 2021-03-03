> __UNDER DEVELOPMENT (Currently not working)__

# Kaluma Testsuite

Test automation for Kaluma runtime

## Configure Raspberry Pi 4

1. Enable Wi-Fi 2.4G Scan

```sh
$ sudo raspi-config
# Localisation Options --> Change WLAN Country --> US --> Yes
```

2. Enable UART (Accessible via `/dev/serial0` or `/dev/ttyS0`)

```sh
$ sudo raspi-config
# Interfacing Options --> P6 Serial --> Yes

$ sudo nano /boot/cmdline.txt
# Delete the text "console=serial0,115200", and save
```

3. Reboot

```sh
$ sudo reboot
```


## Running Tests

```
$ npm test
```
