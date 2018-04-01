Automatic I/O congestion control(AIOCC)
=========================
[![TeamCity CodeBetter](https://img.shields.io/teamcity/codebetter/bt428.svg?maxAge=2592000)]()
[![Packagist](https://img.shields.io/packagist/v/symfony/symfony.svg?maxAge=2592000)]()
[![Yii2](https://img.shields.io/badge/Powered_by-multexu_Framework-green.svg?style=flat)]()
[![Progress](http://progressed.io/bar/80?title=completed)]()


# [访问AIOCC可视化监控平台](http://www.aiocc.xyz:3000)

Lustre是高性能计算领域杰出的并行分布式文件系统，理论上数据节点可以扩展到千节点规模，支撑十万左右的客户端节点并行I/O操作，在实践中表现出极好的扩展性和聚合性。但是，Lustre集群达到较大规模时，会有大量应用通过I/O请求竞争资源，出现类似网络拥塞的I/O拥塞，无法保障服务质量。Lustre的最初设计对大规模下导致的拥塞问题考虑不足，因此，必须对这些请求进行有效管控，以提升集群整体的效率。手动地对集群的存储系统进行I/O拥塞控制在HPC领域的实际应用中极不现实，尤其是存储系统的规模达到艾级（Exascale）时，实现自动I/O拥塞控制，进行全局自适应服务质量研究十分必要。

AIOCC是一种端到端自动I/O拥塞控制机制，从Lustre客户端和服务端对集群I/O请求的发送和处理进行控制，针对不同类型的负载，使用深度增强学习方法优化调控的参数。在AIOCC的调控下，Lustre能够根据调控目标和集群当前的运行状况自动调整相关控制参数，服务端通过令牌桶策略给应用分配服务机会，并将分配的情况反馈到客户端，同时客户端基于应用粒度及时同态调整I/O请求的发送窗口、速率和分配策略。避免Lustre集群由于客户端I/O请求发送过快或者服务端对I/O请求处理不当挂起大量I/O请求而崩溃。AIOCC实现自动管理Lustre集群中的资源竞争，缓解集群I/O拥塞，在不导致拥塞的前提下，最大化实现提高系统的吞吐和降低系统性能偏差这两个目标之间的平衡。

通过实验评估，AIOCC能有效的应对Lustre集群中的部分负载导致的I/O拥塞问题，提高系统吞吐最高达到40%，降低吞吐的性能偏差35%，有效降低I/O任务完成时延达24%。

# AIOCC的特点

- 从服务端和客户端同时着手，实现端到端的拥塞控制；
- 在客户端基于应用粒度分配I/O发送窗口，保证应用间公平性，控制请求的发送数量和速度，充分利用了服务资源，又尽可能避免服务端的拥塞；
- 使用机器学习方法来优化整个调控过程：根据负载的特征对负载进行分类，并用深度学习的方法优化整个调节过程。

我们Lustre 2.9中实现了本文方案的原型AIOCC，通过全面测试论证方案效果并给出了全面的分析。此外，构建了Lustre和AIOCC自动开发、安装和测试的套件，在开源平台Github上贡献了所有源码。

# AIOCC架构

![image](https://github.com/ShijunDeng/aiocc/blob/master/source/image/architecture_aiocc.png)

**AIOCC安装&使用说明**

AIOCC说明是在Lustre2.9+CenOS7上实现原型系统,并在[MULTEXU](https://github.com/ShijunDeng/multexu)、[LustreTools](https://github.com/ShijunDeng/LustreTools)、[ASCAR](https://github.com/mlogic/ascar-lustre-sharp)、CAPES基础上开发的。AIOCC针对的是CentOS7（Linux kernel 3.10.0-514.el7.x86_64）和Lustre2.9.0，其它版本的系统使用本工具可能需要解决一些兼容性问题。另外，CentOS7在安装过程中，选择的版本和安装配置不同，也可能导致一些包的依赖性问题，因此建议CentOS7的安装过程参照视频教程进行安装。AIOCC自带Lustre2.9.0全套安装文件。安装和测试参照[技术文档说明](https://github.com/ShijunDeng/aiocc/tree/master/document)、[MULTEXU](https://github.com/ShijunDeng/multexu)、[LustreTools](https://github.com/ShijunDeng/LustreTools)中相关步骤。

温馨提示：由于Lustre的需要对内核打补丁，直接在物理上进行AIOCC测试可能出现某些操作问题、软件兼容性问题导致的内核崩溃，建议您先使用虚拟机节点构建AIOCC，熟悉相关步骤之后，再在物理机进行测试。对于CentOS的安装最好和我们的视频教程中的保持一致。如果您有任何问题和建议，非常欢迎同我联系。


**AIOCC可视化监控使用**

AIOCC开放了一个展示系统，访问地址193.112.37.106:3000或者http://www.aiocc.xyz:3000，用户名和密码分别是aiocc和aiocc2018。登录之后，点击左上角的HOME，再到AIOCC目录下查看相关监控情况。



## 反馈与建议
- QQ：946057490
- 邮箱：<dengshijun1992@gmail.com>

---------
感谢您阅读这份帮助文档。