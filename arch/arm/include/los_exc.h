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

/**
 * @defgroup los_exc Exception handling
 * @ingroup kernel
 */
#ifndef _LOS_EXC_H
#define _LOS_EXC_H

#include "los_typedef.h"
#include "arch_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_exc
 * Register information structure
 *
 * Description: register information stored when an exception occurs on an LPC2458 platform.
 *
 * Note: The following register names without uw are the register names used in the chip manual.
 */
#ifdef LOSCFG_ARCH_ARM_AARCH64
//64位ARM异常处理
#define EXC_GEN_REGS_NUM     30  //通用寄存器个数
typedef struct {
    UINT64 X[EXC_GEN_REGS_NUM]; /**< Register X0-X29 */ //通用寄存器列表
    UINT64 LR;                  /**< Program returning address. X30 */ //程序返回地址
    UINT64 SP; //栈寄存器
    UINT64 regELR;
    UINT64 SPSR;
} ExcContext;   //64位ARM异常信息上下文
#else
//32位ARM异常处理上下文
typedef struct {
    UINT32 USP;     /**< User mode stack pointer */
    UINT32 ULR;     /**< User mode program returning address */
    UINT32 regCPSR; /**< Current program status register (CPSR) */
    UINT32 R0;      /**< Register R0 */
    UINT32 R1;      /**< Register R1 */
    UINT32 R2;      /**< Register R2 */
    UINT32 R3;      /**< Register R3 */
    UINT32 R4;      /**< Register R4 */
    UINT32 R5;      /**< Register R5 */
    UINT32 R6;      /**< Register R6 */
    UINT32 R7;      /**< Register R7 */
    UINT32 R8;      /**< Register R8 */
    UINT32 R9;      /**< Register R9 */
    UINT32 R10;     /**< Register R10 */
    UINT32 R11;     /**< Register R11 */
    UINT32 R12;     /**< Register R12 */
    UINT32 SP;      /**< Stack pointer */
    UINT32 LR;      /**< Program returning address. */
    UINT32 PC;      /**< PC pointer of the exceptional function */
} ExcContext;  //32位ARM的异常信息上下文
#endif

/**
 * @ingroup los_exc
 * Exception information structure
 *
 * Description: exception information stored when an exception occurs on an LPC2458 platform.
 *
 */
typedef struct {
    UINT16 phase;        /**< Phase in which an exception occurs */ //异常产生的阶段
    UINT16 type;         /**< Exception type */ //异常种类
    UINT16 nestCnt;      /**< Count of nested exception */ //异常嵌套层数
    UINT16 reserved;     /**< Reserved for alignment */ //用于结构体对齐
    ExcContext *context; /**< Hardware context when an exception occurs */ //异常发生时的硬件状态
} ExcInfo; //异常信息控制块

/**
 * @ingroup los_exc
 * @brief Kernel FP Register address obtain function.
 *
 * @par Description:
 * The API is used to obtain the FP Register address.
 * @attention None.
 *
 * @param  None.
 *
 * @retval #UINTPTR The FP Register address.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
STATIC INLINE UINTPTR Get_Fp(VOID)  //获取frame pointer寄存器的内容
{
    UINTPTR regFp;

#ifdef LOSCFG_ARCH_ARM_AARCH64
    __asm__ __volatile__("mov %0, X29" : "=r"(regFp));
#else
    __asm__ __volatile__("mov %0, fp" : "=r"(regFp));
#endif

    return regFp;
}

/**
 * @ingroup los_exc
 * @brief Define an exception handling function hook.
 *
 * @par Description:
 * This API is used to define the exception handling function hook based on the type of
 * the exception handling function and record exceptions.
 * @attention None.
 *
 * @param None.
 *
 * @retval None.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
 //异常处理函数原型
typedef VOID (*EXC_PROC_FUNC)(UINT32, ExcContext *, UINT32, UINT32);

/**
 * @ingroup los_exc
 * @brief Register an exception handling hook.
 *
 * @par Description:
 * This API is used to register an exception handling hook.
 * @attention If the hook is registered for multiple times, the hook registered at the last time is effective.
 * @attention The hook can be registered as NULL, indicating that the hook registration is canceled.
 * @param excHook [IN] Type #EXC_PROC_FUNC: hook function.
 *
 * @retval #LOS_OK                      The exception handling hook is successfully registered.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
 //注册异常处理函数
extern UINT32 LOS_ExcRegHook(EXC_PROC_FUNC excHook);

/**
 * @ingroup los_exc
 * @brief Kernel panic function.
 *
 * @par Description:
 * Stack function that prints kernel panics.
 * @attention After this function is called and stack information is printed, the system will fail to respond.
 * @attention The input parameter can be NULL.
 * @param fmt [IN] Type #CHAR* : variadic argument.
 *
 * @retval #None.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
 //内核出错比较严重的时候使用此函数输出当前调用栈等信息
VOID LOS_Panic(const CHAR *fmt, ...);

/**
 * @ingroup los_exc
 * @brief Kernel backtrace function.
 *
 * @par Description:
 * Backtrace function that prints task call stack information traced from the running task.
 * @attention None.
 *
 * @param None.
 *
 * @retval #None.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
 //输出调用栈回溯
extern VOID OsBackTrace(VOID);

/**
 * @ingroup los_exc
 * @brief Kernel task backtrace function.
 *
 * @par Description:
 * Backtrace function that prints task call stack information traced from the input task.
 * @attention
 * <ul>
 * <li>The input taskID should be valid.</li>
 * </ul>
 *
 * @param  taskID [IN] Type #UINT32 Task ID.
 *
 * @retval #None.
 *
 * @par Dependency:
 * los_exc.h: the header file that contains the API declaration.
 * @see None.
 */
 //输出指定任务的调用栈
extern VOID OsTaskBackTrace(UINT32 taskID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_EXC_H */
