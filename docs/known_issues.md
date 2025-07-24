# Known Issues

1. The `mcopy` tools shipped by Ubuntu 22.04 contain a [[SOLVED] ShredOS unable to build VFAT, fails with the mcopy error "Internal error, size too big" on Ubuntu 22.04LTS · Issue #120 · PartialVolume/shredos.x86_64](https://github.com/PartialVolume/shredos.x86_64/issues/120). Update it using apt-get would not resolve the issue. You need to compile and install it manually. Make sure [system dependencies ](../README.md#system-dependencies)already installed in the system before proceed with these steps.
   
   ```
   wget http://ftp.gnu.org/gnu/mtools/mtools-4.0.44.tar.gz
   tar -xf mtools-4.0.44.tar.gz
   cd mtools-4.0.44
   ./configure
   make
   sudo make install
   mcopy --version
   ```
