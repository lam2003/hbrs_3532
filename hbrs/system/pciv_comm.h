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
        NOTIFY_MEMORY = 0,
        READ_DONE,
        WRITE_DONE
    };

    static const int32_t MaxDataLen = 32;

    int32_t type;

    uint8_t data[MaxDataLen];

} __attribute__((aligned(1)));

struct MemoryInfo
{
    uint32_t phy_addr;
} __attribute__((aligned(1)));

struct PosInfo
{
    int32_t start_pos;
    int32_t end_pos;
} __attribute__((aligned(1)));

struct StreamInfo
{
    int32_t align_len;
    int32_t len;
    uint64_t pts;
    int32_t vdec_chn;
} __attribute__((aligned(1)));

class Context
{
public:
    virtual ~Context() {}

    virtual int32_t Send(int32_t remote_id, int32_t port, uint8_t *data, int32_t len) = 0;

    virtual int32_t Recv(int32_t remote_id, int32_t port, uint8_t *data, int32_t len,int timeout) = 0;

    virtual int32_t GetTransReadPort() = 0;

    virtual int32_t GetTransWritePort() = 0;

    virtual int32_t GetCMDPort() = 0;
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

    int32_t GetTransReadPort() override;

    int32_t GetTransWritePort() override;

    int32_t GetCMDPort() override;

protected:
    static int32_t EnumChip();

    static int32_t OpenPort(int32_t remote_id, int32_t port, std::vector<std::vector<int32_t>> &remote_fds);

    static int32_t WaitConn(int32_t remote_id);

    explicit PCIVComm();

private:
    std::vector<std::vector<int32_t>> remote_fds_;
    bool init_;

    static const int32_t CommCMDPort;
    static const int32_t TransReadPort;
    static const int32_t TransWritePort;
    static const int32_t MsgPortBase;
    static const int32_t MaxPortNum;
};
}; // namespace rs