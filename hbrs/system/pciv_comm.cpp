//self
#include "system/pciv_comm.h"
#include "common/err_code.h"

namespace rs
{

using namespace pciv;

const int32_t PCIVComm::MsgPortBase = 100;
const int32_t PCIVComm::MaxPortNum = 3;

PCIVComm *PCIVComm::Instance()
{
    static PCIVComm *instance = new PCIVComm;
    return instance;
}

PCIVComm::PCIVComm() : init_(false)
{
}

PCIVComm::~PCIVComm()
{
    Close();
}

int32_t PCIVComm::Initialize()
{
    if (init_)
        return KInitialized;

    int32_t ret;

    ret = EnumChip();
    if (ret != KSuccess)
        return ret;

    remote_fds_.resize(PCIV_MAX_CHIPNUM);
    for (int32_t i = 0; i < PCIV_MAX_CHIPNUM; i++)
    {
        remote_fds_[i].resize(MaxPortNum);
        for (int32_t j = 0; j < MaxPortNum; j++)
            remote_fds_[i][j] = -1;
    }

    ret = OpenPort(RS_PCIV_MASTER_ID, RS_PCIV_CMD_PORT, remote_fds_);
    if (ret != KSuccess)
        return ret;
    ret = OpenPort(RS_PCIV_MASTER_ID, RS_PCIV_TRANS_READ_PORT, remote_fds_);
    if (ret != KSuccess)
        return ret;
    ret = OpenPort(RS_PCIV_MASTER_ID, RS_PCIV_TRANS_WRITE_PORT, remote_fds_);
    if (ret != KSuccess)
        return ret;
    ret = WaitConn(RS_PCIV_MASTER_ID);
    if (ret != KSuccess)
        return ret;

    init_ = true;

    return KSuccess;
}

void PCIVComm::Close()
{
    if (!init_)
        return;
    remote_fds_.resize(PCIV_MAX_CHIPNUM);
    for (int32_t i = 0; i < PCIV_MAX_CHIPNUM; i++)
    {
        remote_fds_[i].resize(MaxPortNum);
        for (int32_t j = 0; j < MaxPortNum; j++)
            if (remote_fds_[i][j] != -1)
                close(remote_fds_[i][j]);
    }

    init_ = false;
}

int32_t PCIVComm::WaitConn(int32_t remote_id)
{
    int32_t ret;

    const char *dev = "/dev/mcc_userdev";
    int32_t fd = open(dev, O_RDWR);
    if (fd <= 0)
    {
        log_e("open %s failed,%s", dev, strerror(errno));
        return KSystemError;
    }

    hi_mcc_handle_attr attr;
    attr.target_id = remote_id;
    attr.port = 1000;
    attr.priority = 0;

    ret = ioctl(fd, HI_MCC_IOC_CONNECT, &attr);
    if (ret != KSuccess)
    {
        log_e("ioctl HI_MCC_IOC_CONNECT failed,%s", strerror(errno));
        return KSystemError;
    }

    while (ioctl(fd, HI_MCC_IOC_CHECK, &attr))
        usleep(10000); //10ms

    log_d("chip[%d] connected", remote_id);

    close(fd);
    return KSuccess;
}

int32_t PCIVComm::OpenPort(int32_t remote_id, int32_t port, std::vector<std::vector<int32_t>> &remote_fds)
{
    int32_t ret;

    const char *dev = "/dev/mcc_userdev";
    int32_t fd = open(dev, O_RDWR | O_NONBLOCK);
    if (fd <= 0)
    {
        log_e("open %s failed,%s", dev, strerror(errno));
        return KSystemError;
    }

    hi_mcc_handle_attr attr;
    attr.target_id = remote_id;
    attr.port = port + MsgPortBase;
    attr.priority = 2;

    ret = ioctl(fd, HI_MCC_IOC_CONNECT, &attr);
    if (ret != KSuccess)
    {
        log_e("ioctl HI_MCC_IOC_CONNECT failed,%s", strerror(errno));
        return KSystemError;
    }

    log_d("chip[%d] port[%d] fd[%d]", remote_id, port + MsgPortBase, fd);
    remote_fds[remote_id][port] = fd;
    return KSuccess;
}

int32_t PCIVComm::EnumChip()
{
    int32_t ret;

    const char *dev = "/dev/mcc_userdev";
    int32_t fd = open(dev, O_RDWR);
    if (fd < 0)
    {
        log_e("open %s failed,%s", dev, strerror(errno));
        return KSystemError;
    }

    hi_mcc_handle_attr attr;
    memset(&attr, 0, sizeof(attr));

    ret = ioctl(fd, HI_MCC_IOC_ATTR_INIT, &attr);
    if (ret < 0)
    {
        log_e("ioctl HI_MCC_IOC_ATTR_INIT failed,%s", strerror(errno));
        return KSystemError;
    }

    close(fd);
    return KSuccess;
}

int32_t PCIVComm::Send(int32_t remote_id, int32_t port, uint8_t *data, int32_t len)
{
    if (!init_)
        return KUnInitialized;

    int32_t ret;
    int32_t fd = remote_fds_[remote_id][port];

    int32_t rest_len = len;
    int32_t offset = 0;

    while (rest_len)
    {
    again:
        ret = write(fd, data + offset, rest_len);
        if (ret < 0 && errno == EINTR)
        {
            goto again;
        }
        else if (ret < 0)
        {
            log_e("write failed,%s", strerror(errno));
            return KSystemError;
        }
        offset += ret;
        rest_len -= ret;
    }
    return KSuccess;
}

int32_t PCIVComm::Recv(int32_t remote_id, int32_t port, uint8_t *data, int32_t len, int timeout)
{
    if (!init_)
        return KInitialized;

    fd_set fds;
    timeval tv;

    int32_t fd = remote_fds_[remote_id][port];

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = timeout;

    int ret = select(fd + 1, &fds, NULL, NULL, &tv);
    if (ret <= 0)
        return 0;

    ret = read(fd, data, len);
    if (ret < 0)
    {
        log_e("read failed,%s", strerror(errno));
        return KSystemError;
    }
    
    return ret;
}

} // namespace rs