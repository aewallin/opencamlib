import commands
# import subprocess
import sys
import os

f = open("revision.h", "w")

# change directory to src/ 
os.chdir(sys.argv[1])

# run svnversion to get the revision number
rev_number =  commands.getoutput("svnversion") 
print "get_revision.py: got revision: ",rev_number

# commands is deprecated, should use subprocess instead?
# rev_number = subprocess.Popen(["svnversion"], stdout=subprocess.PIPE).communicate()[0] 
# current_dir =  commands.getoutput("pwd") 
# print "get_revision.py: writing revision.h to ", current_dir

rev_string = '#define OCL_REV_STRING "OpenCAMLib Revision ' + rev_number + '"'

f.write(rev_string)

f.close()
