import ocl
import camvtk
import time
import vtk
import datetime
import math
import gzip
import csv

def TSPLIBReader( filename ):
    """ read a .tsp format TSPLIB file, and return list of points """
    #infile = open(filename,"r")
    infile = gzip.open(filename, 'rb')
    points = []
    found = 0
    n = 1
    for line in infile.readlines():
        if not found:
            for word in line.split():
                if word == "NODE_COORD_SECTION":
                    found = 1
        else:
            words = line.split()
            if len(words)!=0 and words[0]!="EOF" :
                points.append( ocl.Point( float( words[1] ),  float( words[2] ) ) )
            
                
    print("read ",len(points)," points from ", filename)
    return points
    
    
if __name__ == "__main__":  
    
    tsplist = [ ["a280" ,  2579 ], 
                ["berlin52" , 7542 ],
                ["bier127" , 118282 ],
                ["brd14051" , 469385 ],
                ["ch130" , 6110 ],
                ["ch150" , 6528 ],
                ["d198" , 15780 ],
                ["d493" , 35002 ],
                ["d657" , 48912 ],
                ["d1291" , 50801],
                ["d1655" , 62128],
                ["d2103" , 80450],
                ["d15112" , 1573084],
                ["d18512" , 645238],
                ["eil51" ,  426 ],
                ["eil76" ,  538 ],
                ["eil101" ,  629 ],
                ["fl417" ,  11861 ],
                ["fl1400" ,  20127 ],
                ["fl1577" ,  22249 ],
                ["fl3795" ,  28772 ],
                ["fnl4461" ,  182566 ],
                ["gil262" ,  2378 ],
                ["kroA100" ,  21282 ],
                ["kroB100" ,  22141],
                ["kroC100" ,  20749],
                ["kroD100" ,  21294],
                ["kroE100" ,  22068],
                ["kroA150" ,  26524],
                ["kroB150" ,  26130],
                ["kroA200" ,  29368],
                ["kroB200" ,  29437],
                ["lin105" ,  14379],
                ["lin318" ,  42029],
                ["linhp318" ,  41345],
                ["nrw1379" ,  56638],
                ["p654" ,  34643],
                ["pcb442" , 50778],
                ["pcb1173" , 56892],
                ["pcb3038" , 137694],
                ["pr76" , 108159],
                ["pr107" , 44303],
                ["pr124" , 59030],
                ["pr136" , 96772],
                ["pr144" , 58537],
                ["pr152" , 73682],
                ["pr226" , 80369],
                ["pr264" , 49135],
                ["pr299" , 48191],
                ["pr439" , 107217],
                ["pr1002" , 259045],
                ["pr2392" , 378032],
                ["rat99" , 1211] ,
                ["rat195" , 2323],
                ["rat575" , 6773],
                ["rat783" , 8806],
                ["rd100" , 7910],
                ["rd400" , 15281],
                ["rl1304" , 252948],
                ["rl1323" , 270199],
                ["rl1889" , 316536],
                ["rl5915" , 565530], # 22 s
                ["rl5934" , 556045], # 22 s
                ["rl11849" , 923288], # 93 s
                ["st70" , 675],
                ["ts225" , 126643],
                ["tsp225" , 3916],
                ["u159" , 42080],
                ["u574" , 36905],
                ["u724" , 41910],
                ["u1060" , 224094],
                ["u1432" , 152970],
                ["u1817" , 57201],
                ["u2152" , 64253],
                ["u2319" , 234256],
                ["vm1084" , 239297], 
                ["usa13509" , 19982859],
                ["vm1748" , 336556]
                ]
    
    tsplist2=[]
    for t in tsplist:
        tspname = t[0]
        tspfile = "tsplib/"+tspname+".tsp.gz"
        pts = TSPLIBReader( tspfile )
        inst=[]
        inst.append(len(pts)) # length
        inst.append(t[0])     # name
        inst.append(t[1])     # optimal
        inst.append(pts)      # points
        tsplist2.append(inst)
    tsp_sorted = sorted(tsplist2) 
    
    Nmax=65  #60(1577)   65(2103)
    results=[]
    #for n in range(0,Nmax): # len(tsp_sorted)):
    for n in range(0, len(tsp_sorted)):
        resultrow=[]
        tsp = ocl.TSPSolver()
        inst = tsp_sorted[n]
        for p in inst[3]:
            tsp.addPoint( p.x , p.y)
        start_time = time.time()
        tsp.run()
        run_time = time.time() - start_time
        l = tsp.getLength()
        ratio = float(l)/float(inst[2])
        print(n," : ",inst[1],": ",ratio," l=",l," / ",inst[2], " / ",run_time, "seconds")
        resultrow.append( inst[0] ) # length
        resultrow.append( inst[2] ) # optimal
        resultrow.append( l ) # found tour length
        resultrow.append( ratio ) # found tour length
        resultrow.append( run_time )
        results.append(resultrow)
    
    # write time and length results to file
    
    writer = csv.writer(open('tsp_results.csv', 'wb'), delimiter=',',quotechar='|', quoting=csv.QUOTE_MINIMAL)
    for row in results:
        writer.writerow( row )



