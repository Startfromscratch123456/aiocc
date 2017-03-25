#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
/* 定义状态码 */
#define OK 0
#define ERROR 1

#define TRUE 1
#define FALSE 0
#define LIMIT 65535
#define OPERATOR_LEN 256
#define PTHREAD_MAX 10

#define gettidv1() syscall(__NR_gettid)

//最外层的搜索范围,对于一个节点上的所有线程来说都是一样的;不同的计算节点计算范围由主控节点分配
static int LOW = 0;
static int HIGH = 0;

pthread_spinlock_t	global_start_index_a_lock;
//计算范围起始下标
static int global_start_index_a = 1;

pthread_spinlock_t	cur_cpu_core_index_lock;
//cpu核心编号,用于尽量保证每个线程都在不同的核上运行
static int cur_cpu_core_index = 0;//

//求的解的数量,其值为各线程求得的解之和
pthread_mutex_t solution_num_mutex = PTHREAD_MUTEX_INITIALIZER;
static int solution_num = 0;//
//每个进程分配的计算范围步长,初始值为1024,实际上会根据实际情况计算
int STEP0 = 1024;
//CPU的核数,初始阶段会调用相关函数获得准确的值
int CPU_CORE_NUM = 1;

typedef struct
{
    pthread_t	pid;
} PTHREAD_SIG;
PTHREAD_SIG g_pthread_sig[PTHREAD_MAX];
/*
a,b,c均为大于0的自然数：
a/(b+c) + b/(a+c) + c/(a+b) = 4
1.编一个程序证明在a<65536,b<65536,c<65536情况下无解
2.编一个程序证明以下值是正确的：
a=4373612677928697257861252602371390152816537558161613618621437993378423467772036
b=36875131794129999827197811565225474825492979968971970996283137471637224634055579
c=154476802108746166441951315019919837485664325669565431700026634898253202035277999

 */

/**
 * *实现非负大整数相加,用字符串存放大整数,add不做参数合法性检测
 * *num1/num2为要相加的参数
 * *sum 存放相加结果
 */
void  big_data_add( const char* num1, const char* num2, char* sum )
{
    int num1_len = strlen( num1 );
    int num2_len = strlen( num2 );
    int rs_len = 2 + (num1_len > num2_len ? num1_len : num2_len);
    /* char* sum = 0; */
    int i  = 0, j = 0, k = 0;
    int first_not_zero = 0;
    int tmp  = 0;
    int carry  = 0;
    /* 处理参数含0的情况 */
    while ( i < num1_len && num1[i] == '0' )
    {
        i++;
    }
    while ( j < num2_len && num2[j] == '0' )
    {
        j++;
    }
    if ( i == num1_len || j == num2_len )
    {
        if ( i == num1_len && j == num2_len )
        {
            strcpy( sum, "0" );
        }
        else if ( j == num2_len )
        {
            strcpy( sum, num1 );
        }
        else
        {
            strcpy( sum, num2 );
        }
    }

    memset( sum, '0', sizeof(char) * rs_len );
    sum[rs_len - 1] = 0;
    i  = num1_len - 1;
    j  = num2_len - 1;
    k  = rs_len - 2;
    while ( i >= 0 && j >= 0 )
    {
        tmp = (num1[i] - '0') + (num2[j] - '0') + carry;
        sum[k] = tmp % 10 + '0';
        carry = tmp / 10;
        i--;
        j--;
        k--;
    }
    while ( i >= 0 )
    {
        tmp = (num1[i] - '0') + carry;
        sum[k] = tmp % 10 + '0';
        carry = tmp / 10;
        i--;
        k--;
    }
    while ( j >= 0 )
    {
        tmp = (num2[j] - '0') + carry;
        sum[k] = tmp % 10 + '0';
        carry = tmp / 10;
        j--;
        k--;
    }
    //处理最后进位值不为0的情况
    while (carry)
    {
        sum[k] = carry % 10+ '0';
        carry /= 10;
    }
    while ( sum[first_not_zero] == '0' )
    {
        first_not_zero++;     /* 找到第一个非0 */
    }

    if ( first_not_zero )
    {
        i = 0;
        while ( sum[first_not_zero] )  /* 向前移动位置 */
        {
            sum[i] = sum[first_not_zero];
            i++;
            first_not_zero++;
        }
        sum[i] = 0;
    }
}


/**
 * *实现非负大整数相乘,用字符串存放大整数.multiply不做参数合法性检测
 * *num1/num2为要相乘的参数
 * *sum存放相乘结果
 */
