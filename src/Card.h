#pragma once
#include <raylib.h>
#include "Constants.h"

class Card {
public:
    CardDef def;
    bool selected;       // selected in hand for placement
    bool draftSelected;  // selected during drafting

    Card();
    Card(CardDef definition);
    void DrawInHand(Rectangle rect) const;
    void DrawInDraft(Rectangle rect) const;
    int  GetPlacementCost() const;
};
