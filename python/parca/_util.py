A = 20.0

def __calculate_extrs(alist):
    tgS = [ (0, 0.0) ]
    for i in range(1, len(alist)-1):
        m_l = alist[i-1].m
        m = alist[i].m
        m_r = alist[i+1].m
        g_l = alist[i-1].g
        g = alist[i].g
        g_r = alist[i+1].g
        dM_l = float(m - m_l)
        dG_l = float(g - g_l)
        dM_r = float(m_r - m)
        dG_r = float(g_r - g)
        dMdG_l = dM_l / dG_l
        dMdG_r = dM_r / dG_r
        tg_l = dMdG_l / A
        tg_r = dMdG_r / A
        tg_point = (tg_l - tg_r) / (1 + tg_l * tg_r)
        tgS += [ (i, tg_point) ]
    
    extrs = []    
    
    for i in range(1, len(tgS)-1):
        number_l, tg_l = tgS[i-1]
        number  , tg   = tgS[i  ]
        number_r, tg_r = tgS[i+1]
        if tg>tg_l and tg>tg_r:
            extrs += [ (number, tg) ]
    if len(extrs)==0:
        extrs = tgS
        if len(extrs)<len(alist):
            extrs += [ (len(extrs), 0.0) ]
        assert len(extrs)==len(alist)
    return extrs

def get_primary_alignments(alist):
    extrs = __calculate_extrs(alist)
    extrs.sort(key=lambda value: value[1], reverse=True)
    result = []   
    for number, value in extrs:
        result += [alist[number]]
    return result
    
def get_primary_points(alist):
    return __calculate_extrs(alist)
    
def calculate_gops(alist):
        minGOP = [ 0.0 ] * len(alist)
        maxGOP = [ 99999999.9 ] * len(alist)
        g = 0
        while g < len(alist)-1:
            g += 1
            for c in range(g-1, -1, -1): # range [g-1, g-2, ... , 0]
                deltaW = alist[g].m - alist[c].m
                deltaG = alist[g].g - alist[c].g
                nS = float(deltaW) / float(deltaG)
                if nS < maxGOP[c]:
                    t = c
                    break
                else:
                    maxGOP[c] = None
                    minGOP[c] = None
            maxGOP[g] = nS
            minGOP[t] = nS
        result = []
        for i in range(0, len(alist)):
            result += [(minGOP[i],maxGOP[i])]
        return result
    
def alignment_to_string(source, mutant, 
                        alignment, other=None, weights=None, width=60):
    line1 = ""
    line2 = ""
    line3 = ""
    line4 = ""
    line5 = ""
    for i1, i2 in alignment:
        assert i1!=-1 or i2!=-1
        if i1!=-1 and i2!=-1:
            line1 += source[i1]
            line3 += mutant[i2]
            if source[i1]==mutant[i2]:
                line2 += "|"
            else:
                a = source[i1]
                b = source[i2]
                w = None
                if not weights is None:
                    if weights.has_key((a,b)):
                        w = weights[(a,b)]
                    elif weights.has_key((b,a)):
                        w = weights[(b,a)]
                if not w is None:
                    if w>=0:
                        line2 += ":"
                    else:
                        line2 += "."
                else:
                    line2 += " "
                
        elif i1==-1:
            line1 += "-"
            line2 += " "
            line3 += mutant[i2]
        elif i2==-1:
            line1 += source[i1]
            line2 += " "
            line3 += "-"
        if not other is None:
            if (i1, i2) in other:
                line4 += "*"
            else:
                line4 += " "

    result = ""
    if width>0:
        fpe = 0
        spe = 0
        fp = 0
        sp = 0
        index = 0
        while len(line1)>0:
            line1_part = line1[0:width-20]
            line2_part = line2[0:width-20]
            line3_part = line3[0:width-20]
            
            if not other is None:
                line4_part = line4[0:width-20]
                
            if not weights is None:
                line5_part = line5[0:width-20]
                line5_part = line5[0:width-20]
                
            fp = fpe + 1
            sp = spe + 1
            fpe = fp + len(line1_part)-line1_part.count("-")
            spe = sp + len(line3_part)-line3_part.count("-")
            result += str(fp).rjust(6)+" "*2+line1_part.ljust(width-20) + \
                "  "+ str(fpe)+"\n"
            result += " "*8+line2_part.ljust(width-20) + "\n"
            result += str(sp).rjust(6)+" "*2+line3_part.ljust(width-20) + \
                "  "+ str(spe)+"\n"
            if not other is None:
                result += " "*8+line4_part+"\n"
            if not weights is None:
                result += " "*8+line5_part+"\n"
            result += "\n"
            sp = spe
            fp = fpe
            line1 = line1[width-20:]
            line2 = line2[width-20:]
            line3 = line3[width-20:]
            if not other is None:
                line4 = line4[width-20:]
            if not weights is None:
                line5 = line5[width-20:]
            index += width-20
    else:
        result = line1+"\n"+line2+"\n"+line3+"\n"
        if not other is None:
            result += line4 + "\n"
        if not weights is None:
            result += line5 + "\n"
        result += "\n"
    return result
    
def get_common_part(alignments_list):
    assert len(alignments_list)>0
    result = None
    for al in alignments_list:
        if result is None:
            result = set(al)
        else:
            result = result & set(al)
    return result

