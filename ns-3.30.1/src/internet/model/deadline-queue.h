#ifndef DDL_QUEUE_H
#define DDL_QUEUE_H

#include "ns3/queue.h"
#include "ns3/MyHeader.h"
#include "ns3/event-id.h"
#include "ns3/drop-tail-queue.h"
#include <queue>
#include <map>
#include <vector>


namespace ns3{
    class TraceContainer;
    class MyHeader;
    class DDLQueue_Compare
    {
        public:
        bool operator () (Ptr<Packet> const &a, Ptr<Packet> const &b)
        {
            MyHeader ha(7, 1), hb(7, 1);
            a->PeekHeader(ha);
            // std::cout<<"Finished Removing a\n";
            b->PeekHeader(hb);
            // std::cout<<"Finished Removing b\n";
            if(ha.IntFlag == 1  && hb.IntFlag == 1)
            {
                 return ha.GetIntHeader().GetUrgency() + ha.GetIntHeader().time > hb.GetIntHeader().GetUrgency() + hb.GetIntHeader().time;
            }
            else if(ha.IntFlag == 1|| hb.IntFlag != 1)
            {
                return false;
            }
            else 
            {
                return true;
            }
            return false;
        }
    };
    class DDLQueue:public Queue<Packet>
    {
        enum DDLQ
        {
            Urgency=1,
            Deadline = 2,
            Droptail = 4
        };
        public:

        uint32_t m_maxPackets;
        uint32_t m_maxBytes;
        uint32_t m_bytesInQueue;
        uint32_t m_nPackets;
        uint32_t q_mode;

        TracedCallback<Ptr<const Packet>> m_traceEnqueue;
		TracedCallback<Ptr<const Packet>> m_traceDequeue;
        TracedCallback<Ptr<const Packet>> m_traceDropped;

        DDLQueue();
        ~DDLQueue()
        {
            
        }
        static TypeId GetTypeId (void);
        virtual Ptr<Packet> Dequeue (void);
        virtual Ptr<Packet> Remove (void);
        virtual Ptr<const Packet> Peek (void) const;
        virtual bool Enqueue(Ptr<Packet> p);
        void PrintQueue()
        {
            if(q_mode == Urgency)
            {
                std::vector<Ptr<Packet>> v;

            }
        }

        private:
        std::priority_queue<Ptr<Packet>, std::vector<Ptr<Packet>>, DDLQueue_Compare> q;
        // DropTailQueue<Packet> q_drop;
        std::queue<Ptr<Packet>> q_drop;
        std::queue<Ptr<Packet>> q_normal;
    };
}














#endif
