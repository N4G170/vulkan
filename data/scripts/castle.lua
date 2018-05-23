--Castle Scene
--Blue defenders
--Red Attackers

--does this file use a function to generate models?
has_generate_models = true;

parent_position = {0,0,0};

--shaders
default_shader = "diffuse";
skybox_shader = "skybox";

paths =
{
    towerSquareMidColorGreen = "data/models/castle/towerSquareMidColorGreen.obj";
    wallPillar = "data/models/castle/wallPillar.obj";
    wallNarrowWood = "data/models/castle/wallNarrowWood.obj";
    wallNarrowStairsFence = "data/models/castle/wallNarrowStairsFence.obj";
    wallNarrowWoodFence = "data/models/castle/wallNarrowWoodFence.obj";
    wallToNarrow = "data/models/castle/wallToNarrow.obj";
    wallCorner = "data/models/castle/wallCorner.obj";
    wallCornerHalfTower = "data/models/castle/wallCornerHalfTower.obj";
    towerTopRoof = "data/models/castle/towerTopRoof.obj";
    wallNarrowGate = "data/models/castle/wallNarrowGate.obj";
    flagBlue = "data/models/castle/flagBlue.obj";
    metalGate = "data/models/castle/metalGate.obj";

    towerSquareBase = "data/models/castle/towerSquareBase.obj";
    towerSquareBaseBorder = "data/models/castle/towerSquareBaseBorder.obj";
    towerSquareMidWindows = "data/models/castle/towerSquareMidWindows.obj";
    towerSquareMidOpen = "data/models/castle/towerSquareMidOpen.obj";
    towerSquareTopRoof = "data/models/castle/towerSquareTopRoof.obj";

    flagBlueWide = "data/models/castle/flagBlueWide.obj";
    towerTopCorner = "data/models/castle/towerTopCorner.obj";
    knightBlue = "data/models/castle/knightBlue.obj";

    sword = "data/models/castle/sword.obj";
    stairsStoneSlant = "data/models/castle/stairsStoneSlant.obj";
    wallNarrowCorner = "data/models/castle/wallNarrowCorner.obj";
    wallNarrow = "data/models/castle/wallNarrow.obj";
    gate = "data/models/castle/gate.obj";
    towerSquareTop = "data/models/castle/towerSquareTop.obj";
    flagBannerLong = "data/models/castle/flagBannerLong.obj";
    king = "data/models/castle/king.obj";
    towerSquareArch = "data/models/castle/towerSquareArch.obj";
    towerSquareTopRoofHigh = "data/models/castle/towerSquareTopRoofHigh.obj";
    siegeBallistaRed = "data/models/castle/siegeBallistaRed.obj";
    knightRed = "data/models/castle/knightRed.obj";
    siegeTowerRed = "data/models/castle/siegeTowerRed.obj";
    siegeCatapultRed = "data/models/castle/siegeCatapultRed.obj";
    siegeTrebuchetRed = "data/models/castle/siegeTrebuchetRed.obj";
    siegeRamRed = "data/models/castle/siegeRamRed.obj";
}

--skybox = { path = "data/models/shapes/cube.obj";    position = {0, 15, 0}, rotation = {0, 0, 0}, scale = {450, 450, 450};  shader = skybox_shader; model_type = "test" };

