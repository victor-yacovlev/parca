#!/bin/env python
# coding=utf-8

# подключение модуля PARCA
from parca import *

# последовательности в примере приведены 
# из базы данных PREFAB-P:
# S1 - это 1e29A, S2 - 1ctj
S1 = "ELTESTRTIPLDEAGGTTTLTARQFTNGQKIFVDTCT" + \
     "QCHLQGKTKTNNNVSLGLADLAGAEPRRDNVLALVEF" + \
     "LKNPKSYDGEDDYSELHPNISRPDIYPEMRNYTEDDI" + \
     "FDVAGYTLIAPKLDERWGGTIYF"
S2 = "EADLALGKAVFDGNCAACHAGGGNNVIPDHTLQKAAI" + \
     "EQFLDGGFNIEAIVYQIENGKGAMPAWDGRLDEDEIA" + \
     "GVAAYVYDQAAGNKW"


# построение Парето-оптимальных выравниваний
pareto = get_pareto_alignments(S1,S2,0.0,pam240)

# выделение среди них основных выравниваний
primary = get_primary_alignments(pareto)
# выбор первых шести
primary = primary[0:6]

# выделение множеств пар сопоставленных символов
als = map(lambda x: x.data, primary)

# получение общей части
common = get_common_part(als)


# вывод на экран
for val in primary:
    print "MatchScore: ", val.m
    print "Deletions: ", val.d
    print "Gaps: ", val.g
    print "Alignment:"
    print alignment_to_string(S1,S2,
                              val.data,
                              common,
                              pam240)
