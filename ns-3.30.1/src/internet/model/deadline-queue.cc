#include "deadline-queue.h"
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

namespace ns3
{
    NS_LOG_COMPONENT_DEFINE ("DDLQueue");

    DDLQueue::DDLQueue()
    {
        m_bytesInQueue = 0;
        m_nPackets = 0;
        m_maxBytes = INT_MAX;
        q_mode = Urgency;
        m_maxPackets = 64;
    }


    TypeId DDLQueue::GetTypeId(void)
    {
        static TypeId tid = TypeId ("ns3::DDLQueue")
        .SetParent<Queue<Packet> > ()
        .SetGroupName ("TrafficControl")
        .AddConstructor<DDLQueue> ()
        ;
        return tid;
    }

    bool DDLQueue::Enqueue(Ptr<Packet> p)
    {
        if(q_mode&Urgency)
        {
            // std::cout<<"Begin Enqueue.\n";
            //Should check the availability of queue
            if(m_bytesInQueue + p->GetSize() <= m_maxBytes)
            {
                    // std::cout<<"Pushing\n";
                    MyHeader mh;
                    p->PeekHeader(mh);
                    if (mh.IntFlag == 0) {
                        q_normal.push(p);
                    }
                    else if (mh.GetIntHeader().mode == 3) {
                        q_drop.push(p);
                    }
                    else if ( mh.ip_header.GetProtocol() == 0x6) {
                        // TCP here, check if ACK
                        if(mh.tcp.t.GetFlags() & TcpHeader::ACK) {
                            q_drop.push(p);
                        }
                        else {
                            q.push(p);
                        }
                    }
                    else
                    {
                        q.push(p);
                    }
                    
                    // std::cout<<"Finished Pushing.."<<std::endl;
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
        else if(q_mode & Deadline)
        {
            return false;
        }
        else if(q_mode & Droptail)
        {
            q_drop.push(p);
            return true;
        }
        return false;
    }
    Ptr<Packet> DDLQueue::Dequeue (void)
    {
	//std::cout<<"Current Queue Length:"<<q.size()<<std::endl;
	//std::cout<<"Current Queue Drop Length:"<<q_drop.size()<<std::endl;
        if(q_mode & Urgency)
        {
            if(m_nPackets == 0 || m_bytesInQueue ==0)
            {
                // NS_LOG_LOGIC ("Queue empty");
                return 0;
            } 
            Ptr<Packet> p;
            if(q_drop.size() > 0)
            {
                p = q_drop.front();
                q_drop.pop();
                m_nPackets -= 1;
                m_bytesInQueue -= p->GetSize();
                m_traceDequeue(p);
                return p;
            }
            else if(q.size() > 0) {
                p = q.top();
                q.pop();
            }
            else if(q_normal.size() > 0) {
                p = q_normal.front();
                q_normal.pop();
            }
            else{
                return nullptr;
            }
            MyHeader mh(7, 1);
            p->RemoveHeader(mh);
            if(mh.IntFlag == 1) {
                if(mh.GetIntHeader().mode == 1 || mh.GetIntHeader().mode == 2)
                {
                   // Modify delay
                   mh.setDelay();
                   if(mh.GetIntHeader().mode == 2)
                      mh.SetUrgency();
                   mh.addCounter();
                }
            }
	    //std::cout<<"Poping normal packet with deadline "<<mh.GetIntHeader().deadline<<std::endl;
	    // mh.Print(std::cout);
            p->AddHeader(mh);
            m_nPackets -= 1;
            m_bytesInQueue -= p->GetSize();
            m_traceDequeue(p);
            return p;
        }
        return 0;
    }

    Ptr<Packet> DDLQueue::Remove (void)
    {
        return 0;
    }

    Ptr<const Packet> DDLQueue::Peek (void) const
    {
        // Ptr<const Packet> p = q.top();
        return DoPeek(begin());
    }
}
