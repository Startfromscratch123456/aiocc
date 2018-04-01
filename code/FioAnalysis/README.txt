#
#------LuspinfAnalysis 使用说明-----------
#
一、LuspinfAnalysis设计
    目录结构
    ├─data
    │  ├─randread
    │  ├─randrw
    │  ├─randwrite
    │  ├─read
    │  ├─readwrite
    │  └─write
    ├─history
    ├─images
    │  ├─randread
    │  ├─randrw
    │  ├─randwrite
    │  ├─read
    │  ├─readwrite
    │  └─write
    ├─report
    └─  file_utils.py
        fio_document.py
        main.py
        print_util.py
        report_util.py
        statistic.py
        statistic_util.py
    data：中存储fio测试生成的测试结果文件
    history：备份一些重要的文件，没有实际的意义，未来的版本可能会被删除
    images：存放自动化生成报告的过程中产生的图片
    report：自动化生成的报告将被存储在这个目录，并以reportv时间 格式命令
    file_utils.py：读取原始的fio测试结果文件，并处理为fio_document中的对象
    fio_document.py：fio测试结果的数据结构
    main.py：运行这个文件开始自动分析流程，类似面向对象中的class Main
    print_util.py：方便测试的输出
    report_util.py：生成报告的工具
    statistic.py：根据statistic_util中平均化的结果绘图
    statistic_util.py：求fio_document对象的平均值


二、使用说明
将LuspinfTools测试的结果复制到data文件夹下，注意命令格式和文件夹的放置方式不能改变，文件夹的放置方式见bakeup下面的实例，运行main.py即可
