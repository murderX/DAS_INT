#include "ns3/MyHeader.h"
#include "ns3/packet.h"
#include "ns3/int-tcp-option.h"
#include "ns3/core-module.h"

using namespace ns3;

int main(int argc, char** argv)
{
    Packet::EnablePrinting();
    MyHeader mh(7, 1);
    mh.ip_header.SetProtocol(0x6);
    mh.tcp.t.SetSourcePort(0x1234);
    Ptr<IntTcpOption> itOption = CreateObject<IntTcpOption>();
    itOption->SetMode(3);
    mh.tcp.t.AppendOption(itOption);
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

    MyHeader dst(7,1);
    p->RemoveHeader(dst);
    dst.Print(std::cout);
    // dst.ip_header.Print(std::cout);
    std::cout<<"\n"<<dst.GetSerializedSize()<<"\n";

}
