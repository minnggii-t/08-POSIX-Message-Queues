#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>

#define Q_NAME "/pos_msgQ"
#define Q_PERM 0660
#define MAX_MSG 3
#define MAX_MSG_SIZE 256
#define BUF_SIZE 60

static void sig_end()
{
    exit(EXIT_SUCCESS);
}

struct my_msg 
{
   int msg_type;
   char data[MAX_MSG_SIZE];
};

int main(int argc, char *argv[])
{
    // key_t key;
    int msgID, child, nbytes;
    struct my_msg a_msg;
    struct mq_attr q_attr;  // must provide this attr struct
    mqd_t mqd;  // use for posix msgQ ID data type
    unsigned int priority = 0;  // use this instead of long msg_type
    char buffer[BUF_SIZE], *s;
    
    q_attr.mq_maxmsg = MAX_MSG;
    q_attr.mq_msgsize = MAX_MSG_SIZE;

    mqd = mq_open(Q_NAME, O_WRONLY | O_CREAT, 0660, &q_attr);
    if (mqd == -1) {
        perror("mq_open failed ");
        exit(EXIT_FAILURE);
    }

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
        exit(EXIT_FAILURE);
    }

    // key = ftok("./qchat", 202203);
    // msgID = msgget(key, 0666 | IPC_CREAT); //110110110 | 1010000000000 -> 1010110110110
    // if (msgID == -1)
    // {
    //     perror("msgget() failed\n"); exit(EXIT_FAILURE);
    // }

    signal(SIGUSR1, sig_end);
    argv++;
    if (strcmp(*argv, "1") == 0)
    {
        child = fork();
        switch (child)
        {
            case -1: perror("Forking failed\n"); exit(EXIT_FAILURE);
            case  0: while (strncmp(a_msg.data, "end chat", 8))
                     {
                        //  if ((nbytes = msgrcv(msgID, (void *) &a_msg, BUFSIZ, 2, 0)) > 0)
                        //      write(1, a_msg.data, nbytes);
                        if (mq_receive(mqd, (void *)&a_msg, MAX_MSG_SIZE, &priority) == -1)
                        {  //priority for receive need to be the reference
                            perror("mq_receive failed ");
                            exit(EXIT_FAILURE);
                        }
                     }
                    //  if (msgctl(msgID, IPC_RMID, 0) == -1)
                    //  {
                    //      perror("msgctl failed: "); exit(EXIT_FAILURE);
                    //  }
                    mq_close(mqd);  // close the posix msgQ
                    if (mq_unlink(Q_NAME) == -1)
                    { //unlink posix msgQ
                        perror("mq_unlink failed ");
                        exit(EXIT_FAILURE);
                    }
                     break;
            default: 
            priority = 2;
            while (strncmp(buffer, "end chat", 8))
            {
                if ((nbytes = read(0, buffer, BUFSIZ)) > 0)
                {
                    a_msg.msg_type = 1; strcpy(a_msg.data, buffer);
                //  msgsnd(msgID, (void *) &a_msg, nbytes,  0);
                    if (mq_send(mqd, (void *)&a_msg, sizeof(struct my_msg), priority) == -1)
                    {  // send must contain sizeof struct and priority value
                        perror("mq_send failed ");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    else if (strcmp(*argv, "2") == 0)
    {
        child = fork();
        switch (child)
        {
            case -1: perror("Forking failed\n"); exit(EXIT_FAILURE);
            case  0: while (strncmp(a_msg.data, "end chat", 8))
                     {
                        //  if ((nbytes = msgrcv(msgID, (void *) &a_msg, BUFSIZ, 1, 0)) > 0)
                        //      write(1, a_msg.data, nbytes);
                        if (mq_receive(mqd, (void *)&a_msg, MAX_MSG_SIZE, &priority) == -1)
                        {  //priority for receive need to be the reference
                            perror("mq_receive failed ");
                            exit(EXIT_FAILURE);
                        }
                     }
                    //  if (msgctl(msgID, IPC_RMID, 0) == -1)
                    //  {
                    //      perror("msgctl failed: "); exit(EXIT_FAILURE);
                    //  }

                    mq_close(mqd);  // close the posix msgQ
                    if (mq_unlink(Q_NAME) == -1)
                    { //unlink posix msgQ
                        perror("mq_unlink failed ");
                        exit(EXIT_FAILURE);
                    }
                     break;
            default: 
            priority = 1;
            while (strncmp(buffer, "end chat", 8))
            {
                if ((nbytes = read(0, buffer, BUFSIZ)) > 0)
                {
                    a_msg.msg_type = 2; strcpy(a_msg.data, buffer);
                //  msgsnd(msgID, (void *) &a_msg, nbytes,  0);
                    if (mq_send(mqd, (void *)&a_msg, sizeof(struct my_msg), priority) == -1)
                    {  // send must contain sizeof struct and priority value
                        perror("mq_send failed ");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
        
    if (child) kill(child, SIGUSR1);
    else kill(getppid(), SIGUSR1);
   
    exit(EXIT_SUCCESS);
}