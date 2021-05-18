#include "int-header.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include <iostream>

namespace ns3{
    uint8_t IntHeader::maxHop = 8;
     IntHeader::IntHeader()
    {
        mode=0;
        deadline=10;
        hop = new IntHop[maxHop+1];
        totalHop = 0;
        counter = 0;
        time = Simulator::Now().GetMicroSeconds();
        time_stamp=time;
    }
    IntHeader::IntHeader(uint16_t md, int64_t t)
    {
        mode = md;
        time = t;
        hop = new IntHop[maxHop+1];
        totalHop = 0;
        counter = 0;
        deadline=10;
        time_stamp=time;
    }

    IntHeader& IntHeader::operator=(const IntHeader itHeader){
        mode = itHeader.mode;
        totalHop = itHeader.totalHop;
        counter = itHeader.counter;
        deadline = itHeader.deadline;
        time = itHeader.time;
        time_stamp = itHeader.time_stamp;
        for(int j = 0; j<(int)totalHop; j++){
            uint32_t* buf = hop[j].Getbuf();
            buf[0] = itHeader.hop[j].Getbuf()[0];
            buf[1] = itHeader.hop[j].Getbuf()[1];
        }
        return *this;
    }

    void IntHeader::setTime(int64_t t)
    {
        time = t;
    }
    void IntHeader::setDelay()
    {
        int64_t delay = Simulator::Now().GetMicroSeconds() - time;
        hop[counter].setDelay(uint32_t(delay));
    }
    TypeId IntHeader::GetTypeId(void)
        {
            static TypeId tid=TypeId("ns3::IntHeader")
            .SetParent<Header> ()
            .AddConstructor<IntHeader> ()
            ;
            return tid;
        }
    TypeId IntHeader::GetInstanceTypeId(void) const
        {
            return GetTypeId();
        }
    
    IntHeader::~IntHeader()
    {
        hop = NULL;
    }
    void IntHeader::Print(std::ostream &os) const
        {
            os<<"Deadline is "<<deadline<<". There are currently "<<(int)totalHop<<" hops. The size of INT header is "<<GetSerializedSize()<<"\n";
            if(totalHop>0)
            {
                os<<"Each Node's urgency is\n";
                for(int i=0; i<(int)totalHop; i++)
                {
                    os<<"Node "<<i<<": "<<hop[i].GetUrgency()<<"\n";
                }
                os<<"Current Urgency is "<<hop[counter].GetUrgency()<<"\n";
            }
        }
    uint32_t IntHeader:: GetSerializedSize(void) const
        {
            return sizeof(mode) + sizeof(totalHop) + sizeof(counter) + sizeof(deadline) + sizeof(time) + sizeof(IntHop) * maxHop + sizeof(time_stamp);
        }

    void IntHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        i.WriteU8(mode);
        i.WriteU8(totalHop);
        i.WriteU8(counter);
        i.WriteU32(deadline);
        i.WriteU64(time);
        i.WriteU64(time_stamp);
        for(int j=0;j<maxHop;j++)
        {
            uint32_t* buf = hop[j].Getbuf();
            i.WriteU32(buf[0]);
            i.WriteU32(buf[1]);
        }
    }

    uint32_t IntHeader::Deserialize (Buffer::Iterator start){
        Buffer::Iterator i = start;
        mode = i.ReadU8();
        totalHop = i.ReadU8();
        counter = i.ReadU8();
        deadline=i.ReadU32();
        time = i.ReadU64();
        time_stamp = i.ReadU64();
        if(mode != 0){
            for(int j=0;j<maxHop;j++)
            {
                uint32_t* buf = hop[j].Getbuf();
                buf[0]=i.ReadU32();
                buf[1]=i.ReadU32();
            }
            return sizeof(mode) + sizeof(totalHop) + sizeof(counter) + sizeof(deadline) + sizeof(time) + sizeof(IntHop) * maxHop + sizeof(time_stamp);
        }
        else
            return sizeof(mode) + sizeof(totalHop) + sizeof(counter) + sizeof(deadline) + sizeof(time) + sizeof(time_stamp);
        
    }
    
    uint32_t IntHeader::GetUrgency()
    {
        if(mode==2)
        {
            return hop[counter].GetUrgency();
        }
        return deadline;
    }

    void IntHeader::PushHop(uint64_t switch_id, uint32_t dl)
    {
        if(mode==1)
        {
            IntHop ih(switch_id, dl);
            hop[totalHop++] = ih;
        }
        else
            printf("Error mode for pushhop.\n");
        
    }
    bool IntHeader::AddHop(IntHop nh)
    {
        if(mode == 1)
        {
            hop[totalHop++] = nh;
            return true;
        }
        return false;
    }
    bool IntHeader::SetUrgency()
    {
        uint32_t sum = 0;
        for(int i=0; i<(int)totalHop;i++)
        {
            sum += hop[i].GetDelay();
        }
        for(int i=0; i<(int)totalHop; i++)
        {
            if(deadline < sum- hop[i].GetDelay())
                hop[i].setUrgency(0);
            else
            {
                hop[i].setUrgency(deadline - (sum - hop[i].GetDelay()));
            }
        }
        return true;
    }
}

// using namespace ns3;
// int main(int argc, char **argv)
// {
//     Packet::EnablePrinting();
//     Ptr<Packet> p = Create<Packet> ();
//     IntHeader i;
//     i.mode=1;
//     i.deadline = 15;

//     i.PushHop(1, 3);
//     i.PushHop(3, 4);
//     i.PushHop(4, 5);
//     i.SetUrgency();

//     i.Print(std::cout);
//     p->AddHeader(i);

//     IntHeader d;
//     p->RemoveHeader(d);
//     d.Print(std::cout);

//     return 0;

// }