void big_data_multiply( const char* num1, const char* num2, char* sum )
{
    int num1_len = strlen( num1 );
    int num2_len = strlen( num2 );
    int rs_len = num1_len + num2_len + 1;//m*n最多有m+n位,增加一位存'\0'
    /* char* sum = NULL; */
    int i  = 0, j = 0;
    int first_not_zero = 0;
    int tmp  = 0;
    int carry  = 0;
    /* 处理参数含0的情况 */
    while ( i < num1_len && num1[i] == '0' )
    {
        i++;
    }
    if ( i == num1_len ) /* i==num1_len */
    {
        strcpy( sum, "0" );
    }
    while ( j < num2_len && num2[j] == '0' )
    {
        j++;
    }
    if ( j == num2_len )
    {
        strcpy( sum, "0" );
    }
    memset( sum, '0', sizeof(char) * rs_len );
    sum[num1_len + num2_len] = 0;
    for ( i = num1_len - 1; i >= 0; i-- )
    {
        carry = 0;
        for ( j = num2_len - 1; j >= 0; j-- )
        {
            tmp  = (sum[i + j + 1] - '0') + (num1[i] - '0') * (num2[j] - '0') + carry;
            sum[i + j + 1] = tmp % 10 + '0';
            carry  = tmp / 10;
        }
        sum[i] += (carry);
    }

    while ( sum[first_not_zero] == '0' )
    {
        first_not_zero++;     /* 找到第一个非0 */
    }

    if ( first_not_zero )
    {
        i = 0;
        while ( sum[first_not_zero] )  /* 向前移动位置 */
        {
            sum[i++] = sum[first_not_zero++];
        }
        sum[i] = 0;
    }
}
/**
*对于给定的a b c,验证是否有a*a*a + b*b*b + c*c*c == 5 * a * b * c + 3 * (aa * (b + c) + bb * (a + c) +  cc * (a + b))
*返回值为trcmp比较的值
*/
int check_ans_abc0(const char* a, const char* b, const char* c, char(*rs) [256])
{
    char aa[OPERATOR_LEN], bb[OPERATOR_LEN], cc[OPERATOR_LEN];
    char aaa[OPERATOR_LEN], bbb[OPERATOR_LEN], ccc[OPERATOR_LEN];
    char sumabcCube[OPERATOR_LEN], abc5[OPERATOR_LEN];
    char tmp[6][OPERATOR_LEN];
    big_data_multiply(a, a, aa);
    big_data_multiply(b, b, bb);
    big_data_multiply(c, c, cc);
    big_data_multiply(aa, a, aaa);
    big_data_multiply(bb, b, bbb);
    big_data_multiply(cc, c, ccc);
    big_data_add(aaa, bbb, tmp[0]);
    big_data_add(ccc, tmp[0], sumabcCube);
    big_data_multiply(a, b, tmp[0]);
    big_data_multiply(c,  tmp[0], tmp[1]);
    big_data_multiply("5",  tmp[1], abc5);

    big_data_add(b, c, tmp[0]);
    big_data_add(a, c, tmp[1]);
    big_data_add(a, b, tmp[2]);
    big_data_multiply(aa, tmp[0], tmp[3]);
    big_data_multiply(bb, tmp[1], tmp[4]);
    big_data_multiply(cc, tmp[2], tmp[5]);
    big_data_add(tmp[3], tmp[4], tmp[0]);
    big_data_add(tmp[0], tmp[5], tmp[1]);
    big_data_multiply("3", tmp[1], tmp[2]);
    big_data_add(abc5, tmp[2], tmp[0]);
    if (rs)
    {
        strcpy(rs[0], sumabcCube);//a*a*a + b*b*b + c*c*c
        strcpy(rs[1], tmp[0]);//5 * a * b * c + 3 * (aa * (b + c) + bb * (a + c) +  cc * (a + b))
    }

    return strcmp(sumabcCube, tmp[0]);
}
/**
证明以下值是正确的：
a=4373612677928697257861252602371390152816537558161613618621437993378423467772036
b=36875131794129999827197811565225474825492979968971970996283137471637224634055579
c=154476802108746166441951315019919837485664325669565431700026634898253202035277999
*
*/
void check_ans_abc()
{
    char a[OPERATOR_LEN], b[OPERATOR_LEN], c[OPERATOR_LEN], rs[2][OPERATOR_LEN];
    strcpy(a,"4373612677928697257861252602371390152816537558161613618621437993378423467772036");
    strcpy(b,"36875131794129999827197811565225474825492979968971970996283137471637224634055579");
    strcpy(c,"154476802108746166441951315019919837485664325669565431700026634898253202035277999");
    if (check_ans_abc0(a, b, c, rs))
    {
        printf("a*a*a + b*b*b + c*c*c != 5 * a * b * c + 3 * (aa * (b + c) + bb * (a + c) +  cc * (a + b))\n");
    }
    else
    {
        printf("a*a*a + b*b*b + c*c*c == 5 * a * b * c + 3 * (aa * (b + c) + bb * (a + c) +  cc * (a + b))\n");
    }
    printf("LOG: %s\n", rs[0]);
    printf("LOG: %s\n", rs[1]);
}

