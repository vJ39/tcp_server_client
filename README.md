tcp_server_client
=================

jailシステムコールを使ったtcp server/clientのサンプル

##環境
FreeBSD 9.1-RELEASE-p7

##内容
tcpでリモート接続し、argをサーバー側で実行します。  
(IPアドレスとかchrootするディレクトリ名はソースに書いてあるので適宜変更してね)

##ソースのコンパイルのしかた

```
% gcc -Wall -o tcp_server tcp_server.c
% gcc -Wall -o tcp_client tcp_client.c
```

##サーバー環境での準備

chrootするディレクトリーの作成とbinとかのマウントをしときます。  

例）  
```
mkdir -p /jail/echo/{lib,libexec,bin,usr/bin,usr/local,usr/lib,usr/libexec,dev,sbin,usr/sbin}
mount_unionfs -oro /lib         /jail/echo/lib
mount_unionfs -oro /libexec     /jail/echo/libexec
mount_unionfs -oro /bin         /jail/echo/bin
mount_unionfs -oro /usr/bin     /jail/echo/usr/bin
mount_unionfs -oro /usr/local   /jail/echo/usr/local
mount_unionfs -oro /usr/lib     /jail/echo/usr/lib
mount_unionfs -oro /usr/libexec /jail/echo/usr/libexec
mount_unionfs -oro /dev         /jail/echo/dev
mount_unionfs -oro /sbin        /jail/echo/sbin
mount_unionfs -oro /usr/sbin    /jail/echo/usr/sbin
```

##サーバーの起動と終了

```
# ./tcp_server
Master Jail ID = 1360
# jail -r 1360
```

```クライアントからの接続
% ./tcp_client /bin/ls
bin
dev
echo_server.pl
lib
libexec
sbin
usr

% ./tcp_client /bin/ps axwww
  PID TT  STAT    TIME COMMAND
12475  2  RJ   0:00.00 /bin/ps axwww
```

#オプション

jailid・・・子プロセスのJAIL IDを返します

```jailid
% ./tcp_client jailid
JAIL ID = 1364
```

jailremove・・・子プロセスのJAIL IDを削除します

```jailremove
vmware% ./tcp_client /usr/bin/perl -e 'sleep 999'
jail(child)(create): File exists
^C
vmware% ./tcp_client jailid
jail(child)(create): File exists
JAIL ID = 1365
vmware% ./tcp_client jailremove 1365
jailremove 1365
```

#その他注意点

サーバーを起動するとワーカーが50プロセスぐらいたちあがります。  
最大プロセス数、仮想メモリーサイズ、CPU時間に制限をくわえてあります。  
jailは、kern.securelevel = 2で走ります。  
子JAIL最大数も制限をくわえてあります。  
ほかに、いらんファイルもありますけど、気にしないでください。  
気が向いたら勝手に更新します。
