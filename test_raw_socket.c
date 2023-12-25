#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <pcap.h>
#include <stdint.h>    // for uint64_t
#include <byteswap.h>  // for __bswap_64()

#define FRAME_SIZE 1500

// 从文件中读取数据帧
void read_packet_from_file() 
{
    FILE *file = fopen("captured_packets.bin", "rb+");
    if (file == NULL) {
        perror("fopen()");
        return;
    }

    unsigned char frame[FRAME_SIZE];
    size_t bytesRead;

    // 逐个读取数据帧
    while ((bytesRead = fread(frame, 1, FRAME_SIZE, file)) > 0) {
        // 这里简单地打印帧的内容
        printf("Read %zu bytes: ", bytesRead);
        for (size_t i = 0; i < bytesRead; ++i) {
            printf("%02x ", frame[i]);
        }
        printf("\n");
    }

    fclose(file);
}

// 存储数据帧到文件
void save_packet_to_file(const unsigned char *packet, int length) {
    FILE *file = fopen("captured_packets.bin", "wb+");
    if (file != NULL) {
        fwrite(packet, 1, length, file);
        fflush(file);  // 确保数据被立即写入文件
        fclose(file);
    }
}

// 篡改数据包的内容（假定只篡改了某个字节）
void tamperPacket(unsigned char *packet) {
    if (packet != NULL) {
        // 修改第一个字节为 0xFF
        packet[0] = 0xFF;
    }
}

// 转发数据包到网络接口
void forwardPacket(const unsigned char *packet, int length) {
    int sockfd;
    struct sockaddr_ll dest_addr;
	unsigned char dest_mac[6] = {0x00, 0x0c, 0x29, 0xac, 0x03, 0xb2};
	//00:0c:29:ac:03:b2
	
    unsigned char src_mac[6] = {0xe0, 0xa5, 0x09, 0x26, 0x14, 0x31};
    char *iface = "eth0";  // 当前要转发数据包的网络接口的名称
    unsigned char *frame; // 用于存储要发送的帧的缓冲区
	int frame_len = length;
	struct ether_header *eth_hdr;

    frame = calloc(frame_len, 1);
    memcpy(frame, packet, frame_len);

    // 尝试篡改数据包中的内容
    // tamperPacket(frame);

    // 创建套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0) {
		perror("socket()");
		exit(1);
	}

   	 // 设置目标地址信息
  	  memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sll_family = AF_PACKET; // 地址家族
	dest_addr.sll_protocol = htons(ETH_P_ALL); // 表示链路层协议
	dest_addr.sll_ifindex = if_nametoindex(iface); // 网络接口的索引，指定该套接字所关联的网络接口, 指示数据帧应该从哪个网络接口发送
	memcpy(dest_addr.sll_addr, dest_mac, 6); // 硬件地址，存储实际的硬件地址信息
	dest_addr.sll_halen = 6; // 硬件地址长度

    	// 无需创建新的数据帧但需要修改接受到的数据帧中的源和目的地址
   	memcpy(frame, dest_mac, 6);
	memcpy(frame + 6, src_mac, 6);

    	// 控制台打印相关帧信息
	eth_hdr = (struct ether_header *)frame;
    	printf("send from %s", ether_ntoa((struct ether_addr *)eth_hdr->ether_shost));
	printf(" to %s, type 0x%04x, length %d\n", ether_ntoa((struct ether_addr *)eth_hdr->ether_dhost), htons(eth_hdr->ether_type), frame_len);

   	 // 发送数据包到目标地址（可以选择从存储的数据帧文件中读取并发送）
    	int bytes_sent = sendto(sockfd, frame, frame_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	if (bytes_sent < 0) {
		perror("sendto()");
		exit(1);
	}
	printf("%d bytes sent\n", bytes_sent);

    	// 关闭套接字
    	close(sockfd);
}


static void dump_hex(char *desc, const void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	/* Output description if given. */
	if (desc != NULL)
		printf("%s:\n", desc);

	/* Process every byte in the data. */
	for (i = 0; i < len; i++) {
		/* Multiple of 16 means new line (with line offset). */
		if ((i % 16) == 0) {
			/* Just don't print ASCII for the zeroth line. */
			if (i != 0)
				printf("  %s\n", buff);

			/* Output the offset. */
			printf("  %08x:", i);
		}

		/* Now the hex code for the specific character. */
		printf(" %02x", pc[i]);

		/* And store a printable ASCII character for later. */
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	/* And print the final ASCII bit. */
	printf("  %s\n", buff);
}

void packet_handler(unsigned char *user_data, const struct pcap_pkthdr *pkthdr, const unsigned char *packet)
{	
	struct ether_header *eth_hdr = (struct ether_header *)packet;
    	unsigned char specific_mac[6] = {0xe0, 0xa5, 0x09, 0x26, 0x14, 0x31};
	unsigned char specific_type[2] = {0x81, 0x00};
//e0:a5:09:26:14:31
	if (memcmp(eth_hdr->ether_shost, specific_mac, 6) == 0 && 
	    memcmp(eth_hdr->ether_type, specific_type, 2)) {
       		printf("Captured packet from specific source address!\n");
		printf("captured %d bytes\n", pkthdr->len);
		dump_hex("packet", packet, pkthdr->len);
    	// save_packet_to_file(packet, pkthdr->len);
    	// forwardPacket(packet, pkthdr->len);
    	} else { // 过滤非目标源发来的数据帧
        	printf("Ignored packet from a different source address.\n");
    	}

}

int main()
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;

	// Open the capture interface
   	// "enp5s0"：想要捕获数据包的网络接口的名称
	// ETH_FRAME_LEN：捕获的数据包的最大长度
	// 1: 这表示混杂模式（promiscuous mode），它允许捕获所有经过网络接口的数据包，而不仅仅是发往当前计算机的数据包
	// 1000: 超时值， 表示捕获操作将在等待数据包到达的时间超过 1000 毫秒后返回。这里的单位是毫秒
	// errbuf：有错误发生，错误消息将被存储在这里
	handle = pcap_open_live("eth0", ETH_FRAME_LEN, 1, 1000, errbuf); 
	
	if (handle == NULL) {
		fprintf(stderr, "Error opening interface: %s\n", errbuf);
		return 1;
	}

	// Start capturing packets
	// pcap_loop: 这是 libpcap 中的循环捕获数据包的函数
	// handle: 之前通过 pcap_open_live 打开的网络捕获会话。
	// -1: 表示在捕获数据包时无限循环。你也可以指定一个数字表示捕获多少个数据包后停止。
	// packet_handler: 这是一个回调函数，对于每个捕获到的数据包都会调用这个函数进行处理。
	// NULL: 这是传递给回调函数的参数，在这里是没有额外参数，所以传入 NULL
    	printf("Start capturing packets.....\n");
	if (pcap_loop(handle, -1, packet_handler, NULL) == -1) {
		fprintf(stderr, "Error capturing packets: %s\n", pcap_geterr(handle));
		pcap_close(handle);
		return 1;
	}

	// Close the capture interface
    	printf("End capturing packets.....\n");
	pcap_close(handle);

	return 0;
}