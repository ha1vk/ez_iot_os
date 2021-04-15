linux环境下执行如下；

sed -i "s/\r//" build.sh 或sed -i "s/^M//" build.sh, 直接将回车符替换为空字符串。