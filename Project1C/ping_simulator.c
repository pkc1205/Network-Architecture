#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

unsigned short in_cksum(unsigned short *, int);

int main(int argc, char* argv[])
{
  struct iphdr* ip;
  struct iphdr* ip_response;
  clock_t t1,t2;
  float diff,mean_rtt=0;

  struct icmphdr* icmp;
  struct icmphdr* icmp_response;
  struct sockaddr_in connection;

  struct hostent* h;
  char buf[256];
  char dst_addr[20];
  char src_addr[20];

  char* packet; //buffer to send the echo request
  char* buffer; //buffer for echo packet response
  int sockfd;
  int optval = 1;
  int len;
  int siz;
  int i;
  int packetloss=0;

  
  /* get destination hostname */
  h = gethostbyname(argv[1]);
  strncpy(dst_addr,(char *)inet_ntoa(*(struct in_addr *)h->h_addr),20);

  /* source hostname */
  gethostname(buf,256);
  h = gethostbyname(buf);
  strncpy(src_addr,(char *)inet_ntoa(*(struct in_addr *)h->h_addr),20);

  packet = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));
  buffer = malloc(sizeof(struct iphdr) + sizeof(struct icmphdr));

  ip = (struct iphdr*) packet;
  icmp = (struct icmphdr*) (packet + sizeof(struct iphdr));

  /* Now prepare the ip header and the icmp header */
  ip->ihl = 5;
  ip->version = 4;
  ip->tos = 0;
  ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
  ip->id = htons(0);
  ip->frag_off = 0;
  ip->ttl = 60;
  ip->protocol = IPPROTO_ICMP;
  ip->saddr = inet_addr(src_addr);
  ip->daddr = inet_addr(dst_addr);
  ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

  icmp->type = ICMP_ECHO;
  icmp->code = 0;
  icmp->un.echo.id = 123;
  icmp->un.echo.sequence = htons(0);
  icmp->checksum = in_cksum((unsigned short *)icmp, sizeof(struct icmphdr));

  connection.sin_family = AF_INET;
  connection.sin_addr.s_addr = inet_addr(dst_addr);


  if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
  {
     printf("socket() failed");
     return 1;
  }

  /* Dont add the default IP */
  setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optval, sizeof(int));


  /*
  * now send the multiple packets 
  */

  for(i=0;i<10;i++)
  {
     sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&connection, sizeof(struct sockaddr));

     /* Look for response */
     len = sizeof(connection);
     if((siz = recvfrom(sockfd, buffer, sizeof(struct iphdr) + sizeof(struct icmphdr), 0, (struct sockaddr *)&connection, &len)) < 0)
     {
       printf("response error\n");
       packetloss++;
     }
     else
     {
       ip_response   = (struct iphdr*) buffer;
       icmp_response = (struct icmphdr*)(buffer+sizeof(struct iphdr));

       printf("icmp_response->type %d ,size %d\n",icmp_response->type,siz);
       printf("reply from %s:TTL=%d\n",dst_addr,ip->ttl);
     }
  }


  printf("packetloss = %d\n",packetloss);

  free(packet);
  free(buffer);
  close(sockfd);
  return 0;
}

/*
* Note : using existing checksum function
*/
unsigned short in_cksum(unsigned short *addr, int len)
{
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;

   while (nleft > 1)
   {
      sum += *w++;
      nleft -= 2;
   }

   if (nleft == 1)
   {
     *(u_char *) (&answer) = *(u_char *) w;
     sum += answer;
   }

   sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
   sum += (sum >> 16); /* add carry */
   answer = ~sum; /* truncate to 16 bits */
   return (answer);
}
