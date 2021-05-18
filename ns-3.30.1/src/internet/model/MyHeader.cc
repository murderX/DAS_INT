#include "ns3/assert.h"
#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "MyHeader.h"

namespace ns3{
    NS_LOG_COMPONENT_DEFINE ("MyHeader");
    NS_OBJECT_ENSURE_REGISTERED (MyHeader);

    MyHeader::MyHeader()
    {
        header_type = L2_Header | L3_Header | L4_Header;
        pppProto = 0x0021;
        ip_header = Ipv4Header();
        IntFlag = 1;
    }
     MyHeader::MyHeader(uint32_t h_type, uint16_t i_flag)
    {
        header_type = h_type;
        pppProto = 0x0021;
        ip_header = Ipv4Header();
        IntFlag = i_flag;
        ip_header.SetProtocol(0);
    }
    MyHeader::~MyHeader()
    {
    }
    TypeId MyHeader::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::MyHeader")
                .SetParent<Header> ()
                .SetGroupName ("Internet")
                .AddConstructor<MyHeader> ()
            ;
        return tid;
    }
    TypeId MyHeader::GetInstanceTypeId (void) const
    {
        return GetTypeId();
    }
    void MyHeader::Print (std::ostream &os) const
    {
        os<<"HeaderType:"<<header_type<<"\n";
        os<<"Protocol Type:"<<int(ip_header.GetProtocol())<<"\n";
        os<<"IntFlag:"<<IntFlag<<"\n";
        if(IntFlag&&(L4_Header & header_type))
        {
            if(ip_header.GetProtocol() == 0x6)
            {
                tcp.ih.Print(os);
            }
            else if(ip_header.GetProtocol() == 0x11)
            {
                // udp.u.Print(os);
                udp.ih.Print(os);
            }
        }
    }
    uint32_t MyHeader::GetSerializedSize (void) const
    {
        uint32_t sz=0;
        if(header_type&L2_Header)
            sz += 2;
        if(header_type&L3_Header)
            sz += ip_header.GetSerializedSize();
        if(header_type&L4_Header)
        {
            if(ip_header.GetProtocol() == 0x6)
            {
                sz += tcp.t.GetSerializedSize();
	        if(IntFlag == 1)
                   sz += tcp.ih.GetSerializedSize();
            }
            else if(ip_header.GetProtocol() == 0x11)
            {
                sz += udp.u.GetSerializedSize();
		        if(IntFlag == 1)
                   sz += udp.ih.GetSerializedSize();
            }
        }
        return sz;
    }
    void MyHeader::Serialize (Buffer::Iterator start) const
    {
        Buffer::Iterator i = start;
        if(header_type&L2_Header)
        {
            i.WriteHtonU16(pppProto);
        }
        if(header_type&L3_Header)
        {
            ip_header.Serialize(i);
            i.Next(20);
        }
        if(header_type&L4_Header)
        {
            if(ip_header.GetProtocol()==0x6) //tcp
            {
                tcp.t.Serialize(i);
                i.Next(tcp.t.GetSerializedSize());
                if (IntFlag) {
                    tcp.ih.Serialize(i);
                }
                
            }
            else if(ip_header.GetProtocol()==0x11) //udp
            {
                udp.u.Serialize(i);
                i.Next(udp.u.GetSerializedSize());
                if(IntFlag)
                {
                    udp.ih.Serialize(i);
                    i.Next(udp.ih.GetSerializedSize());
                }
            }
        }
    }
    uint32_t MyHeader::Deserialize (Buffer::Iterator start)
    {
        // std::cout<<"Begin Deserialization!\n";
        //To be finished
        Buffer::Iterator i = start;
        uint32_t sz=0;
        if(header_type&L2_Header)
        {
            pppProto = i.ReadNtohU16();
            // i.Next(12);
            // std::cout<<"Finished L2\n";
            int l2_size = 2;
            sz += l2_size;
        }

        if(header_type&L3_Header)
        {
            int l3_size = ip_header.Deserialize(i);
            sz += l3_size;
            i.Next(l3_size);
            // std::cout<<"Finished IP\n";
        }

        if(header_type&L4_Header)
        {
            int l4_size = 0;
            // TODO(huasunsx): Try elegant way,
            // Now in TCP, we use option code 15 to represent the 
            // existing of INT header, in UDP, we just assume there needs
            // to be an INT header.
            if(ip_header.GetProtocol() == 0x6) //tcp
            {
                l4_size += tcp.t.Deserialize(i);
                i.Next(l4_size);
                if( tcp.t.HasOption(TcpOption::INT))
                {
                    IntFlag = 1;
                    tcp.ih.Deserialize(i);
                }
                else
                {
                    IntFlag = 0;
                }
                
                
            }
            else if(ip_header.GetProtocol() == 0x11)//udp
            {
                l4_size += udp.u.Deserialize(i);
                i.Next(l4_size);
                if( IntFlag == 1)
                {
                    l4_size += udp.ih.Deserialize(i);
                    i.Next(udp.ih.GetSerializedSize());
                }
            }
            sz += l4_size;
        }
        return sz;
    }
    IntHeader MyHeader::GetIntHeader()
    {
        if((header_type& L4_Header) && IntFlag)
            {
                if(ip_header.GetProtocol()==0x6)
                    return tcp.ih;
                else if(ip_header.GetProtocol() == 0x11)
                    return udp.ih;
            }
            else
            {
                std::cout<<"No Int Header included!"<<std::endl;
                IntHeader empty;
                return empty;
            }
            IntHeader empty;
            return empty;
    }
}


using namespace ns3;

int main(int argc, char** argv)
{
    Packet::EnablePrinting();
    MyHeader mh(7, 1);
    mh.ip_header.SetProtocol(0x6);
    mh.tcp.t.SetSourcePort(0x1234);
    // mh.udp.u.ForcePayloadSize(0);
    mh.tcp.ih.mode = 1;
    mh.tcp.ih.deadline = 15;
    mh.tcp.ih.PushHop(1, 6);
    mh.tcp.ih.PushHop(2, 7);
    mh.tcp.ih.PushHop(7, 4);
    mh.tcp.ih.mode = 2;

    mh.tcp.ih.SetUrgency();

    mh.Print(std::cout);
    // mh.ip_header.Print(std::cout);
    std::cout<<"\n"<<mh.GetSerializedSize()<<"\n";

    Ptr<Packet> p = Create<Packet> ();
    p->AddHeader(mh);
    // p->AddPaddingAtEnd(1000);

    MyHeader dst;
    dst.header_type=7;
    p->RemoveHeader(dst);
    dst.Print(std::cout);
    // dst.ip_header.Print(std::cout);
    std::cout<<"\n"<<dst.GetSerializedSize()<<"\n";

}
