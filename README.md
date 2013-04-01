These are the release notes for  openmediavault-greyhole

openmediavault-greyhole is a plug-in for OpenMediaVault providing the functionality in Greyhole (ref. http://www.greyhole.net/).
The software is licensed under GPL 3+ with copyright ditributed among authors. See LICENSE for details.

DOCUMENTATION
Please refer to the web-site http://www.openmediavault.org/ for pointers to documentation.

INSTALLING OPENMEDIAVAULT-GREYHOLE
If you want the latest stable openmediavault-greyhole plug-in, simply use the binary distribution.
You will find Debian packages here: http://packages.omv-plugins.org/ providing a list of available plug-ins in the OpenMediaVault web-interface.
However, if you want to test, install or develop the plugin, you should consider downloading and compiling from source.

DOWNLOADING AND BUILDING OPENMEDIAVAULT-GREYHOLE
For Debian based systems (such as OpenMediaVault itself), install dependencies with:

```
apt-get install git fakeroot debhelper build-essential
```

Download with:

```
git clone https://github.com/OMV-Plugins/openmediavault-greyhole.git
cd openmediavault-greyhole
git checkout develop
```

The first time you build you can simply do:

```
fakeroot debian/rules clean binary
```

For subsequent builds, you will need to update sources and clean up:

```
git fetch
rm -rf usr/share/openmediavault/locale
rm -f debian/rules
git checkout usr/share/openmediavault/locale
git checkout debian/rules
git pull origin develop
git-dch --snapshot --auto --ignore-branch
git-buildpackage --git-ignore-branch --git-builder='dpkg-buildpackage -T binary' --git-ignore-new
```
