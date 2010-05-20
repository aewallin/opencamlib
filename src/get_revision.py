import commands

f = open("revision.h", "w")

rev_number =  commands.getoutput("svnversion") 

rev_string = '#define OCL_REV_STRING "OpenCAMLib Revision ' + rev_number + '"'

f.write(rev_string)

f.close()
