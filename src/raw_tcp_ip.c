#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

#include "cdma_p2p_common.h"

struct ethhdr_t {
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
};

// pseudo_header ,for tcp crc
struct pseudo_header {
	uint32_t source_address;
	uint32_t dest_address;
	uint8_t placeholder;
	uint8_t protocol;
	uint16_t tcp_length;
};

struct iphdr_t {
    unsigned char ihl:4;        
    unsigned char version:4;   
    unsigned char tos;         
    unsigned short tot_len;    
    unsigned short id;        
    unsigned short frag_off;   
    unsigned char ttl;         
    unsigned char protocol;    
    unsigned short check;       
    unsigned char saddr[4];
    unsigned char daddr[4];
};

void custom_inet_addr(const char *ip_str, unsigned char *ip_addr) {
    int a, b, c, d;
    if (sscanf(ip_str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
        ip_addr[0] = (unsigned char) a;
        ip_addr[1] = (unsigned char) b;
        ip_addr[2] = (unsigned char) c;
        ip_addr[3] = (unsigned char) d;
    } else {
        fprintf(stderr, "Invalid IP address format\n");
    }
}

void set_iphdr_addresses(struct iphdr_t *header, const char *src_ip, const char *dest_ip) {
    custom_inet_addr(src_ip, header->saddr);
    custom_inet_addr(dest_ip, header->daddr);
}

uint16_t checksum(void *vdata, size_t length) {
	char *data = (char *)vdata;
	unsigned long sum = 0;

	for (; length > 1; length -= 2) {
		unsigned short word;
		memcpy(&word, data, 2);
		sum += word;
		data += 2;
	}

	if (length == 1) {
		unsigned short word = 0;
		memcpy(&word, data, 1);
		sum += word;
	}

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (uint16_t)(~sum);
}

// uint32_t custom_inet_addr(const char *ip_str) {
//     uint32_t ip = 0;
//     int segs[4]; // 存储IP地址的四个部分
//     printf("custom_inet_addr1\n");
//     sscanf(ip_str, "%d.%d.%d.%d", &segs[0], &segs[1], &segs[2], &segs[3]);
//     printf("custom_inet_addr2\n");
//     for (int i = 0; i < 4; ++i) {
//         ip |= segs[i] << ((3 - i) * 8);
//     }
//     printf("ip:0x%x\n", ip);
//     return ip;
// }

// tcp crc
uint16_t tcp_checksum(struct iphdr_t *ip, struct tcphdr *tcp, char *payload, int payload_size) {
	struct pseudo_header psh;
	psh.source_address = *(u32*)ip->saddr;
	psh.dest_address = *(u32*)ip->daddr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + payload_size);

	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payload_size;
	char *pseudogram = malloc(psize);

	memcpy(pseudogram, (char *)&psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcp, sizeof(struct tcphdr));
	memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct tcphdr), payload, payload_size);

	uint16_t csum = checksum(pseudogram, psize);

	free(pseudogram);
	return csum;
}

void parse_mac_address(const char *mac_str, unsigned char *mac) {
	sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
		&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}

void set_eth_header(struct ethhdr_t *eth, const char *src_mac_str, const char *dest_mac_str) {
	parse_mac_address(src_mac_str, eth->h_source);
	parse_mac_address(dest_mac_str, eth->h_dest);
	eth->h_proto = htons(ETH_P_IP);
}

