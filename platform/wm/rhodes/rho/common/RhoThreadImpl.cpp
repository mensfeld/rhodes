/*------------------------------------------------------------------------
* (The MIT License)
* 
* Copyright (c) 2008-2011 Rhomobile, Inc.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* http://rhomobile.com
*------------------------------------------------------------------------*/

#include "stdafx.h"

#include "RhoThreadImpl.h"


namespace rho{
namespace common{
IMPLEMENT_LOGCLASS(CRhoThreadImpl,"RhoThreadWM");

CRhoThreadImpl::CRhoThreadImpl() : m_hAwakeEvent(0), m_hThread(0)
{
	LOG(INFO) + " JDP create CRhoThreadImpl(WM) object";
	m_hAwakeEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

CRhoThreadImpl::~CRhoThreadImpl()
{
    if ( m_hAwakeEvent )
        ::CloseHandle(m_hAwakeEvent);
}

static DWORD WINAPI runProc(void* pv) throw()
{
	IRhoRunnable* p = static_cast<IRhoRunnable*>(pv);
	p->runObject();
#if !defined(OS_WP8)
    ::ExitThread(0);
#endif
	return 0;
}

void CRhoThreadImpl::start(IRhoRunnable* pRunnable, IRhoRunnable::EPriority ePriority)
{
    m_hThread = ::CreateThread(NULL, 0, runProc, pRunnable, 0, NULL);
    setThreadPriority(ePriority);
}

void CRhoThreadImpl::setThreadPriority(IRhoRunnable::EPriority ePriority)
{
    int nPriority = THREAD_PRIORITY_NORMAL;
    if ( ePriority == IRhoRunnable::epHigh )
        nPriority = THREAD_PRIORITY_HIGHEST;
    else if (ePriority == IRhoRunnable::epLow)
        nPriority = THREAD_PRIORITY_LOWEST;
	else if (ePriority == IRhoRunnable::epCritical)
        nPriority = THREAD_PRIORITY_TIME_CRITICAL;

    ::SetThreadPriority(m_hThread,nPriority);
}

void CRhoThreadImpl::stop(unsigned int nTimeoutToKillMs)
{
    stopWait();
    if ( m_hThread )
    {
        DWORD dwRes = ::WaitForSingleObject( m_hThread, nTimeoutToKillMs );
        if ( dwRes != WAIT_OBJECT_0 )
        {
            LOG(INFO) + "Terminate thread. ID: " + ::GetCurrentThreadId() + "; Result: " + dwRes;
            ::TerminateThread(m_hThread,0);
        }
        ::CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}

int CRhoThreadImpl::wait(unsigned int nTimeoutMs)
{
    DWORD dwRes = ::WaitForSingleObject( m_hAwakeEvent, nTimeoutMs );
    if ( dwRes == WAIT_FAILED )
        LOG(ERROR) + "WaitForSingleObject failed. ID: " + ::GetCurrentThreadId() + "; Result: " + dwRes;

    return dwRes == WAIT_TIMEOUT ? 1 : 0;
}

void CRhoThreadImpl::stopWait()
{
    ::SetEvent(m_hAwakeEvent);
}

void CRhoThreadImpl::sleep(unsigned int nTimeoutMs)
{
    ::Sleep(nTimeoutMs);
}

}
}

extern "C" {

void* rho_nativethread_start()
{
    //TODO: rho_nativethread_start
    return 0;
}

void rho_nativethread_end(void* pData)
{
    //TODO: rho_nativethread_end
}

} //extern "C"