/**
证明在a<65536,b<65536,c<65536情况下无解
*/
void *_check_ans_abc_lt65536()
{
    char a[OPERATOR_LEN], b[OPERATOR_LEN], c[OPERATOR_LEN];
    int i = 0, j = 0, k = 0;
    int count = 0;
    int local_val = 0;
    int local_cpu_core;
    int local_count = 0;
    cpu_set_t mask;
    cpu_set_t get;
    pid_t pid;
    pthread_t tid;

    pthread_spin_lock(&global_start_index_a_lock);
    //global_start_index_a的意义是global_start_index_a 前的范围都有线程在计算了
    local_val = global_start_index_a;
    //更新global_start_index_a的位置
    global_start_index_a += STEP0;
    if (global_start_index_a > LIMIT)
    {
        global_start_index_a = global_start_index_a;
    }
    pthread_spin_unlock(&global_start_index_a_lock);
    //绑定一个cpu core
    pthread_spin_lock(&cur_cpu_core_index_lock);
    local_cpu_core = cur_cpu_core_index;
    cur_cpu_core_index ++;
    if (cur_cpu_core_index > CPU_CORE_NUM - 1)
    {
        cur_cpu_core_index = 0;
    }
    pthread_spin_unlock(&cur_cpu_core_index_lock);
    CPU_ZERO(&mask);
    CPU_SET(local_cpu_core, &mask);
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
    {
        fprintf(stderr, "set thread affinity failed\n");
    }
    CPU_ZERO(&get);
    if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
    {
        fprintf(stderr, "get thread affinity failed\n");
    }

    if (CPU_ISSET(local_cpu_core, &get))
    {

        pid = getpid();
        tid = pthread_self();
        printf("[LOG] pid:%u, u_tid:%u (0x%x), k_tid:%u, processor:%d, loop:[%d, %d)-[%d, %d)\n",\
               (unsigned int)pid, (unsigned int)tid, (unsigned int)tid, (unsigned int)gettidv1(),\
               local_cpu_core, LOW, HIGH, local_val, local_val + STEP0);
    }

    for (i = LOW; i < HIGH; i++)
    {
        for (j = local_val; j < local_val + STEP0; j++)
        {
            for (k =(i/4 -j >j/4 -i?i/4 -j:j/4 -i); k <= 4*(i + j); k++)
            {
                sprintf(a, "%d", i);
                sprintf(b, "%d", j);
                sprintf(c, "%d", k);
                local_count += check_ans_abc0(a, b, c, NULL);//
            }
        }
    }
    //计算完毕，更新解的数量
    pthread_mutex_lock(&solution_num_mutex);
    solution_num += local_count;
    pthread_mutex_unlock(&solution_num_mutex);
}

int check_ans_abc_lt65536(int low,int high)
{
    pthread_t ptid;
    int i = 0;
    int result = 0;
    void * status;
    LOW = low;
    HIGH = high;
    global_start_index_a = LOW + 1;//这里还属于初始化阶段,不用加锁
    pthread_spin_init(&global_start_index_a_lock, PTHREAD_PROCESS_PRIVATE);
    pthread_spin_init(&cur_cpu_core_index_lock, PTHREAD_PROCESS_PRIVATE);
    pthread_mutex_init(&solution_num_mutex,NULL);
    CPU_CORE_NUM = sysconf(_SC_NPROCESSORS_CONF);
    memset(g_pthread_sig, 0, sizeof(g_pthread_sig));
    struct sigaction sigact;
    STEP0 = (high - low) / PTHREAD_MAX;
    //创建PTHREAD_MAX个线程进行计算
    for(i = 0; i < PTHREAD_MAX; i++)
    {
        result = pthread_create(&ptid, NULL, _check_ans_abc_lt65536, NULL);
        if(result !=0 )
        {
            printf("pthread_create error:%d\n", result);
            return ERROR;
        }
        else
        {
            g_pthread_sig[i].pid = ptid;
        }
    }
    for(i = 0; i < PTHREAD_MAX; i++)
    {
        pthread_join(g_pthread_sig[i].pid, &status);
    }
    return OK;
}
//将执行结果写回到文件:结果是本节点发现的解的个数
int write_result(char* name_prefix, int result)
{
    int count = 0;
    char file_name[16];
    sprintf(file_name,"result/%s_%d_%d", name_prefix, LOW, HIGH);
    FILE *f = fopen(file_name,"wb+");
    if (f && result != -1)
    {
        fprintf(f,"%d",result);
    }
    else
    {
        return ERROR;
    }
    fclose(f);
    return OK;
}
int main(int argc, char ** argv)
{
    int low, high;
    //Q1/Q2 解决问题1/2
    const char OPTION[2][3] = {"Q1", "Q2"};
    if (argc < 2 || !argv[1])
    {
        printf("No parameter!\n");
        return ERROR;
    }
    write_result(argv[1], -1);//建立文件

    if ( !strcmp(argv[1], OPTION[0]))
    {
        //参数检测
        if (!(argc < 4 || (low = atoi(argv[2])) == 0 || (high = atoi(argv[3])) ==0 || low > LIMIT || low >= high))
        {
            high = high > LIMIT ?LIMIT:high;
            check_ans_abc_lt65536(low, high);
        }
    }
    else  if ( !strcmp(argv[1], OPTION[1]))
    {
        check_ans_abc();

    }
    else
    {
        printf("Illegal parameter!\n");
    }
    //回写结果
    write_result(argv[1], solution_num);
    return OK;
}

