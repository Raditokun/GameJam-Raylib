#pragma once

// Penerima UDP non-blocking untuk bridge Python vision tracker.
// Format paket (ASCII, 8 field):
//     "X,Y,IS_CLICKING,START_WAVE,UPGRADE,SELL,ULT,HAND_PRESENT"
//   contoh "0.4523,0.6210,1,0,0,0,0,1"
//   - X, Y ternormalisasi ke [0.0, 1.0]
//   - Semua field flag bernilai 0 atau 1.
//   - HAND_PRESENT bernilai 1 selama tangan kanan (kursor) ada di frame, selain itu 0,
//     agar game bisa menyembunyikan crosshair, bukan membekukannya di tempat.
// Paket dengan jumlah field berbeda ditolak diam-diam (memaksa sisi Python
// tetap sinkron).
// Panggil Poll() tepat sekali per frame agar flag edge klik/gestur tidak berulang.

class UDPReceiver {
public:
    explicit UDPReceiver(unsigned short port = 5005);
    ~UDPReceiver();

    UDPReceiver(const UDPReceiver&) = delete;
    UDPReceiver& operator=(const UDPReceiver&) = delete;

    // Kuras semua paket di antrian, simpan hanya nilai paling baru.
    // Mengembalikan true jika minimal satu paket baru dikonsumsi pada panggilan ini.
    bool Poll();

    float NormalizedX()   const { return x_; }
    float NormalizedY()   const { return y_; }
    bool  ClickDown()     const { return clickDown_; }
    bool  ClickPressed()  const { return clickPressed_; }
    bool  ClickReleased() const { return clickReleased_; }
    bool  HandPresent()   const { return handPresent_; }

    // ── Edge gestur (pulsa satu frame pada rising edge) ─────
    bool  StartWavePressed() const { return startWavePressed_; }
    bool  UpgradePressed()   const { return upgradePressed_; }
    bool  SellPressed()      const { return sellPressed_; }
    bool  UltPressed()       const { return ultPressed_; }

    // True jika ada paket yang tiba dalam satu detik terakhir.
    bool IsAlive() const;

private:
    // Simpan handle OS sebagai unsigned long long agar header ini
    // tidak perlu menyertakan <winsock2.h>.
    unsigned long long sock_;
    bool valid_;
    bool wsaStarted_;

    float x_         = 0.0f;
    float y_         = 0.0f;
    bool  handPresent_   = false;   // tangan kanan (kursor) ada di frame pada paket ini
    bool  clickDown_      = false;
    bool  clickPressed_   = false;
    bool  clickReleased_  = false;
    bool  prevClickDown_  = false;

    // ── State aksi-gestur (mengikuti pola klik) ─────
    bool startWaveDown_ = false, prevStartWave_ = false, startWavePressed_ = false;
    bool upgradeDown_   = false, prevUpgrade_   = false, upgradePressed_   = false;
    bool sellDown_      = false, prevSell_      = false, sellPressed_      = false;
    bool ultDown_       = false, prevUlt_       = false, ultPressed_       = false;

    double lastPacketTime_ = -1.0;  // GetTime() dari paket sukses terakhir
};
