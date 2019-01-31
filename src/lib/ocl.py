import platform
import sys

osname = ''
if sys.platform == "linux" or sys.platform == "linux2":
    osname = 'linux'
elif sys.platform == "darwin":
    osname = 'darwin'
elif sys.platform == "win32":
    osname = 'win32'
elif sys.platform == "win64":
    osname = 'win64'

major_version = sys.version_info[0]
minor_version = sys.version_info[1]

__import__('ocl.%s%s.%s' % (major_version, minor_version, osname))