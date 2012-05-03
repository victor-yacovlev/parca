import _parca
import os
import os.path

from MatrixInfo import *

class alignment:
    m = int()
    g = int()
    d = int()
    data = list()
    def __init__(self, c_al):
        self.m = c_al.m
        self.d = c_al.d
        self.g = c_al.g
        self.data = []
        for a, b in c_al.data:
            aa = int(a)
            bb = int(b)
            self.data += [(aa,bb)]

def _shift_matrix(matrix, shift):
    r = dict()
    for key in matrix.keys():
        val = matrix[key]
        val += shift
        r[key] = int(val)
    return r

_aligner = _parca.aligner(40, "parca-"+str(os.getpid()), 350)

def _utf8_matrix_keys(matrix):
    return matrix
    r = dict()
    for a, b in matrix.keys():
        val = matrix[a,b]
        ua = a.encode('utf-8')
        ub = b.encode('utf-8')
        key = ua, ub
        r[ua,ub] = val
    return r

def set_score_matrix(matrix):
    global _aligner
    _aligner.set_score_matrix(_utf8_matrix_keys(matrix))

def process_direct_stage(sequence1, sequence2,
                         gep=1.0,
                         matrix=blosum62,
                         limit=40):
    global _aligner
    _aligner.init(limit, "parca-"+str(os.getpid()), 350)
    shift = int(gep*2)
    set_score_matrix(_shift_matrix(matrix,shift))
    _aligner.process_direct_stage(sequence1,sequence2)
    cnt = _aligner.result_count()
    r = []
    for i in range(0,cnt):
        r += [_aligner.get_alignment_info(i)]
    return r
    
def process_backward_stage(no):
    global _aligner
    return _aligner.get_alignment(no).data

def get_pareto_alignments(sequence1, sequence2,
                          gep=1.0,
                          matrix=blosum62,
                          limit=40):
    global _aligner
    process_direct_stage(sequence1,sequence2,gep,matrix,limit)
    cnt = _aligner.result_count()
    r = []
    for i in range(0,cnt):
        c_al = _aligner.get_alignment(i)
        r += [alignment(c_al)]
    return r

def set_temporary_directory(dirname):
    global _aligner
    normpath = os.path.abspath(dirname)
    _aligner.set_temporary_directory(normpath.encode("utf-8"))

def get_last_error():
    global _aligner
    return _aligner.get_last_error()
    
# perform self-tests on import
#_aligner.selftest_matrix_1()
#_aligner.selftest_matrix_1()
#_aligner.selftest_matrix_1()

