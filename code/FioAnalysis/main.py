# -*- coding: utf-8 -*-
from sklearn.cluster import KMeans  # 导入Kmeans算法包
from sklearn.metrics import silhouette_score  # 计算轮廓系数
import matplotlib.pyplot as plt  # 画图工具
from WorkloadClassify import WorkloadClassify
from FileUtils import *

colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k', 'b']
markers = ['o', 's', 'D', 'v', '^', 'p', '*', '+']

i = 2
clusters = [i]
while i < 30:
    i += 1
    clusters.append(i)
sc_scores = []

fio_filter("data")
atributes = WorkloadClassify().load_data("data")
row = len(atributes)
col = len(atributes[0])

print("[", row, "x", col, "] sized input")

for t in clusters:
    kmeans_model = KMeans(n_clusters=t).fit(atributes)

    sc_score = silhouette_score(atributes, kmeans_model.labels_, metric='euclidean')
    sc_scores.append(sc_score)
    plt.title('K=%s,silhouette coefficient=%0.03f' % (t, sc_score))

# 绘制轮廓系数与不同类簇数量的直观显示图
print(sc_scores)

for s in sc_scores:
    print(s)
plt.plot(clusters, sc_scores, '*-')
plt.xlabel('Numbers of clusters')
plt.ylabel('Silhouette Coefficient score')
plt.show()
#=("randrw" "readwrite" "write" "randwrite" "read" "randread")
