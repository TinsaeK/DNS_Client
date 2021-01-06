#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h> 
#include<netinet/in.h>
/***************
Tinsae Kebede  *
G01163908      *
****************/

void dnsQuery(unsigned char*);
void octet(unsigned char*, unsigned char*);
unsigned char* reverseOctet(unsigned char*,unsigned char*);

struct HEADER
{
  unsigned short id;
  unsigned char rd :1;
  unsigned char tc :1;
  unsigned char aa :1;
  unsigned char opCode :4;
  unsigned char qr :1;
  unsigned char rCode :4;
  unsigned char z :1;
  unsigned char ra :1;
  unsigned short qDCount;//entries in question section
  unsigned short aNCount;//resource records in answers
  unsigned short nSCount;//name server resource records in authority records
  unsigned short aRCount;//resource records in additional records section

  
};

struct QUESTION
{
  unsigned short qType;
  unsigned short qClass;
};

struct RRF
{
  unsigned short type;
  unsigned short claSS;
  unsigned int ttl;
  unsigned short rDLength;
 

};
int jump;
int main( int argc, char *argv[])
{

  dnsQuery(argv[1]);
  
 
  return 0;
  
  
  }
  
void octet(unsigned char *hostName,unsigned char* query){//converts a standard string to the octet form expected by the dns query message
int i;
    int temp=0;
    
    strcat((char*)hostName,".");
    for(i=0;i<strlen((char*)hostName);i++)
    {
     
      if(hostName[i]=='.')
      {
      
        
        *query++ =i-temp;
        for(temp=temp;temp<i;temp++)
        {
            *query++=hostName[temp];
        }
        temp++;
      }
    
    }
    *query++='\0';
}
  
void dnsQuery(unsigned char *hostName)
{
    char server[] = "8.8.8.8";
    
    struct timeval timeOut;
    timeOut.tv_sec=5;
    timeOut.tv_usec=0;
    
    printf("Preparing DNS query...\n");
    
    int sock= socket(AF_INET , SOCK_DGRAM, IPPROTO_UDP);
    
    
    srand(time(NULL));

    unsigned char cache[65536];
    struct sockaddr_in destinaTion;
    destinaTion.sin_family = AF_INET;
    destinaTion.sin_port = htons(53);
    destinaTion.sin_addr.s_addr = inet_addr(server);
    

    struct HEADER *header = (struct HEADER *)&cache;
    header->id = (unsigned short) htons(rand()% 65536);
    header->qr =0;
    header->opCode =0;
    header->aa =0;
    header->tc =0;
    header->rd =1;
    header->ra =0;
    header->z =0;
    header->rCode =0;
    header->qDCount= htons(1);
    header->aNCount= 0;
    header->nSCount= 0;
    header->aRCount= 0;

    unsigned char *query=(unsigned char*)&cache[sizeof(struct HEADER)];
    
    octet(hostName, query);
    
    
    printf("Contacting DNS server ...\n");
        struct QUESTION *question =(struct QUESTION*)&cache[sizeof(struct HEADER)+ (strlen((const char*)query)+1)];
    question->qType = htons(1);
    question->qClass = htons(1);
    printf("Sending DNS query ...\n");
    if(sendto(sock,(char*)cache,sizeof(struct HEADER) + (strlen((const char*)query)+1) + sizeof(struct QUESTION),0,(struct sockaddr*)&destinaTion,sizeof(destinaTion)) < 0)
    {
      perror("sending failed\n");
    }

    int a=0;
    int num=sizeof(destinaTion);
     printf("DNS response recieved (attempt 1 of 3)\n");
      if(recvfrom (sock,(char*)cache , 65536 , 0 , (struct sockaddr*)&destinaTion , (socklen_t*)&num ) < 0)
      {
        perror("receiving failed\n");
      } 
      
    printf("Processing DNS response ...\n");
    printf("________________________________________\n");
    header =(struct HEADER*) cache;
    query =(unsigned char*)&cache[sizeof(struct HEADER)];
    question =(struct QUESTION*)&cache[sizeof(struct HEADER)+(strlen((const char*)query)+1)];
    printf("header.ID = 0x%x\n", header->id);
    printf("header.QR = %u\n", header->qr);
    printf("header.OPCODE = %u\n", header->opCode);
    printf("header.AA = %u\n", header->aa); 
    printf("header.TC = %u\n", header->tc);   
    printf("header.RD = %u\n", header->rd);
    printf("header.RA = %u\n", header->ra);
    printf("header.Z = %d\n", ntohs(header->z));
    printf("header.RCODE = %u\n", header->rCode);
    printf("header.QDCOUNT = %d\n", ntohs(header->qDCount));
    printf("header.ANCOUNT = %d\n", ntohs(header->aNCount));
    printf("header.NSCOUNT = %d\n", ntohs(header->nSCount));
    printf("header.ARCOUNT = %d\n", ntohs(header->aRCount));
    printf("....\n");
    printf("....\n");
    printf("question.QNAME = 0x%x\n",query);
    printf("question.QTYPE = %d\n",ntohs(question->qType));
    printf("question.QCLASS= %d\n",ntohs(question->qClass));  
    printf("....\n");
    printf("....\n");
    unsigned char *dnsResponse=&cache[sizeof(struct HEADER)+(strlen((const char*)query)+1)+sizeof(struct QUESTION)];
    if(ntohs(header->aNCount)==0){
      printf("No Answers...\n");
    
    }
    else{
    int j, e;
    jump=0;
    for(j=0;j<ntohs(header->aNCount);j++){
      unsigned char *name=reverseOctet(dnsResponse,cache);
     printf("answer.NAME = %s\n", name);
     dnsResponse+=jump;
     struct RRF *rrf=(struct RRF*)(dnsResponse);
     printf("answer.TYPE = %d\n",ntohs(rrf->type));
     printf("answer.CLASS = %d\n",ntohs(rrf->claSS));
     printf("....\n");
     printf("....\n");
     dnsResponse+=sizeof(unsigned short)+sizeof(unsigned short)+sizeof(int)+sizeof(short);
     int soize=ntohs(rrf->rDLength);
     unsigned char *rData=(unsigned char*)malloc(soize);
     for(e=0; e<soize;e++){
       rData[e]=dnsResponse[e];
     }
     rData[soize]='\0';
     long *ipadd=(long*)rData;
     struct sockaddr_in ipfind;
     ipfind.sin_addr.s_addr=(*ipadd);
     printf("answer.RDATA = %s\n",inet_ntoa(ipfind.sin_addr));
     
     
    
    }
    
    }
    
    
         
}
unsigned char* reverseOctet(unsigned char* response,unsigned char* cache)//retrieves the octet form names and addresses from the dns response and translates them back to a normal string
{
    unsigned char *url;
     int buffer;
    
 
    url = (unsigned char*)malloc(256);
 
    jump=1;
    int check=0;
    int iter=0;
    while(*response!=0)
    {
        if(*response<192){
         url[iter++]=*response;
        }
        else
        {
            buffer = (*response)*256 + *(response+1) - 49152; 
            response = cache + buffer - 1;
            check=1;
        }
        
        response++;
        if(check==0) jump++;
        
 
       
    }
 
    url[iter]='\0'; 
    if(check==1) jump++;
    
 
    int i , j;
    int cap=strlen((char*)url);
    for(i=0;i<cap;i++) 
    {
        iter=url[i];
        for(j=0;j<iter;j++) 
        {
            url[i]=url[i+1];
            i=i+1;
        }
        url[i]='.';
    }
    url[i-1]='\0'; 
    return url;
}