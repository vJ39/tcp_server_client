use strict;
use warnings;
use Socket;
 
my $port = 9001;
socket(CLIENT_WAITING, PF_INET, SOCK_STREAM, 0) or die $!;
setsockopt(CLIENT_WAITING, SOL_SOCKET, SO_REUSEADDR, 1) or die $!;
bind(CLIENT_WAITING, pack_sockaddr_in($port, INADDR_ANY)) or die $!;
listen(CLIENT_WAITING, SOMAXCONN) or die $!;
while (1){
    my $paddr = accept(CLIENT, CLIENT_WAITING);
    my ($client_port, $client_iaddr) = unpack_sockaddr_in($paddr);
    my $client_hostname = gethostbyaddr($client_iaddr, AF_INET) || "";
    my $client_ip = inet_ntoa($client_iaddr);
    select(CLIENT); $|=1; select(STDOUT);
    while (<CLIENT>){
        print CLIENT $_;
    }
    close(CLIENT);
}
