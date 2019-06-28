#pragma once

//stl
#include <vector>
//self
#include "common/global.h"

namespace rs
{
namespace pciv
{

struct Msg
{
    enum Type
    {
        START_TRANS = 0,
        STOP_TRANS,
        READ_DONE,
        WRITE_DONE,
        CONF_ADV7842,
        QUERY_ADV7842,
        ACK
    };

    static const int32_t MaxDataLen = 32;

    int32_t type;

    uint8_t data[MaxDataLen];
};

struct MemoryInfo
{
    uint32_t phy_addr;
};

struct PosInfo
{
    int32_t start_pos;
    int32_t end_pos;
};

struct StreamInfo
{
    int32_t align_len;
    int32_t len;
    uint64_t pts;
    int32_t vdec_chn;
};

struct Adv7842Conf
{
    int mode;
};

struct QueryVIFmt
{
    VideoInputFormat fmt;
};

class Context
{
public:
    virtual ~Context() {}

    virtual int32_t Send(int32_t remote_id, int32_t port, uint8_t *data, int32_t len) = 0;

    virtual int32_t Recv(int32_t remote_id, int32_t port, uint8_t *data, int32_t len, int timeout) = 0;
};
} // namespace pciv

class PCIVComm : public pciv::Context
{
public:
    virtual ~PCIVComm();

    static PCIVComm *Instance();

    int32_t Initialize();

    void Close();

    int32_t Send(int32_t remote_id, int32_t port, uint8_t *data, int32_t len) override;

    int32_t Recv(int32_t remote_id, int32_t port, uint8_t *data, int32_t len, int timeout) override;

protected:
    static int32_t EnumChip();

    static int32_t OpenPort(int32_t remote_id, int32_t port, std::vector<std::vector<int32_t>> &remote_fds);

    static int32_t WaitConn(int32_t remote_id);

    explicit PCIVComm();

private:
    std::vector<std::vector<int32_t>> remote_fds_;
    bool init_;

    static const int32_t MsgPortBase;
    static const int32_t MaxPortNum;
};
}; // namespace rs