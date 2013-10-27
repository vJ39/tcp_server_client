tcp_server_client
=================

jailシステムコールを使ったtcp server/clientのサンプル

#環境
FreeBSD 9.1-RELEASE-p7

#内容
tcpでリモート接続し、argをサーバー側で実行します。

```サーバー側
# ./tcp_server
```

```クライアント側
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

#サーバーの起動と終了

```
# ./tcp_server
Master Jail ID = 1360
# jail -r 1360
```

#その他

最大プロセス数、仮想メモリーサイズ、CPU時間に制限をくわえてあります。  
jailは、kern.securelevel = 2で走ります。
子JAIL最大数も制限をくわえてあります。
