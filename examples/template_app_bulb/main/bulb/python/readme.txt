1.在外部控制台下载物模型文件，命名为profile.json
2.执行脚本python profile_json2c.py 

note：
1.脚本会自动遍历profile 文件所有的功能点生成对应的函数，同时加上文件头以及文件尾固定模板
2.生成的c文件完善注释代码中的提示后，删除打印