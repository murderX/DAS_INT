#ifndef EDF_QUEUE_H
#define EDF_QUEUE_H

#include "ns3/queue.h"
#include "ns3/MyHeader.h"
#include "ns3/event-id.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/tcp-option.h"
#include <queue>
#include <map>
#include <vector>


namespace ns3
{
    class TraceContainer;
    class Comparator
    {
        public:
        bool operator() (Ptr<Packet> a, Ptr<Packet> b)
        {
            MyHeader ma, mb;
            a->PeekHeader(ma);
            b->PeekHeader(mb);
            Ptr<const IntTcpOption> itOptionA = DynamicCast<const IntTcpOption>(ma.tcp.t.GetOption(TcpOption::INT));
            Ptr<const IntTcpOption> itOptionB = DynamicCast<const IntTcpOption>(mb.tcp.t.GetOption(TcpOption::INT));
            return itOptionA->GetDeadline() > itOptionB->GetDeadline();
        }
    };


    template <typename Item>
    class EDFQueue:public Queue<Item>
    {
        public:

        uint32_t m_maxPackets;
        uint32_t m_maxBytes;
        uint32_t m_bytesInQueue;
        uint32_t m_nPackets;
        uint32_t max_packets=0;

        TracedCallback<Ptr<const Packet>> m_traceEnqueue;
		TracedCallback<Ptr<const Packet>> m_traceDequeue;
        TracedCallback<Ptr<const Packet>> m_traceDropped;

        EDFQueue();
        ~EDFQueue()
        {
            
        }
        static TypeId GetTypeId (void);
        virtual Ptr<Packet> Dequeue (void);
        virtual Ptr<Packet> Remove (void);
        virtual Ptr<const Packet> Peek (void) const;
        virtual bool Enqueue(Ptr<Packet> p);
        private:
        std::priority_queue<Ptr<Packet>, std::vector<Ptr<Packet>>, Comparator> q;
        std::queue<Ptr<Packet>>  q_norm;

    };
    
    extern template class EDFQueue<Packet>;
}

#endif
