
# Continuous Testing

## 2. Creating Self-Hosted github runner

I chose a Raspberry Pi 3B for the runner
Download Ubuntu Server 20.04 32bit from [](https://www.raspberrypi.org/software/operating-systems/)
Technically the 64bit image is 


### 1.1 Install Ubuntu
Using a seperate computer running debian, insert the SD card
 - `unxz ubuntu-20.04.3-preinstalled-server-armhf+raspi.img.xz` (Filename will be different)
 - `lsblk` or `fdisk -l` to find the device name of the sd card 
 - `sudo dd if=<path to image> of=<path to device> bs=8M status=progress` to flash the image


### 1.2 Configure the default user account

Default user account username/password is `ubuntu/ubuntu`

The root user is not enabled by default
 - `sudo passwd root` to change root password
 - `passwd -u root` to unlock the account. (Should return `passwd: password expiry information changed`)

Now logout and login as root to change the default user account name
 - `usermod -l <new username> ubuntu` to change the username
 - `usermod -m -d /home/<new username> <new username>` to set the home folder

## 2. Install github runner
[Guide here](https://dev.to/l04db4l4nc3r/self-hosted-github-actions-using-raspberry-pi-212m)
ping 

Navigate to `Settings -> Actions -> Runners` and add a new runner with architecture `Linux` and architecture `ARM`.

Follow the commands given by the UI. Make sure to add a new tag to the runner called `nrf24l01` to enable the CI/CD workflow to target the specific runner with the correct SPI hardware attached.

Note that `./run.sh` needs to be executed on every boot. To launch the runner as a service, run:
`sudo ./svc.sh install`

Test that the runner is working by creating a new workflow with 
```
name: CI/CD 
on: [pull_request, workflow_dispatch]
jobs:
  test:
    runs-on: [self-hosted, nrf24l01]
    steps:
      - name: Hello World
        run: |
          echo "Hello world"
```

## 3. Creating the test scripts

Connect to the runner using the VS Code remote extension.

Run `ip a` on the runner to get the local ip address. Then run `ssh <runner ip address>` on the development computer and use your user password to connect.

Install the `ms-vscode-remote.remote-ssh` extension. Use the new `Remote Explorer` tab to add a new SSH host.

In my case I had to install `libatomic` to resolve an error with the automated installation of VS Code server.
`sudo apt install libatomic1`