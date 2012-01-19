import sys
import ocl

# This example shows how to aggregate lines and arcs into path objects and then
# how to retrieve such data.
print ocl.version()

paths = []

# use Point(x,y) which sets z=0
path_id_1 = ocl.Path()
path_id_1.append(ocl.Line(ocl.Point(6,-9), ocl.Point(10.79422863,0.69615242)))
path_id_1.append(ocl.Line(ocl.Point(10.79422863,0.69615242), ocl.Point(4.79422863,9.69615242)))
path_id_1.append(ocl.Line(ocl.Point(4.79422863,9.69615242), ocl.Point(-6,9)))
path_id_1.append(ocl.Line(ocl.Point(-6,9), ocl.Point(-10.7942286,-0.69615242)))
path_id_1.append(ocl.Line(ocl.Point(-10.7942286,-0.69615242), ocl.Point(-4.79422863,-9.69615242)))
path_id_1.append(ocl.Line(ocl.Point(-4.79422863,-9.69615242), ocl.Point(6,-9)))
paths.append(path_id_1)

path_id_2 = ocl.Path()
path_id_2.append(ocl.Line(ocl.Point(15,-14,0), ocl.Point(19.62435565,5.99038106,0)))
path_id_2.append(ocl.Line(ocl.Point(19.62435565,5.99038106,0), ocl.Point(4.62435565,19.99038106,0)))
path_id_2.append(ocl.Line(ocl.Point(4.62435565,19.99038106,0), ocl.Point(-15,14,0)))
path_id_2.append(ocl.Line(ocl.Point(-15,14,0), ocl.Point(-19.6243556,-5.99038106,0)))
path_id_2.append(ocl.Line(ocl.Point(-19.6243556,-5.99038106,0), ocl.Point(-4.62435565,-19.9903811,0)))
path_id_2.append(ocl.Line(ocl.Point(-4.62435565,-19.9903811,0), ocl.Point(15,-14,0)))
paths.append(path_id_2)

path_id_3 = ocl.Path()
path_id_3.append(ocl.Line(ocl.Point(-27,2,0), ocl.Point(-18,18,0)))
path_id_3.append(ocl.Arc(ocl.Point(-18,18,0),ocl.Point(-10.00000001,21.99999996,0),ocl.Point(-11.0434783,14.08695652,0),True))
path_id_3.append(ocl.Line(ocl.Point(-10,22,0), ocl.Point(10,23,0)))
paths.append(path_id_3)

for path in paths:
	# Retrieve a list of type/span pairs.  The type indicates whether it's a line or an arc
	spans = path.getTypeSpanPairs()
	print '\n\nSpan'

	for span in spans:
		span_type = span[0]
		span_element = span[1]

		if (span_type == ocl.ArcSpanType):
			arc = span_element
			print 'Arc(start=' + str(arc.p1) + ', centre=' + str(arc.c) + ' end=' + str(arc.p2) + ')'

		if (span_type == ocl.LineSpanType):
			line = span_element
			print 'Line(start=' + str(line.p1) + ', end=' + str(line.p2) + ')'

