import commands

f = open("revision.h", "w")

rev_number = int( commands.getoutput("svnversion") )

rev_string = '#define OCL_REV_STRING "OpenCAMLib Revision ' + ('%d' % rev_number) + '"'

f.write(rev_string)

f.close()
