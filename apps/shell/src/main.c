/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "show.h"
#include "shmsg.h"
#include "shcmd.h"
#include "semaphore.h"
#include "securec.h"
#include "unistd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

ShellCB *g_shellCB = NULL; //shell控制块

ShellCB *OsGetShellCb()
{
    return g_shellCB;
}

void ShellDeinit(ShellCB *shellCB)
{
	//释放shell相关的数据
    (void)pthread_mutex_destroy(&shellCB->historyMutex);
    (void)pthread_mutex_destroy(&shellCB->keyMutex);
    OsShellKeyDeInit((CmdKeyLink *)shellCB->cmdKeyLink);
    OsShellKeyDeInit((CmdKeyLink *)shellCB->cmdHistoryKeyLink);
    (void)free(shellCB);
}

//创建shell进程的2个线程
static int OsShellCreateTask(ShellCB *shellCB)
{
    struct sched_param param = { 0 };
    int ret;

	//首先调整进程的调度优先级
    ret = sched_getparam(getpid(), &param);
    if (ret != SH_OK) {
        goto OUT;
    }

    param.sched_priority = SHELL_PROCESS_PRIORITY_INIT;

    ret = sched_setparam(getpid(), &param);
    if (ret != SH_OK) {
        goto OUT;
    }

	//创建shellTask 线程
    ret = ShellTaskInit(shellCB);
    if (ret != SH_OK) {
        goto OUT;
    }

	//创建shellEntry 线程
    ret = ShellEntryInit(shellCB);
    if (ret != SH_OK) {
        goto OUT;
    }

	//等待shellTask线程退出
    (void)pthread_join(shellCB->shellTaskHandle, NULL);
	//等待shellEntry线程退出
    (void)pthread_join(shellCB->shellEntryHandle, NULL);

OUT:
    ShellDeinit(shellCB);
    return ret;
}


//   /bin/shell进程入口函数
int main()
{
    int ret = SH_NOK;
    ShellCB *shellCB = NULL;

	//申请shell控制块内存
    shellCB = (ShellCB *)malloc(sizeof(ShellCB));
    if (shellCB == NULL) {
        goto ERR_OUT1;
    }
	//初始化控制块各字段
    ret = memset_s(shellCB, sizeof(ShellCB), 0, sizeof(ShellCB));
    if (ret != SH_OK) {
        goto ERR_OUT1;
    }

    ret = pthread_mutex_init(&shellCB->keyMutex, NULL);
    if (ret != SH_OK) {
        goto ERR_OUT1;
    }

    ret = pthread_mutex_init(&shellCB->historyMutex, NULL);
    if (ret != SH_OK) {
        goto ERR_OUT2;
    }

	//命令队列和命令历史队列初始化
    ret = (int)OsShellKeyInit(shellCB);
    if (ret != SH_OK) {
        goto ERR_OUT3;
    }
	//shell的工作目录初始化为根目录
    (void)strncpy_s(shellCB->shellWorkingDirectory, PATH_MAX, "/", 2); /* 2:space for "/" */

    sem_init(&shellCB->shellSem, 0, 0);

    g_shellCB = shellCB;
	//创建2个线程来处理shell的具体事务
    return OsShellCreateTask(shellCB);

ERR_OUT3:
    (void)pthread_mutex_destroy(&shellCB->historyMutex);
ERR_OUT2:
    (void)pthread_mutex_destroy(&shellCB->keyMutex);
ERR_OUT1:
    (void)free(shellCB);
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
