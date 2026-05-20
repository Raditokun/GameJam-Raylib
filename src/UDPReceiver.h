#pragma once

// Non-blocking UDP receiver for the Python vision tracker bridge.
// Packet format (ASCII, 7 fields):
//     "X,Y,IS_CLICKING,START_WAVE,UPGRADE,SELL,ULT"
//   e.g. "0.4523,0.6210,1,0,0,0,0"
//   - X, Y are normalized to [0.0, 1.0]
//   - All flag fields are 0 or 1.
// Packets with any other field count are silently rejected (forces the Python
// half to stay in sync).
// Call Poll() exactly once per frame so click/gesture edge flags do not repeat.

class UDPReceiver {
public:
    explicit UDPReceiver(unsigned short port = 5005);
    ~UDPReceiver();

    UDPReceiver(const UDPReceiver&) = delete;
    UDPReceiver& operator=(const UDPReceiver&) = delete;

    // Drains every queued packet, keeping only the most recent values.
    // Returns true if at least one fresh packet was consumed this call.
    bool Poll();

    float NormalizedX()   const { return x_; }
    float NormalizedY()   const { return y_; }
    bool  ClickDown()     const { return clickDown_; }
    bool  ClickPressed()  const { return clickPressed_; }
    bool  ClickReleased() const { return clickReleased_; }

    // ── Gesture edges (one-frame pulses on rising edge) ─────
    bool  StartWavePressed() const { return startWavePressed_; }
    bool  UpgradePressed()   const { return upgradePressed_; }
    bool  SellPressed()      const { return sellPressed_; }
    bool  UltPressed()       const { return ultPressed_; }

    // True if a packet arrived within the last second.
    bool IsAlive() const;

private:
    // Hold an OS handle as unsigned long long so this header
    // does not need to pull in <winsock2.h>.
    unsigned long long sock_;
    bool valid_;
    bool wsaStarted_;

    float x_         = 0.0f;
    float y_         = 0.0f;
    bool  clickDown_      = false;
    bool  clickPressed_   = false;
    bool  clickReleased_  = false;
    bool  prevClickDown_  = false;

    // ── Gesture-action state (mirror the click pattern) ─────
    bool startWaveDown_ = false, prevStartWave_ = false, startWavePressed_ = false;
    bool upgradeDown_   = false, prevUpgrade_   = false, upgradePressed_   = false;
    bool sellDown_      = false, prevSell_      = false, sellPressed_      = false;
    bool ultDown_       = false, prevUlt_       = false, ultPressed_       = false;

    double lastPacketTime_ = -1.0;  // GetTime() of last successful packet
};
