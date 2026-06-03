#include "UDPReceiver.h"

// raylib harus di-include SEBELUM winsock2.h agar struct `Rectangle` dan
// fungsi `DrawText` milik raylib dideklarasikan sebelum <windows.h> mengubah
// identifier tersebut menjadi macro (via wingdi.h / winuser.h).
#include <raylib.h>   // TraceLog, GetTime

#define WIN32_LEAN_AND_MEAN
#define NOGDI         // lewati wingdi.h (jika tidak, macro Rectangle tetap didefinisikan)
#define NOUSER        // lewati winuser.h (macro DrawText)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdio>
#include <cstdlib>

static_assert(sizeof(SOCKET) <= sizeof(unsigned long long),
              "SOCKET must fit in unsigned long long");

UDPReceiver::UDPReceiver(unsigned short port)
    : sock_(static_cast<unsigned long long>(INVALID_SOCKET)),
      valid_(false), wsaStarted_(false) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        TraceLog(LOG_WARNING, "UDP: WSAStartup failed (code %d) — bridge disabled", WSAGetLastError());
        return;
    }
    wsaStarted_ = true;

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        TraceLog(LOG_WARNING, "UDP: socket() failed (code %d) — bridge disabled", WSAGetLastError());
        return;
    }

    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);
    if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        TraceLog(LOG_WARNING, "UDP: bind() to port %u failed (code %d) — bridge disabled", port, WSAGetLastError());
        closesocket(s);
        return;
    }

    u_long nonBlocking = 1;
    if (ioctlsocket(s, FIONBIO, &nonBlocking) != 0) {
        TraceLog(LOG_WARNING, "UDP: ioctlsocket(FIONBIO) failed (code %d) — bridge disabled", WSAGetLastError());
        closesocket(s);
        return;
    }

    sock_  = static_cast<unsigned long long>(s);
    valid_ = true;
    TraceLog(LOG_INFO, "UDP: listening on 0.0.0.0:%u (non-blocking)", port);
}

UDPReceiver::~UDPReceiver() {
    if (valid_) closesocket(static_cast<SOCKET>(sock_));
    if (wsaStarted_) WSACleanup();
}

bool UDPReceiver::Poll() {
    // Edge adalah pulsa satu frame — selalu reset di awal.
    clickPressed_      = false;
    clickReleased_     = false;
    startWavePressed_  = false;
    upgradePressed_    = false;
    sellPressed_       = false;
    ultPressed_        = false;
    if (!valid_) return false;

    bool gotPacket = false;
    // Buffer berlebih untuk paket 8-field (float + 6 int + koma).
    char buf[128];
    SOCKET s = static_cast<SOCKET>(sock_);

    // Kuras antrian — simpan hanya nilai terbaru agar tidak pernah tertinggal.
    while (true) {
        int n = recvfrom(s, buf, (int)sizeof(buf) - 1, 0, nullptr, nullptr);
        if (n == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                TraceLog(LOG_WARNING, "UDP: recvfrom error %d", err);
            }
            break;
        }
        if (n <= 0) break;
        buf[n] = '\0';

        float nx = 0.0f, ny = 0.0f;
        int   c = 0, sw = 0, up = 0, sl = 0, ul = 0, hp = 0;
        // Tolak apa pun yang bukan tepat protokol 8-field.
        if (sscanf(buf, "%f,%f,%d,%d,%d,%d,%d,%d",
                   &nx, &ny, &c, &sw, &up, &sl, &ul, &hp) == 8 &&
            nx >= 0.0f && nx <= 1.0f && ny >= 0.0f && ny <= 1.0f) {
            x_              = nx;
            y_              = ny;
            clickDown_      = (c  != 0);
            startWaveDown_  = (sw != 0);
            upgradeDown_    = (up != 0);
            sellDown_       = (sl != 0);
            ultDown_        = (ul != 0);
            handPresent_    = (hp != 0);
            gotPacket       = true;
        }
    }

    if (gotPacket) {
        lastPacketTime_ = GetTime();

        clickPressed_     = ( clickDown_      && !prevClickDown_);
        clickReleased_    = (!clickDown_      &&  prevClickDown_);
        prevClickDown_    = clickDown_;

        startWavePressed_ = ( startWaveDown_  && !prevStartWave_);
        prevStartWave_    = startWaveDown_;

        upgradePressed_   = ( upgradeDown_    && !prevUpgrade_);
        prevUpgrade_      = upgradeDown_;

        sellPressed_      = ( sellDown_       && !prevSell_);
        prevSell_         = sellDown_;

        ultPressed_       = ( ultDown_        && !prevUlt_);
        prevUlt_          = ultDown_;
    }
    return gotPacket;
}

bool UDPReceiver::IsAlive() const {
    if (!valid_ || lastPacketTime_ < 0.0) return false;
    return (GetTime() - lastPacketTime_) < 1.0;
}
