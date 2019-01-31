import os



_scale = {'kB': 1024.0, 'mB': 1024.0*1024.0,
          'KB': 1024.0, 'MB': 1024.0*1024.0}

def _VmB(VmKey):
    '''Private.
    '''
    _proc_status = '/proc/%d/status' % os.getpid()
    #global _proc_status, _scale
    global _scale
     # get pseudo file  /proc/<pid>/status
    try:
        t = open(_proc_status)
        v = t.read()
        t.close()
    except:
        print("procmemory.py ERROR!")
        return 0.0  # non-Linux?
    #print v
    # get VmKey line e.g. 'VmRSS:  9999  kB\n ...'
    i = v.index(VmKey)
    v = v[i:].split(None, 3)  # whitespace
    if len(v) < 3:
        return 0.0  # invalid format?
     # convert Vm value to bytes
    return float(v[1]) * _scale[v[2]]


def memory(since=0.0):
    '''Return memory usage in bytes.
    '''
    return _VmB('VmHWM:') - since

def data(since=0.0):
    '''Return memory usage in bytes.
    '''
    return _VmB('VmExe:') - since

def resident(since=0.0):
    '''Return resident memory usage in bytes.
    '''
    return _VmB('VmRSS:') - since


def stacksize(since=0.0):
    '''Return stack size in bytes.
    '''
    return _VmB('VmStk:') - since
    
if __name__ == "__main__":  
    m1 = memory()
    print(m1)
    l=[]
    for n in range(100000):
        l.append(n)
    m2 = memory()
    
    print(m2/_scale['kB'], 'kB')
    raw_input("Press Enter to terminate") 
