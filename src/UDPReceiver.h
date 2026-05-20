#pragma once

// Non-blocking UDP receiver for the Python vision tracker bridge.
// Packet format (ASCII): "X,Y,IS_CLICKING"  e.g. "0.4523,0.6210,1"
//   - X, Y are normalized to [0.0, 1.0]
//   - IS_CLICKING is 0 or 1 (pinch gesture)
// Call Poll() exactly once per frame so click edge flags do not repeat.

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

    double lastPacketTime_ = -1.0;  // GetTime() of last successful packet
};
