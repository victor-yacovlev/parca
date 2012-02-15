import _base
import _util
import MatrixInfo

import sys
import os.path

def _print_usage_and_exit(code):
    sys.stderr.write("parca [--gel=REAL] [--matrix=STRING] [--out=FILENAME] FILE1 FILE2\n")
    sys.exit(code)
    
def _read_fasta(filename):
    if not os.path.exists(filename):
        sys.stderr.write("Error: File not exists: "+filename+"\n")
        sys.exit(4)
    f = open(filename, 'r')
    lines = f.read().split("\n")
    f.close()
    lines = map(lambda x: x.strip(), lines)
    lines = filter(lambda x: len(x)>0, lines)
    seqs = []
    for line in lines:
        if line.startswith(">"):
            name = line[1:].strip()
            seqs += [ (name,"") ]
        else:
            if len(seqs)==0: seqs += [("","")]
            curname,curseq = seqs[-1]
            part = line.replace('.','').replace(' ','').replace('-','')
            part = part.upper()
            curseq += part
            seqs[-1] = curname,curseq
    return seqs

if __name__=="__main__":
    file1name = None
    file2name = None
    gep = 1.0
    mn = "blosum62"
    out = sys.stdout
    for arg in sys.argv[1:]:
        if arg.startswith("--gep="):
            try:
                gep = float(arg[6:])
            except:
                sys.stderr.write("Error: --gep value is not real number\n")
                sys.exit(1)
            if gep<0:
                sys.stderr.write("Error: --gep value can not be negative\n")
                sys.exit(1)
        elif arg.startswith("--matrix="):
            mn = arg[9:].lower()
            if not mn in MatrixInfo.available_matrices:
                sys.stderr.write("Error: Unknown matrix name: "+mn+"\n")
                sys.exit(2)
            if not (mn.startswith("blosum") or mn.startswith("pam")):
                sys.stderr.write("Error: Only BLOSUM and PAM matrices supported\n")
                sys.exit(3)
        elif arg.startswith("--out="):
            outname = arg[6:]
            out = f.open(outname, 'w')
        elif arg.startswith("-"):
            _print_usage_and_exit(0)
        else:
            if file1name is None:
                file1name = arg
            else:
                file2name = arg
    if file1name is None:
        _print_usage_and_exit(0)
    seqs = _read_fasta(file1name)
    if len(seqs)==0:
        sys.stderr.write("Error: File '"+file1name+"' has no any sequences\n")
        sys.exit(5)
    name1, seq1 = seqs[0]
    if not file2name is None:
        seqs = _read_fasta(file2name)
        if len(seqs)==0:
            sys.stderr.write("Error: File '"+file2name+"' has no any sequences\n")
            sys.exit(5)
        name2, seq2 = seqs[0]
    else:
        if len(seqs)<2:
            sys.stderr.write("Error: File '"+file1name+"' has only one sequence\n")
            sys.exit(6)
        name2, seq2 = seqs[1]
    matrix = MatrixInfo.__dict__[mn]
    sys.stderr.write("Please wait...\n")
    als = _base.get_pareto_alignments(seq1,seq2,gep,matrix,40)
    primary = _util.get_primary_alignments(als)
    # TODO find common parts
    cps = None
    out.write("# Alignments by PARCA of '"+name1+"' and '"+name2+"'\n")
    for pa in primary:
        out.write("\n# Score = %i, Deletions = %i, Gaps = %i \n" % (pa.m, pa.d, pa.g))
        s = _util.alignment_to_string(seq1,seq2,pa.data,cps,matrix,60)
        out.write(s+"\n")
    out.close()
        