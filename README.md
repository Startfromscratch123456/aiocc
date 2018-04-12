Automatic I/O congestion control(AIOCC)
=========================
[![TeamCity CodeBetter](https://img.shields.io/teamcity/codebetter/bt428.svg?maxAge=2592000)]()
[![Packagist](https://img.shields.io/packagist/v/symfony/symfony.svg?maxAge=2592000)]()
[![Yii2](https://img.shields.io/badge/Powered_by-multexu_Framework-green.svg?style=flat)]()
[![Progress](http://progressed.io/bar/80?title=completed)]()




# 为什么会有AIOCC
并行分布式文件系统中，数据节点可以扩展到千节点规模，支撑十万左右的客户端节点并行I/O操作，在实际应用中表现出很好的扩展性和聚合性。但是，当节点达到一定规模时，大量I/O请求竞争I/O资源，造成类似网络拥塞的I/O拥塞，导致I/O吞吐率下降和I/O延迟不可控。我们以Lustre分布式文件系统为应用场景代表，对I/O拥塞控制进行研究。现有Lustre设计对大规模集群下的I/O拥塞问题考虑不足，没有对I/O请求进行有效管控，使得集群整体的效率不高。手动地对集群的存储系统进行I/O拥塞控制在HPC领域的实际应用中不够灵活，尤其是当存储系统的规模达到艾级（Exascale）时，更是难以实现。因此，有必要对自动I/O拥塞控制机制进行研究，保证全局自适应服务质量。亚马逊和卡耐基梅隆大学的 Dana Van Aken和Andy Pavlo等人设计了OtterTune对MySQL数据库管理系统（DBMS， Database Management System）进行自动参数调优。OtterTune针对数据库系统中不同的负载，用机器学习方法分析影响调控目标（如I/O吞吐率和I/O延迟）的关键参数并进行参数调优，将优化后的DBMS参数按负载类别进行存储，形成一个智能的DBMS。DBMS通过OtterTune进行参数调优后，能使MySQL处理事务的延迟下降大约60%，I/O吞吐率提升22% 到35%。OtterTune为Lustre分布式文件系统自动化的I/O拥塞控制提供了一个成功的范例。AIOCC正是在这种背景下，应运而生。

AIOO是一种完全自动I/O拥塞控制机制，在Lustre客户端和服务端使用深度Q-learning方法优化I/O拥塞控制参数。AIOCC可以根据Lustre集群当前的运行状况，自动地调整服务端请求调度参数和客户端请求发送窗口控制参数，降低Lustre集群应用中I/O拥塞对系统I/O吞吐率和I/O延迟性能的影响。

初步的实验结果表明，AIOCC能大大降低Lustre集群中由I/O拥塞对I/O吞吐率和I/O延迟性能造成的影响。对比现有Lustre集群系统，AIOCC能使写操作为主的负载I/O吞吐率提高34.82%，I/O延迟降低26.17%。尽管AIOCC现在还不十分完善，我们相信，经过后续的优化和改进，AIOCC将有更令人振奋的表现！


# AIOCC的实现机制

- 针对服务端I/O RPC请求调度问题，设计一种以提高应用的I/O效率为调度目标的调度策略，并考虑服务端的资源状况，对服务端的I/O请求进行调度；使用TBF策略以分配令牌的方式实施调度过程，并将分配值反馈给客户端；
- 针对客户端I/O RPC请求发送窗口控制问题，设计一种自动调整I/O请求发送窗口的方案，根据服务端的反馈和拥塞状况，动态调整I/O请求发送的数量和速率；
- 针对I/O拥塞控制模型中需要动态调整的控制参数，使用基于深度Q-learning方法，针对不同类型的负载来训练调节参数，优化调节过程。实现自动I/O拥塞控制，提高系统的I/O吞吐率，降低I/O延迟；

基于Lustre 2.9和TensorFlow1.0中实现了本文方案的原型AIOCC，通过全面测试论证方案效果并给出了全面的分析。此外，构建了Lustre和AIOCC自动开发、安装和测试的套件，在平台上贡献了所有源码和开发过程中一些设计图、笔记。

# AIOCC架构

![image](https://github.com/ShijunDeng/aiocc/blob/master/source/image/architecture_aiocc.png)

**AIOCC安装&使用说明**

- AIOCC说明是在Lustre2.9+CenOS7上实现原型系统,并在[MULTEXU](https://github.com/ShijunDeng/multexu)、[LustreTools](https://github.com/ShijunDeng/LustreTools)基础上开发的。AIOCC针对的是CentOS7（Linux kernel 3.10.0-514.el7.x86_64）和Lustre2.9.0，其它版本的系统使用本工具可能需要解决一些兼容性问题
- CentOS7在安装过程中，选择的版本和安装配置不同，也可能导致一些包的依赖性问题，因此建议CentOS7的安装过程参照视频教程进行安装。AIOCC自带Lustre2.9.0全套安装文件。安装和测试参照[技术文档说明](https://github.com/ShijunDeng/aiocc/tree/master/document)、[MULTEXU](https://github.com/ShijunDeng/multexu)、[LustreTools](https://github.com/ShijunDeng/LustreTools)中相关步骤

- 由于Lustre的需要对内核打补丁，直接在物理上进行AIOCC测试可能出现某些操作问题、软件兼容性问题导致的内核崩溃，建议您先使用虚拟机节点构建AIOCC，熟悉相关步骤之后，再在物理机进行测试。对于CentOS的安装最好和我们的视频教程中的保持一致。如果您有任何问题和建议，非常欢迎同我联系。全部文件的下载地址，参见[LustreTools](http://pan.baidu.com/s/1gfDkj7P)下载
- [document](https://github.com/ShijunDeng/aiocc/tree/master/document)目录下贡献了相关文档和开发笔记，欢迎阅读和交流


**AIOCC可视化监控使用**

AIOCC开放了一个展示系统：

- 访问地址193.112.37.106:3000或者http://www.aiocc.xyz:3000
- 用户名和密码分别是aiocc和aiocc2018，登录之后，点击左上角的HOME，再到AIOCC目录下查看相关监控情况。



## 反馈与建议
- QQ：946057490
- 邮箱：<dengshijun1992@gmail.com>

---------
感谢您阅读这份文档。