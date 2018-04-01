# -*- coding: utf-8 -*-
import  random

start=108.04

data_num=144

index=1
print(start)
while index < 12:
    print(start*1.2+(random.random()-0.8)*20)
    index+=1

while index < 24:
    print(start*1.3+(random.random()-0.6)*10)
    index+=1

while index < 36:
    print(start*1.3+(random.random()-0.5)*7)
    index+=1


while index < 48:
    print(start*1.4+(random.random()-0.4)*5)
    index+=1
