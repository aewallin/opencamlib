import commands
import subprocess

f = open("revision.h", "w")

rev_number =  commands.getoutput("svnversion") 
# rev_number = subprocess.Popen(["svnversion"], stdout=subprocess.PIPE).communicate()[0] 

rev_string = '#define OCL_REV_STRING "OpenCAMLib Revision ' + rev_number + '"'

f.write(rev_string)

f.close()
