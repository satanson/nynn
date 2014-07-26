消息通信中间件—nynn_mq

系统简介：在分布式系统，普遍存在向多个机器发布消息或者订阅来自多个机器消息的需求，
nynn_mq采用了PUB/SUB 通信模式，与zmq不同的是，nynn_mq socket即是PUB socket，
又是SUB sockt。nynn_mq的传输以任意长度的消息为单位。

特点：线程安全，全双工。

owned by ranpanf