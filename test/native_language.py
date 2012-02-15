#!/bin/env python
# coding=utf-8

# подключение модуля PARCA
from parca import *

# алфавит
ALPH_A = u"аеёиоуыэюяi"
ALPH_B = u"бвгджзйклмнпрстфхцчшщьъ"
ALPH_S = " ,:!.?-;"
ALPHABET = ALPH_A+ALPH_B+ALPH_S

# создаем матрицу весов
matrix = dict()
for a in ALPHABET:
    for b in ALPHABET:
        if a==b:
            # точное совпадение
            matrix[a,b] = 50
        elif a in ALPH_A and b in ALPH_A:
            # сопоставление гласных
            matrix[a,b] = 20
        elif a in ALPH_B and b in ALPH_B:
            # сопоставление согласных
            matrix[a,b] = 10
        elif a in ALPH_S and b in ALPH_S:
            # сопоставление разделителей
            matrix[a,b] = 150
        elif not a in ALPH_S and not b in ALPH_S:
            # сопоставление букв
            matrix[a,b] = 2
        else:
            # прочее сопоставление
            matrix[a,b] = 0

RU = u"это простой текст на естественном языке"
UA = u"це простий текст на природнiй мовi"

# построение Парето-оптимальных выравниваний
pareto = get_pareto_alignments(RU,UA,1.0,matrix)

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
    print alignment_to_string(RU,UA,
                              val.data,
                              common,
                              matrix)
