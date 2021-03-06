/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JavaScriptDebugServer_h
#define JavaScriptDebugServer_h

#include <kjs/debugger.h>

#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/RefPtr.h>

namespace JSC {
    class DebuggerCallFrame;
}

namespace WebCore {

    class Frame;
    class FrameView;
    class Page;
    class PageGroup;
    class PausedTimeouts;
    class JavaScriptCallFrame;
    class JavaScriptDebugListener;

    class JavaScriptDebugServer : JSC::Debugger {
    public:
        static JavaScriptDebugServer& shared();

        void addListener(JavaScriptDebugListener*);
        void removeListener(JavaScriptDebugListener*);

        void addListener(JavaScriptDebugListener*, Page*);
        void removeListener(JavaScriptDebugListener*, Page*);

        void addBreakpoint(int sourceID, unsigned lineNumber);
        void removeBreakpoint(int sourceID, unsigned lineNumber);
        bool hasBreakpoint(int sourceID, unsigned lineNumber) const;
        void clearBreakpoints();

        bool pauseOnExceptions() const { return m_pauseOnExceptions; }
        void setPauseOnExceptions(bool);

        void pauseProgram();
        void continueProgram();
        void stepIntoStatement();
        void stepOverStatement();
        void stepOutOfFunction();

        JavaScriptCallFrame* currentCallFrame();

        void pageCreated(Page*);

        typedef HashSet<JavaScriptDebugListener*> ListenerSet;
        typedef void (JavaScriptDebugListener::*JavaScriptExecutionCallback)();

    private:
        JavaScriptDebugServer();
        ~JavaScriptDebugServer();

        bool hasListeners() const { return !m_listeners.isEmpty() || !m_pageListenersMap.isEmpty(); }
        bool hasListenersInterestedInPage(Page*);

        void setJavaScriptPaused(const PageGroup&, bool paused);
        void setJavaScriptPaused(Page*, bool paused);
        void setJavaScriptPaused(Frame*, bool paused);
        void setJavaScriptPaused(FrameView*, bool paused);

        void dispatchFunctionToListeners(JavaScriptExecutionCallback, Page*);
        void pauseIfNeeded(Page*);

        virtual void sourceParsed(JSC::ExecState*, int sourceID, const JSC::UString& sourceURL, const JSC::SourceProvider& source, int startingLineNumber, int errorLine, const JSC::UString& errorMsg);
        virtual void callEvent(const JSC::DebuggerCallFrame&, int sourceID, int lineNumber);
        virtual void atStatement(const JSC::DebuggerCallFrame&, int sourceID, int firstLine);
        virtual void returnEvent(const JSC::DebuggerCallFrame&, int sourceID, int lineNumber);
        virtual void exception(const JSC::DebuggerCallFrame&, int sourceID, int lineNumber);
        virtual void willExecuteProgram(const JSC::DebuggerCallFrame&, int sourceId, int lineno);
        virtual void didExecuteProgram(const JSC::DebuggerCallFrame&, int sourceId, int lineno);
        virtual void didReachBreakpoint(const JSC::DebuggerCallFrame&, int sourceId, int lineno);

        typedef HashMap<Page*, ListenerSet*> PageListenersMap;
        PageListenersMap m_pageListenersMap;
        ListenerSet m_listeners;
        bool m_callingListeners;
        bool m_pauseOnExceptions;
        bool m_pauseOnNextStatement;
        bool m_paused;
        bool m_doneProcessingDebuggerEvents;
        JavaScriptCallFrame* m_pauseOnCallFrame;
        RefPtr<JavaScriptCallFrame> m_currentCallFrame;
        HashMap<RefPtr<Frame>, PausedTimeouts*> m_pausedTimeouts;
        HashMap<int, HashSet<unsigned>*> m_breakpoints;
    };

} // namespace WebCore

#endif // JavaScriptDebugServer_h
