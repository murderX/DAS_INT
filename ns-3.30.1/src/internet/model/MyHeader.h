#ifndef MY_HEADER_H
#define MY_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/int-header.h"
#include "ns3/int-tcp-option.h"

namespace ns3{
    enum HeaderType{
	L2_Header = 1,
	L3_Header = 2,
	L4_Header = 4
    };
    class MyHeader:public Header{
        public:
        uint32_t header_type;
        uint16_t IntFlag;
        MyHeader();
        MyHeader(uint32_t h_type, uint16_t i_flag);
        ~MyHeader();
        // PPP 
        uint16_t pppProto=0x0021;
        // Ipv4
        Ipv4Header ip_header;
        //L4 Header
        struct{
                TcpHeader t;
                IntHeader ih;
        }tcp;
        struct{
                UdpHeader u;
                IntHeader ih;
        }udp;
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        virtual void Print (std::ostream &os) const;
        virtual uint32_t GetSerializedSize (void) const;
        virtual void Serialize (Buffer::Iterator start) const;
        virtual uint32_t Deserialize (Buffer::Iterator start);
        void setTime(uint64_t t)
        {
            {
            if(header_type& L4_Header)
            {
                IntHeader ih;
                if(ip_header.GetProtocol()==0x6)
                    tcp.ih.setTime(t);
                else if(ip_header.GetProtocol() == 0x11)
                    udp.ih.setTime(t);
            }
        }
        }
        void setDelay(){
            if(header_type& L4_Header)
            {
                IntHeader ih;
                if(ip_header.GetProtocol()==0x6)
                    tcp.ih.setDelay();
                else if(ip_header.GetProtocol() == 0x11)
                    udp.ih.setDelay();
            }
        }
        void addCounter()
        {
            if(header_type& L4_Header)
            {
                IntHeader ih;
                if(ip_header.GetProtocol()==0x6)
                    tcp.ih.counter++;
                else if(ip_header.GetProtocol() == 0x11)
                    udp.ih.counter++;
            }
        }
        void PushHop(uint64_t switch_id, uint32_t dl)
        {
            if(header_type& L4_Header)
            {
                if(ip_header.GetProtocol()==0x6)
                    tcp.ih.PushHop(switch_id, dl);
                else if(ip_header.GetProtocol() == 0x11)
                    udp.ih.PushHop(switch_id, dl);
            }
        }

        void UpdatingINT(){
            
        }

        void SetUrgency()
        {
            if(header_type& L4_Header)
            {
                if(ip_header.GetProtocol()==0x6)
                    tcp.ih.SetUrgency();
                else if(ip_header.GetProtocol() == 0x11)
                    udp.ih.SetUrgency();
            }
        }
        IntHeader GetIntHeader();
    };
}

#endif
