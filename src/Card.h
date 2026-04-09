#pragma once
#include <raylib.h>
#include "Constants.h"

class AssetManager;

class Card {
public:
    CardDef def;
    bool selected;       // selected in hand for placement
    bool draftSelected;  // selected during drafting

    Card();
    Card(CardDef definition);
    void DrawInHand(Rectangle rect, AssetManager* assets = nullptr) const;
    void DrawInDraft(Rectangle rect, AssetManager* assets = nullptr) const;
    int  GetPlacementCost() const;
};
