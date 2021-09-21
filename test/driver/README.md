
Library was pulled using `wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.XX.tar.gz`.
Replace XX with the current version number from `http://www.airspayce.com/mikem/bcm2835/index.html`

Install GCC `sudo apt install gcc`
Install Make `sudo apt install make`

Extraction with `tar zxvf bcm2835-1.XX.tar.gz`.
 - Change into the directory
 - Run with `./configure`
 - Build `make`
 - Check `sudo make check`
 