## ez_iot_os代码贡献指南

### 编程规范

参见：[编程规范](./ezos_coding_style.md)

### 贡献流程

​	ez_iot_os的代码仓库托管在github上，因此代码贡献者需要再github上注册账号才能贡献代码。

​	代码贡献流程如下：

```sequence
Title:代码贡献流程

Ezviz_OpenBiz/ez_iot_os->user/ez_iot_os:fork
user/ez_iot_os->PC:clone
PC->PC:modify
PC->user/ez_iot_os:push
user/ez_iot_os-->Ezviz_OpenBiz/ez_iot_os:pull request
Ezviz_OpenBiz/ez_iot_os->Ezviz_OpenBiz/ez_iot_os:check
Ezviz_OpenBiz/ez_iot_os->Ezviz_OpenBiz/ez_iot_os:merge
```

#### 1. fork

​	fork ez_iot_os源代码到自己github账户下。

![fork_1](.\figures\pull_request\1_fork_1.png)



![fork_2](.\figures\pull_request\2_fork_2.png)

#### 2. clone

​	将fork（自己账户下）的仓库clone到本地

![clone](.\figures\pull_request\3_clone.png)

#### 3. 提交修改到fork的仓库

​	将本地修改提交到fork（自己账户下）的仓库

#### 4. 提交Pull Request到ez_iot_os主仓库

- 创建pull request

![4_pull_request](.\figures\pull_request\4_pull_request.png)

- 确认要提交的信息

![](.\figures\pull_request\5_pull_request_2.png)

- 填写描述信息

![6_pull_request_3](.\figures\pull_request\6_pull_request_3.png)

- 提交pull request

#### 5. ez_iot_os管理员审核

​	ez_iot_os管理员审核通过之后，即可看到提交的更新