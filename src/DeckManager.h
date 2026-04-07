#pragma once
#include <vector>
#include "Card.h"

class DeckManager {
public:
    // Draft pool
    std::vector<Card> pool;
    std::vector<int>  draftPicks;   // indices into pool
    bool draftComplete;

    // Active hand (PLAYING)
    std::vector<Card> hand;
    int selectedHandIndex;

    DeckManager();
    void InitPool();

    // Drafting
    void UpdateDrafting();
    void DrawDrafting() const;
    bool IsDraftReady() const;
    void FinalizeDraft();

    // Playing
    void UpdatePlaying();
    void DrawPlaying() const;
    Card* GetSelectedCard();
    void DeselectAll();
    bool HasSelection() const;
};