models =
{
    --base
    -- {   path = paths["towerSquareMidColorGreen"];    position = {250, -2, -250}, rotation = {0, 0, 0}, scale = {50, 1, 50};  shader = default_shader; model_type = ""    },
--<f> Motte
    --<f> WALLS
        --<f> East
        {   path = paths["wallPillar"];      position = {-30, 0, -20},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];      position = {-30, 0, -30},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];      position = {-30, 0, -40},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowWood"];  position = {-40, 0, -40},   rotation = {0, 0, 0},       scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowWood"];  position = {-40, 0, -50},   rotation = {0, 0, 0},       scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];      position = {-30, 0, -70},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];      position = {-30, 0, -80},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];      position = {-30, 0, -90},   rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /East

        --<f> West
        {   path = paths["wallNarrowStairsFence"];   position = {30, 0, -40}, rotation = {0, 180, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowWoodFence"];     position = {30, 0, -50}, rotation = {0, 180, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallToNarrow"];            position = {30, 0, -60}, rotation = {0, 180, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];              position = {20, 0, -60}, rotation = {0, 0, 0},      scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];              position = {20, 0, -70}, rotation = {0, 0, 0},      scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];              position = {20, 0, -80}, rotation = {0, 0, 0},      scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /West

        --<f> South
        {   path = paths["wallPillar"];          position = {10, 0, -100},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallCorner"];          position = {0, 0, -90},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerTopRoof"];        position = {-15, 13, -115}, rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallCornerHalfTower"]; position = {0, 0, -100},    rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowGate"];      position = {-10, 0, -110},  rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallCornerHalfTower"]; position = {-10, 0, -100},  rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerTopRoof"];        position = {15, 13, -115},  rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallCorner"];          position = {-10, 0, -90},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];          position = {-30, 0, -100},  rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /South

        --<f> gatehouse flags + gate
        {   path = paths["flagBlue"];    position = {10, 17, -110},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "flag";  },
        {   path = paths["flagBlue"];    position = {-20, 17, -110},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "flag";  },
        {   path = paths["metalGate"];   position = {-8.2, 0.6, -107.3}, rotation = {0, 90, 90}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /gatehouse flags + gate

        --<f> North
        {   path = paths["wallPillar"];  position = {-30, 0, -10}, rotation = {0, -90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];  position = {-20, 0, -10}, rotation = {0, -90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["wallPillar"];  position = {-10, 0, -10}, rotation = {0, -90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        --</f> North
    --</f> /WALLS

    --<f> Towers
        --<f> SE
        {   path = paths["towerSquareBase"];         position = {-40, 0, -90},       rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerSquareBaseBorder"];   position = {-40, 10, -90},      rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerSquareMidWindows"];   position = {-40, 13, -90.7},    rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerSquareMidOpen"];      position = {-40, 23, -90.7},    rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerSquareTopRoof"];      position = {-40, 33, -90},      rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },

        {   path = paths["flagBlueWide"];            position = {-35, 38.9, -95},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = "flag"   },
        --</f> /SE

        --<f> SW no tower, corner only
        {   path = paths["wallCorner"];  position = {30, 0, -100}, rotation = {0, 180, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        --</f> /SW no tower, corner only

        --<f> NE
        {   path = paths["wallCornerHalfTower"]; position = {-30, 0, -10},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["towerTopCorner"];      position = {-45, 13.1, -5}, rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /NE
    --</f> /Towers

    --<f> Soldiers
        --<f> Courtyard Columns
        --courtyard end right column
        {   path = paths["knightBlue"];  position = {-19, 0, -20}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-19, 0, -25}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-19, 0, -30}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-19, 0, -35}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-19, 0, -40}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },

        --courtyard end left column
        {   path = paths["knightBlue"];  position = {-9, 0, -20}, rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-9, 0, -25}, rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-9, 0, -30}, rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-9, 0, -35}, rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-9, 0, -40}, rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },

        --columns captain
        {   path = paths["knightBlue"];  position = {-7, 0, -50}, rotation = {0, 135, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = ""    },
        --</f> /Courtyard Columns

        --<f> Gate defence
        {   path = paths["knightBlue"];  position = {-13, 0, -85},   rotation = {0, -45, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-8, 0, -80},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-2, 0, -78},    rotation = {0, 10, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {2, 0, -85},     rotation = {0, 45, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        --</f> /Gate defence

        --<f> Walls
        --tower SE
        {   path = paths["knightBlue"];  position = {-34.7, 23.3, -97.9},    rotation = {0, 0, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-36.9, 23.3, -94},      rotation = {0, 90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        --wall S gatehouse
        {   path = paths["knightBlue"];  position = {18.1, 11.8, -95.8},     rotation = {0, -45, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {2.3, 11.8, -104},       rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-13.3, 11.8, -104},     rotation = {0, -45, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-18.1, 11.8, -95.8},    rotation = {0, -45, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = ""    },
        --wall W
        {   path = paths["knightBlue"];  position = {26, 11.8, -80},         rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {25.8, 11.8, -70},       rotation = {0, -90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {26.2, 11.8, -56},       rotation = {0, -90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        --wall E
        {   path = paths["knightBlue"];  position = {-35.2, 11.8, -80},      rotation = {0, 90, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-35, 11.8, -66},        rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-34, 11.8, -50},        rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-36, 11.8, -33},        rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-36.9, 11.8, -17.9},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --Tower NE
        {   path = paths["knightBlue"];  position = {-39.9, 13.1, 0},        rotation = {0, 135, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = ""    },
        --</f> /Walls

        --<f> Wounded
        {   path = paths["knightBlue"];  position = {-17, 0, -70},       rotation = {-90, 0, 45},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-18, 0, -65},       rotation = {-90, 0, -25},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {-23, 0, -65},       rotation = {-90, 0, 185},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["sword"];       position = {-23, 2.9, -63.1},   rotation = {180, 0, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /Wounded
    --</f> /Soldiers
--</f> /Motte

--<f> Bailey
    --<f> Base and Stairs
    --base
    {   path = paths["towerSquareBase"];     position = {0, 10, -30},        rotation = {0, 0, 180}, scale = {6, 4, 6};  shader = default_shader; model_type = ""    },
    --stairs to base
    {   path = paths["stairsStoneSlant"];    position = {13.5, 3.4, -38.2},  rotation = {0, 0, 0},   scale = {3, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["stairsStoneSlant"];    position = {13.5, -0.5, -43},   rotation = {0, 0, 0},   scale = {3, 1, 1};  shader = default_shader; model_type = ""    },
    --</f> /Base and Stairs

    --<f> Walls
        --<f> East
        {   path = paths["wallNarrowCorner"];    position = {0, 10, -30},    rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {0, 10, -15},    rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {0, 10, -5},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {0, 10, 5},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {0, 10, 15},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {0, 10, 25},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowCorner"];    position = {0, 10, 30},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /East

        --<f> South
        {   path = paths["wallNarrowGate"];          position = {5, 10, -30},    rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowWood"];          position = {15, 10, -30},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowStairsFence"];   position = {25, 10, -30},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];              position = {35, 10, -30},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];              position = {45, 10, -30},   rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowCorner"];        position = {60, 10, -30},   rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /South

        --<f> West
        {   path = paths["wallNarrow"];          position = {60, 10, -25},   rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {60, 10, -15},   rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {60, 10, -5},    rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {60, 10, 5},     rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];          position = {60, 10, 15},    rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrowCorner"];    position = {60, 10, 30},    rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /West

        --<f> North
        {   path = paths["wallNarrow"];  position = {15, 10, 30}, rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];  position = {25, 10, 30}, rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];  position = {35, 10, 30}, rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];  position = {45, 10, 30}, rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["wallNarrow"];  position = {55, 10, 30}, rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /North

        --<f> Gate
        {   path = paths["gate"];    position = {13.3, 10, -29.6}, rotation = {0, 90, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = ""    },
        --</f> /Gate
    --</f> /Walls

    --<f> Keep
    {   path = paths["towerSquareBase"];         position = {55.9, 8.3, .9},     rotation = {0, 0, 0},   scale = {2.5, 3, 2.5};  shader = default_shader; model_type = ""    },
    {   path = paths["towerSquareTop"];          position = {55.85, 38.6, .9},   rotation = {0, 0, 0},   scale = {2.5, 1, 2.5};  shader = default_shader; model_type = ""    },
    --banners
    {   path = paths["flagBannerLong"];          position = {43.95, 38.6, .9},   rotation = {0, 90, 0},  scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    {   path = paths["flagBannerLong"];          position = {30.95, 38.6, 12.7}, rotation = {0, 180, 0}, scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    --gate
    {   path = paths["gate"];                    position = {47, 7.6, .9},       rotation = {0, 90, 0},  scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    --king & platform & top flag
    {   path = paths["king"];                    position = {43.55, 40.3, 12.6}, rotation = {0, 0, 0},   scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    {   path = paths["towerSquareArch"];         position = {48.45, 40.3, 9.2},  rotation = {0, 0, 0},   scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    {   path = paths["towerSquareTopRoofHigh"];  position = {48.45, 50.4, 8.5},  rotation = {0, 0, 0},   scale = {1, 1, 1};      shader = default_shader; model_type = ""    },
    {   path = paths["flagBlueWide"];            position = {43.45, 60.1, 13.5}, rotation = {0, 0, 0},   scale = {1, 1, 1};      shader = default_shader; model_type = "flag";  },
    --</f> /Keep

    --<f> Soldiers
        --<f> Wall South
        {   path = paths["knightBlue"];  position = {10, 21.8, -26.4}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {25, 21.8, -26.4}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {45, 21.8, -26.4}, rotation = {0, 0, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /Wall South

        --<f> Wall East
        {   path = paths["knightBlue"];  position = {3.5, 21.8, -15},    rotation = {0, 90, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {3.5, 21.8, 0},      rotation = {0, 90, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {3.5, 21.8, 25},     rotation = {0, 90, 0}, scale = {1, 1, 1};   shader = default_shader; model_type = ""    },
        --</f> /Wall East

        --<f> Wall West
        {   path = paths["knightBlue"];  position = {56.4, 21.8, -14.2}, rotation = {0, -90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> /Wall West

        --<f> Courtyard
        --right column
        {   path = paths["knightBlue"];  position = {13, 10, 0},     rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {13, 10, 5},     rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {13, 10, 10},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {13, 10, 15},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },

        --left column
        {   path = paths["knightBlue"];  position = {20, 10, 0},     rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {20, 10, 5},     rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {20, 10, 10},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        {   path = paths["knightBlue"];  position = {20, 10, 15},    rotation = {0, 0, 0}, scale = {1, 1, 1};    shader = default_shader; model_type = ""    },
        --</f> /Courtyard
    --</f> /Soldiers
--</f> Bailey

--<f> Red Army
    --<f> West
        --<f> Siege Engines & teams
        {   path = paths["siegeBallistaRed"];    position = {130, 0, 10},        rotation = {0, 160, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {139, 0, 18},        rotation = {0, 35, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {138.7, 0, 8.5},     rotation = {0, 135, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        ----------------------------
        {   path = paths["siegeBallistaRed"];    position = {130, 0, -20},       rotation = {0, 140, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {138.5, 0, -15.5},   rotation = {0, 45, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {139, 0, -7.5},      rotation = {0, 45, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        ----------------------------
        {   path = paths["siegeTowerRed"];       position = {100, 0, -65},       rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --siege tower assault team
        {   path = paths["knightRed"];   position = {110, 0, -70},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];   position = {120, 0, -70},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];   position = {130, 0, -70},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];   position = {110, 0, -60},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];   position = {120, 0, -60},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];   position = {130, 0, -60},   rotation = {0, 90, 0},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        ----------------------------
        {   path = paths["siegeCatapultRed"];    position = {135, 0, -115},   rotation = {0, 200, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {140, 0, -127},   rotation = {0, 135, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {150, 0, -115},   rotation = {0, 90, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {145, 0, -105},   rotation = {0, 45, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> Siege Engines & teams
    --</f> West

    --<f> East
        --<f> Siege Engines
        {   path = paths["siegeBallistaRed"];    position = {-110, 0, 55},      rotation = {0, 35, 0},      scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-118.7, 0, 55},    rotation = {0, -90, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-113.1, 0, 64.1},  rotation = {0, 45, 0},      scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        ----------------------------
        {   path = paths["siegeTrebuchetRed"];    position = {-145, 0, -40},    rotation = {0, 0, 0},       scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-155, 0, -49},     rotation = {0, -135, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-155, 0, -40},     rotation = {0, -90, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-155, 0, -31.8},   rotation = {0, -45, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        ----------------------------
        {   path = paths["siegeTrebuchetRed"];    position = {-145, 0, -90},    rotation = {0, -25, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-152, 0, -101},    rotation = {0, -135, 0},    scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-157, 0, -92},     rotation = {0, -90, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        {   path = paths["knightRed"];           position = {-155, 0, -81.8},   rotation = {0, -45, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
        --</f> Siege Engines

        --<f> Patrol
        --closer to wall
        {   path = paths["knightRed"];  position = {-165, 0, 0},        rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-165, 0, -7.5},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-165, 0, -15},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-165, 0, -22.5},    rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-165, 0, -30},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-165, 0, -37.5},    rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },

        --farther from wall
        {   path = paths["knightRed"];  position = {-175, 0, 0},        rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-175, 0, -7.5},     rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-175, 0, -15},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-175, 0, -22.5},    rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-175, 0, -30},      rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        {   path = paths["knightRed"];  position = {-175, 0, -37.5},    rotation = {0, 0, 0},   scale = {1, 1, 1};  shader = default_shader; model_type = "patrol"    },
        --</f> /Patrol
    --</f> East

    --<f> South
    --<f> Ram
    {   path = paths["siegeRamRed"];    position = {-20, 0, -125.9},    rotation = {0, -40, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];      position = {-15, 0, -130.9},    rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];      position = {-20, 0, -135.9},    rotation = {0, 180, 0},     scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    --dead
    {   path = paths["knightRed"];      position = {-29.5, 1.3, -132},  rotation = {90, 180, -65},  scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    --</f> /Ram

    --<f> Assault team
    {   path = paths["knightRed"];  position = {-4, 0, -132.5}, rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];  position = {1, 0, -137.5},  rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];  position = {-9, 0, -137.5}, rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];  position = {1, 0, -142.9},  rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];  position = {-4, 0, -140.9}, rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["knightRed"];  position = {-9, 0, -142.9}, rotation = {0, 180, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    --</f> /Assault team
    --</f> South
--</f> Red Army
};

function GeneratedModels()

    models = {}
    counter = 1;

    --<f> West
    for i = 0, 3, 1
    do
        start_x = 190; start_z = -115;
        squad_z_increase = 30;

        for j = 0, 2, 1
        do
            row_x_increase = 10;
            for k = 0, 1, 1
            do
                column_z_increase = -10;
                x = start_x + row_x_increase * j;
                z = start_z + squad_z_increase * i + column_z_increase * k;
                models[counter] = { path = paths["knightRed"]; position = {x, 0, z}, rotation = {0, 90, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = "" };
                counter = counter + 1;
            end
        end
    end
    --</f> West

    --<f> North
    for i = 0, 1, 1
    do
        start_x = 150; start_z = 150;
        squad_x_decrease = -200;

        for j = 0, 2, 1
        do
            row_z_increase = 10;
            for k = 0, 5, 1
            do
                column_x_decrease = -10;
                x = start_x + squad_x_decrease * i + column_x_decrease * k;
                z = start_z + row_z_increase * j;
                models[counter] = { path = paths["knightRed"]; position = {x, 0, z}, rotation = {0, 0, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = "" };
                counter = counter + 1;
            end
        end
    end
    --</f> North

    --<f> South
    for i = 0, 0, 1
    do
        start_x = 15; start_z = -180;
        squad_x_decrease = 0;

        for j = 0, 2, 1
        do
            row_z_decrease = 10;
            for k = 0, 4, 1
            do
                column_x_decrease = -10;
                x = start_x + squad_x_decrease * i + column_x_decrease * k;
                z = start_z + row_z_decrease * j;
                models[counter] = { path = paths["knightRed"]; position = {x, 0, z}, rotation = {0, 180, 0}, scale = {1, 1, 1}; shader = default_shader; model_type = "" };
                counter = counter + 1;
            end
        end
    end
    --</f> South

    return models;
end
