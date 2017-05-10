# unittest

## 介绍
##### 功能
- **编译源代码和测试代码**
- **生成代码覆盖率报告**

##### 细节
- 可生成两个可执行文件，分别是server_test和unit_test
- server_test是开发代码编译后生成的可执行文件，可代替build.sh生成的可执行文件来执行功能测试；与build.sh生成的文件不同的是，执行过后可产生代码覆盖率信息，以便生成代码覆盖率报告
- unit_test是CODE_TEST目录下的测试代码编译后生成的可执行文件，执行单元测试
- 单元测试代码可参考分支mxrsessionapply中CODE_TEST目录下的示范代码




## 使用
- 配置

```
将该项目clone到某个目录下
# git clone https://github.com/512444693/unittest.git

示例目录结构如下:
.
├── build.sh
├── conf
├── env.sh
├── include
├── install
├── logs
├── obj
├── src
├── start.sh
├── stat
├── test
└── unittest

配置Makefile.comm示例:
#绝对路径
BASEBASEROOT=/opt/p2p_server/branches/matrix/server

#被测试程序的主目录
SERVER_ROOT = ${BASEBASEROOT}/reallive/mxrsessionapply

#生成测试报告目录
HTML_DIR = ./gtest_report
```

- 生成两个可执行文件
```
# make
```
- 单独生成某个可执行文件
```
# make server_test或unit_test
```
- 功能测试
```
生成server_test后会自动复制到程序目录下，不需要手动复制

启动程序
# make start

关闭程序
# make kill
```
- 单元测试
```
执行
# ./unit_test
```
- 生成报告(没有进行过测试会生成失败)
```
# make report
```


## 建议
在执行功能测试时建议使用以下命令

```
valgrind -v --tool=memcheck --leak-check=full ./server_test -l >> valReport 2>&1
```
这样在执行完毕后即可以生成代码覆盖率信息也可以生成内存泄漏报告

## 编译gtest/gmock
- 由于gmock代码中包含gtest，所以只要编译gmock即可

1. 进入gmock-1.7.0/make
2. 执行命令make gmock.a（默认生成gmock_main.a，包含main函数，这个项目需要自己编译），将gmock.a静态库复制到/usr/local/lib目录下，cp gmock.a /usr/local/libgmock.a
3. 将包含main函数的程序入口gmock-1.7.0/src/gmock-all.cc复制到需要的地方（这个示例复制到了CODE_TEST目录下）
4. 将gmock和gtest的头文件复制到/usr/local/include目录中，cp -r gmock-1.7.0/include/gmock /usr/local/include;cp -r gtest-1.7.0/include/gtest /usr/local/include
