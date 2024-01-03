#ifndef __RAW_TCP_IP_H__
#define __RAW_TCP_IP_H__





void build_packet(char *src_addr, char* dst_addr, size_t packet_len, size_t* head_len_p);
void parse_mac_address(const char *mac_str, unsigned char *mac);






#endif