#!/bin/env python
# coding=utf-8
# список допустимых символов алфавита
AMINOS = "ARNDCQEGHILKMFPSTWYVBZX*"

# инициализация генератора случайных чисел
import random
random.seed()

# создание матрицы
MyCustomMatrix = dict()
for first in AMINOS:
    for second in AMINOS:
        # положительный вес за совпадение
        # и отрицательный в противном случае
        positive = random.random()
        negative = -positive
        # ключ словаря - это пара символов
        key = first, second
        if first!=second:
            MyCustomMatrix[key] = negative
        else:
            MyCustomMatrix[key] = positive
        # достаточно заполнить только половину 
        # матрицы, поскольку она симметрична
        if first==second: 
            break