void build_packet(char *src_addr, char* dst_addr, size_t packet_len, size_t* head_len_p) {
	printf("into\n");
	// eth header
	struct ethhdr_t *eth = (struct ethhdr_t *)src_addr;
	printf("struct ethhdr_t:%d\n",sizeof(struct ethhdr_t));
	set_eth_header(eth, "8e:d1:b3:03:50:24", "04:32:01:2c:ec:31");
	printf("into2\n");
	// IP header
	struct iphdr_t *ip = (struct iphdr_t *)(src_addr + sizeof(struct ethhdr_t));

	//printf("src_addr:0x%lx ip_addr:0x%lx ip->saddr:0x%lx",src_addr, ip,ip->saddr);
	printf("into23\n");
	ip->ihl = 5;
	printf("into24\n");
	ip->version = 4;
	ip->tos = 0;
	ip->id = 0;
	ip->frag_off = 0;
	ip->ttl = 255;
	ip->protocol = IPPROTO_TCP;
	printf("into77\n");
	set_iphdr_addresses(ip, "192.168.1.2", "10.0.0.1");

    	printf("Source IP Address: %d.%d.%d.%d\n", 
        	ip->saddr[0], ip->saddr[1], ip->saddr[2],ip->saddr[3]);
    	printf("Destination IP Address: %d.%d.%d.%d\n", 
           	ip->daddr[0], ip->daddr[1], ip->daddr[2], ip->daddr[3]);
	printf("into3\n");
	// TCP header
	struct tcphdr *tcp = (struct tcphdr *)(src_addr + sizeof(struct ethhdr_t) + sizeof(struct iphdr_t));
	tcp->source = htons(1234);
	tcp->dest = htons(80);
	tcp->seq = htonl(1);
	tcp->doff = 5;
	tcp->window = htons(5840);
	printf("into4\n");
	// payload
	size_t head_len = sizeof(struct ethhdr_t) + sizeof(struct iphdr_t) + sizeof(struct tcphdr);
	char *data = src_addr + head_len;
	char *dst_addr_payload = dst_addr +  head_len;
	size_t payload_size = packet_len - head_len;
	*head_len_p = head_len;
	write_frame((uintptr_t)data, 
		    (uintptr_t)dst_addr_payload, 
		    packet_len - head_len);
	printf("into5\n");


	// cal length and crc
	ip->tot_len = htons(sizeof(struct iphdr_t) + sizeof(struct tcphdr) + payload_size);
	ip->check = 0;
	ip->check = checksum(ip, sizeof(struct iphdr_t));

	tcp->check = 0;
	tcp->check = tcp_checksum(ip, tcp, data, payload_size);

	if (sizeof(struct ethhdr_t) + ntohs(ip->tot_len) > packet_len) {
		fprintf(stderr, "Header size exceeds packet length\n");
		exit(EXIT_FAILURE);
    	}
	// printf packet
	printf("Packet size: %zu bytes\n", sizeof(struct ethhdr_t) + ntohs(ip->tot_len));
	printf("Packet contents:\n");
	for (int i = 0; i < sizeof(struct ethhdr_t) + ntohs(ip->tot_len); i++) {
			printf("%02x ", (unsigned char)src_addr[i]);
		if ((i + 1) % 16 == 0) printf("\n");
	}
	printf("\n");
}

// int main() {
// 	int sock;
// 	struct sockaddr_ll target;
// 	char src[1500]; // 数据包缓冲
// 	char dst[1500]; 
// 	size_t tol_head_len;
// 	// 创建原始套接字
// 	sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
// 	if (sock == -1) {
// 		perror("Socket creation failed");
// 		exit(EXIT_FAILURE);
// 	}
// 	// 设置目标地址信息
// 	memset(&target, 0, sizeof(struct sockaddr_ll));
// 	target.sll_family = AF_PACKET;
// 	target.sll_protocol = htons(ETH_P_IP);
// 	target.sll_ifindex = if_nametoindex("ens33"); // 使用适当的网络接口名替换"eth0"

// 	// 构建数据包
// 	build_packet(src, dst, 1280, &tol_head_len);
// 	printf("tol_head_len:%lu\n",tol_head_len);

// 	// 计算整个数据包的大小
// 	struct iphdr_t *ip = (struct iphdr_t *)(src + sizeof(struct ethhdr_t));
// 	size_t packet_size = sizeof(struct ethhdr_t) + ntohs(ip->tot_len);

// 	// 使用sendto发送数据包
// 	if (sendto(sock, src, packet_size, 0, (struct sockaddr *)&target, sizeof(struct sockaddr_ll)) < 0) {
// 		perror("Send failed");
// 		close(sock);
// 		exit(EXIT_FAILURE);
// 	}

// 	printf("Packet sent successfully\n");

// 	// 关闭套接字
// 	close(sock);
// 	return 0;
// }

