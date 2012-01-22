
CPack is used to create a debian binary package (.deb)

"make package" should build the package.
"sudo dpkg -i opencamlib-<version>-<arch>.deb" should install the package

The DebSourcePPA.cmake script is used to create a debian source package.

The main CMakeLists.txt file defines a target "spackage" that builds
the source package in build/Debian with "make spackage"

This can be uploaded to launchpad with
dput cam-ppa *.changes

cam-ppa is defined in /etc/dput.cf with:
[cam-ppa]
fqdn        = ppa.launchpad.net
method      = sftp
incoming    = ~anders-e-e-wallin/cam/ubuntu
login       = anders-e-e-wallin

Using sftp requires that an SSH key has been registered on launchpad.

Building of the source package can be tested with:
sudo pbuilder --create
sudo pbuilder build *.dsc

If the package has dependencies from "universe" then the following is 
needed in ~/.pbuilderrc
HOOKDIR="/var/cache/pbuilder/hook.d"
COMPONENTS="main universe"
(the pbuilder environment needs to be re-created after changing .pbuilderrc)
