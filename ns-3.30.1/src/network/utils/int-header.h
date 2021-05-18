#ifndef INT_HEADER_H
#define INT_HEADER_H

#include "ns3/buffer.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <stdint.h>
#include <cstdio>

namespace ns3{
    class IntHop{
        public:
        IntHop()
        {
            data.delay = 0;
            data.urgency = 0;
        }
        IntHop(uint64_t switch_id, uint32_t dl)
        {
            (void)switch_id;
            data.urgency=0;
            data.delay = dl;
        }
        //static uint32_t deltat;

        void setDelay(uint32_t d)
        {
            data.delay=d;
        }

        void setHop(uint64_t id, uint32_t d)
        {
            setDelay(d);
            data.urgency = 0;
        }

        bool setUrgency(uint32_t u)
        {
            data.urgency = u;
            return true;
        }
        uint32_t GetDelay()
        {
            return data.delay;
        }
        uint32_t GetUrgency()
        {
            return data.urgency;
        }
        uint32_t* Getbuf()
        {
            return buf;
        }
        private:
        union {
            struct{
                uint32_t delay;
                uint32_t urgency;
            }data;
            uint32_t buf[2];
        };
    };
    class IntHeader:public Header{
        public:
        uint8_t mode; //0 for disable int, 2 for normal, 1 for init, 3 for ack
        static uint8_t maxHop;
        uint8_t totalHop;
        uint8_t counter;
        uint32_t deadline;
        int64_t time;
        int64_t time_stamp;
        class IntHop* hop;
        IntHeader();
        IntHeader(uint16_t md, int64_t t);
        ~IntHeader();
        IntHeader& operator=(const IntHeader itHeader);
        // Not Finished
        uint32_t GetUrgency();
	    void PushHop(uint64_t switch_id, uint32_t dl);
        bool SetUrgency();
        static TypeId GetTypeId(void);
        virtual TypeId GetInstanceTypeId(void) const;
        virtual void Print(std::ostream &os) const;
        virtual uint32_t GetSerializedSize(void) const;
	    virtual void Serialize (Buffer::Iterator start) const;
	    virtual uint32_t Deserialize (Buffer::Iterator start);
        bool AddHop(IntHop nh);
        uint32_t GetStaticSize() const;
        void setTime(int64_t t);
        void setDelay();
    };
}

#endif /* INT_HEADER_H */