//
// Created by GourdBoy on 2017/8/11.
//
#include "com_haier_networkdetect_networkdetect_NativeNetworkUtils.h"
#define LOG "NETWORKDETECT"
#ifdef DEBUG
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG,__VA_ARGS__)
#else
#define LOGI(...)
#define LOGE(...)
#endif
#define MAX_WAIT_TIME   3
#define MAX_NO_PACKETS  1
#define ICMP_HEADSIZE 8
#define PACKET_SIZE     4096
struct timeval tvsend,tvrecv;
struct timeval tvsend1,tvrecv1;
struct sockaddr_in dest_addr,recv_addr;
struct sockaddr_in dest_addr1,recv_addr1;
int sockfd;
int sockfd1;
int is_thread_started = 0;
pid_t pid;
static volatile int isThreadExit = 0 ;
static volatile int isIpCorrect = 0 ;
static volatile int isIpCorrect1 = 0 ;
volatile int thread_num = 0;
JavaVM * j_VM;
jobject jObj;
pthread_mutex_t mutex;
int a = 1;
int b = 2;
JNIEXPORT void JNICALL Java_com_haier_networkdetect_networkdetect_NativeNetworkUtils_networkDetectInit(JNIEnv *env, jobject jcls)
{
    (*env)->GetJavaVM(env,&j_VM);
    jObj = (*env)->NewGlobalRef(env,jcls);
    pthread_mutex_init(&mutex,NULL);
}
JNIEXPORT void JNICALL Java_com_haier_networkdetect_networkdetect_NativeNetworkUtils_networkDetectSetIP1(JNIEnv *env, jobject jcls,jstring j_str)
{
    const char *c_str = (*env)->GetStringUTFChars(env, j_str, NULL);
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    if(c_str != NULL&&inet_pton(AF_INET, c_str, &(dest_addr.sin_addr)) > 0)
    {
        isIpCorrect=1;
    }
    else
    {
        isIpCorrect = 0;
        LOGE("inet_pton errno %d %s\n", errno, strerror(errno));
    }
    LOGI("got ip1 is %s",c_str);
    (*env)->ReleaseStringUTFChars(env, j_str,c_str);
}
JNIEXPORT void JNICALL Java_com_haier_networkdetect_networkdetect_NativeNetworkUtils_networkDetectSetIP2(JNIEnv *env, jobject jcls,jstring j_str)
{
    const char *c_str = (*env)->GetStringUTFChars(env, j_str, NULL);
    bzero(&dest_addr1, sizeof(dest_addr1));
    dest_addr1.sin_family = AF_INET;
    if(c_str != NULL&&inet_pton(AF_INET, c_str, &(dest_addr1.sin_addr)) > 0)
    {
        isIpCorrect1=1;
    }
    else
    {
        isIpCorrect1 = 0;
        LOGE("inet_pton 2 errno %d %s\n", errno, strerror(errno));
    }
    LOGI("got ip2 is %s",c_str);
    (*env)->ReleaseStringUTFChars(env, j_str,c_str);
}
JNIEXPORT void JNICALL Java_com_haier_networkdetect_networkdetect_NativeNetworkUtils_networkDetectThreadStart(JNIEnv *env, jobject jcls)
{
    if(is_thread_started)
        return;
    isThreadExit = 0;
    pthread_t threadId;
    pthread_t threadId1;
    if(pthread_create(&threadId,NULL,thread_network_exec,(void*)&a)!=0)
    {
        LOGE("native thread1 start failed!");
    }
    else
    {
        thread_num++;
    }
    if(pthread_create(&threadId1,NULL,thread_network_exec,(void*)&b)!=0)
    {
        LOGE("native thread2 start failed!");
    }
    else
    {
        thread_num++;
    }
    is_thread_started = 1;
}
JNIEXPORT void JNICALL Java_com_haier_networkdetect_networkdetect_NativeNetworkUtils_networkDetectThreadStop(JNIEnv *env, jobject jcls)
{
    isThreadExit = 1;
}
void* thread_network_exec(void *p)
{
    int id = *((int *)p);
     LOGE("network thread %d started",id);
     if(id!=1&&id!=2)
     {
        LOGE("Invalid thread id!");
        return NULL;
     }
     JNIEnv *env;
     if ((*j_VM)->AttachCurrentThread(j_VM, &env, NULL) != 0)
     {
        LOGE("AttachCurrentThread failed!");
        return NULL;
     }
     jclass javaClass = (*env)->GetObjectClass(env, jObj);
     if (javaClass == 0)
     {
             LOGE("Unable to find class");
             (*j_VM)->DetachCurrentThread(j_VM);
             return NULL;
     }
     jmethodID javaCallbackId;
     if(id==1)
        javaCallbackId = (*env)->GetMethodID(env, javaClass,"networkDetectCallback1", "(Z)V");
     else if(id==2)
        javaCallbackId = (*env)->GetMethodID(env, javaClass,"networkDetectCallback2", "(Z)V");
     if (javaCallbackId == NULL)
     {
             LOGE("Unable to find method:networkDetectCallback");
             (*j_VM)->DetachCurrentThread(j_VM);
             return NULL;
     }
     int i;
     while(!isThreadExit)
     {
        sleep(5);
        int ipCorrect;
        int sock;
        if(id==1)
        {
            ipCorrect = isIpCorrect;
        }
        else if(id==2)
        {
            ipCorrect = isIpCorrect1;
        }
        if(ipCorrect)
        {
            if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP))==-1)
            {
                LOGE("thread%d create SOCK_DGRAM failed ,sockfd is %d,errno is %d  %s",id,sock,errno,strerror(errno));
                continue;
            }
            char sendpacket[PACKET_SIZE];
            char recvpacket[PACKET_SIZE];
            if(id==1)
            {
                sockfd = sock;
            }
            else if(id==2)
            {
                sockfd1 = sock;
            }
            pid=getpid();
            LOGI("pid %d",pid);
            for(i=0;i<MAX_NO_PACKETS;i++)
            {
                if(send_packet(i,sendpacket,id)<0)
                {
                    LOGE("send packet failed!");
                    (*env)->CallVoidMethod(env, jObj, javaCallbackId,JNI_FALSE);
                    break;
                }
                if(recv_packet(i,recvpacket,id)>0)
                {
                    struct timeval endtime;
                    struct timeval starttime;
                    if(id==1)
                    {
                        endtime = tvrecv;
                        starttime = tvsend;
                    }
                    else
                    {
                        endtime = tvrecv1;
                        starttime = tvsend1;
                    }
                    LOGI("network%d is ok! time=%.2fms",id,(float)((1000000*(endtime.tv_sec-starttime.tv_sec)+(endtime.tv_usec-starttime.tv_usec))/1000));
                    (*env)->CallVoidMethod(env, jObj, javaCallbackId,JNI_TRUE);
                    break;
                }
                else
                {
                    (*env)->CallVoidMethod(env, jObj, javaCallbackId,JNI_FALSE);
                }
            }
            _CloseSocket(id);
            LOGI("close socket");
        }
        else
        {
            (*env)->CallVoidMethod(env, jObj, javaCallbackId,JNI_FALSE,(*env)->NewStringUTF(env,"-"));
        }
     }
      LOGI("network thread id%d loop exit",id);
      pthread_mutex_lock(&mutex);
      thread_num--;
      LOGI("network thread id%d num %d",id,thread_num);
      if(!thread_num)
      {
        LOGI("DeleteGlobalRef");
        (*env)->DeleteGlobalRef(env,jObj);
      }
     env = NULL;
     pthread_mutex_unlock( &mutex );
     if(!thread_num)
     {
         LOGI("pthread_mutex_destroy");
         pthread_mutex_destroy( &mutex );
     }
     (*j_VM)->DetachCurrentThread(j_VM);
     LOGI("DetachCurrentThread %d",id);
     pthread_exit(0);
}
int send_packet(int pkt_no,char *sendpacket,int id)
{
    int packetsize;
    struct sockaddr_in d_addr;
    packetsize=pack(pkt_no,sendpacket);
    if(id==1)
    {
        gettimeofday(&tvsend,NULL);
        d_addr = dest_addr;
    }
    else if(id==2)
    {
        gettimeofday(&tvsend1,NULL);
        d_addr = dest_addr1;
    }
    int code = sendto(sockfd,sendpacket,packetsize,0,(struct sockaddr *)&d_addr,sizeof(d_addr));
    if(code<0)
    {
        LOGE("sendto error %d, errno %d  %s",code,errno,strerror(errno));
        return -1;
    }
    return 1;
}
int recv_packet(int pkt_no,char *recvpacket,int id)
{
    int rc;
    socklen_t fromlen;
    struct sockaddr_in r_addr;
    fd_set rfds;
    int sock;
    if(id==1)
    {
            r_addr = recv_addr;
            sock = sockfd;
    }
    else
    {
            r_addr = recv_addr1;
            sock = sockfd1;
    }
    FD_ZERO(&rfds);
    FD_SET(sock,&rfds);
    fromlen=sizeof(r_addr);
    struct timeval timeout = {MAX_WAIT_TIME, 0};
    rc=select(sock+1, &rfds, NULL, NULL, &timeout);
    if (rc == 0)
    {
        LOGE("no reply in 3 second\n");
        return -1;
    }
    else if (rc < 0)
    {
        LOGE("select errno %d %s\n", errno, strerror(errno));
        return -1;
    }
    rc=recvfrom(sockfd,recvpacket,PACKET_SIZE,0,(struct sockaddr *)&r_addr,&fromlen);
    if( rc <=0)
    {
        LOGE("revcfrom errno %d %s\n", errno, strerror(errno));
        return -1;
    }
    if(unpack(pkt_no,recvpacket,rc)==-1)
        return -1;
    if(id==1)
            gettimeofday(&tvrecv,NULL);
    else
            gettimeofday(&tvrecv1,NULL);
    return 1;
}
int pack(int pkt_no,char*sendpacket)
{
    int i,packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp=(struct icmp*)sendpacket;
    icmp->icmp_type=ICMP_ECHO;   //设置类型为ICMP请求报文
    icmp->icmp_code=0;
    icmp->icmp_cksum=0;
    icmp->icmp_seq=pkt_no;
    icmp->icmp_id=pid;           //设置当前进程ID为ICMP标示符
    packsize=ICMP_HEADSIZE+sizeof(struct timeval);
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);
    icmp->icmp_cksum=cal_chksum( (unsigned short *)icmp,packsize);
    return packsize;
}
int unpack(int cur_seq,char *buf,int len)
{
    //LOGI("unpack packet");
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    ip=(struct ip *)buf;
    iphdrlen=ip->ip_hl<<2;
    icmp=(struct icmp *)(buf+iphdrlen);
    len-=iphdrlen;
    if( len<8)
    {
        LOGE("receive packet lenth < 8");
        return -1;
    }
    if( (icmp->icmp_type==ICMP_ECHOREPLY) && (icmp->icmp_seq==cur_seq))
    {
         //LOGI("unpack packet success");
        return 0;
    }
    LOGE("unpack packet failed,current sequence %d,icmp_seq %d",cur_seq,icmp->icmp_seq);
    return -1;
}
unsigned short cal_chksum(unsigned short *addr,int len)
{
    int nleft=len;
    int sum=0;
    unsigned short *w=addr;
    unsigned short answer=0;
    while(nleft>1)
    {
        sum+=*w++;
        nleft-=2;
    }
    if( nleft==1)
    {
        *(unsigned char *)(&answer)=*(unsigned char *)w;
        sum+=answer;
    }
    sum=(sum>>16)+(sum&0xffff);
    sum+=(sum>>16);
    answer=~sum;
    return answer;
}
void _CloseSocket(int id)
{
    if(id==0)
    {
        close(sockfd);
        sockfd = 0;
    }
    else
    {
        close(sockfd1);
            sockfd1 = 0;
    }
}