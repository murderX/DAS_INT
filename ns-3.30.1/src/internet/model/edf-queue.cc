#include "edf-queue.h"
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE ("EDFQueue");
    NS_OBJECT_TEMPLATE_CLASS_DEFINE (EDFQueue,Packet);    

    template <typename Item>
    EDFQueue<Item>::EDFQueue()
    {
        m_bytesInQueue = 0;
        m_nPackets = 0;
        m_maxBytes = INT_MAX;
        m_maxPackets = 64;
    }

    template <typename Item>
    TypeId EDFQueue<Item>::GetTypeId(void)
    {
        static TypeId tid = TypeId (("ns3::EDFQueue" + GetTypeParamName<EDFQueue<Item> > () + ">").c_str ())
        .SetParent<Queue<Item> > ()
        .SetGroupName ("Internet")
        .template AddConstructor<EDFQueue<Item>> ()
        ;
        return tid;
    }

    template <typename Item>
    bool EDFQueue<Item>::Enqueue(Ptr<Packet> p)
    {
        if(m_bytesInQueue + p->GetSize() <= m_maxBytes)
            {
                    // std::cout<<"Pushing\n";
                    MyHeader mh;
                    p->PeekHeader(mh);
                    if(mh.ip_header.GetProtocol() == 0x6 && mh.tcp.t.HasOption(TcpOption::INT)) {
                        q.push(p);
                    }
                    else {
                        q_norm.push(p);
                    }
                    m_bytesInQueue += p->GetSize();
		   
                    
            }
            else
            {
                // NS_LOG_LOGIC ("Queue full -- dropping pkt");
                // NS_LOG_FUNCTION (this << p);
                m_traceDropped(p);
                return false;
            }
            m_nPackets += 1;
            // NS_LOG_LOGIC ("m_traceEnqueue (p)");
            m_traceEnqueue(p);
            return true;
    }

    template <typename Item>
    Ptr<Packet> EDFQueue<Item>::Dequeue (void)
    {
	//std::cout<<"Current Queue Length:"<<q.size()<<std::endl;
	//std::cout<<"Current Queue Drop Length:"<<q_drop.size()<<std::endl;
        if(m_nPackets == 0 || m_bytesInQueue ==0)
            {
                // NS_LOG_LOGIC ("Queue empty");
                return 0;
            } 
        Ptr<Packet> p;
        if(q.size() > 0) {
           p = q.top();
           q.pop();
        }
        else {
           p = q_norm.front();
           q_norm.pop();
        }
        m_nPackets -= 1;
        m_bytesInQueue -= p->GetSize();
        m_traceDequeue(p);
        return p;
    }

    template <typename Item>
    Ptr<Packet> EDFQueue<Item>::Remove (void)
    {
        return 0;
    }

    template <typename Item>
    Ptr<const Packet> EDFQueue<Item>::Peek (void) const
    {
        // Ptr<const Packet> p = q.top();
        return 0;
    }
}
