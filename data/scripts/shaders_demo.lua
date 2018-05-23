--Models using different shaders

--does this file use a function to generate models?
has_generate_models = false;

--shaders
default_shader = "diffuse";
wireframe_shader = "wireframe";
toon_shader = "toon";
points_shader = "points";

parent_position = {0,0,0};

paths =
{
    siegeBallista = "data/models/castle/siegeBallista.obj";
    siegeTower = "data/models/castle/siegeTower.obj";
    siegeCatapult = "data/models/castle/siegeCatapult.obj";
    siegeTrebuchet = "data/models/castle/siegeTrebuchet.obj";
    siegeRam = "data/models/castle/siegeRam.obj";
}

wireframe_x = -225;
solid_x = -200;
toon_x = -200;
points_x = -225;

models =
{

    --<f> Wireframe
    {   path = paths["siegeBallista"];  position = {wireframe_x, 0, -80},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = wireframe_shader; model_type = ""    },
    {   path = paths["siegeTower"];     position = {wireframe_x, 0, -110},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = wireframe_shader; model_type = ""    },
    {   path = paths["siegeRam"];       position = {wireframe_x, 0, -140},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = wireframe_shader; model_type = ""    },
    {   path = paths["siegeTrebuchet"]; position = {wireframe_x, 0, -170},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = wireframe_shader; model_type = ""    },
    {   path = paths["siegeCatapult"];  position = {wireframe_x, 0, -200},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = wireframe_shader; model_type = ""    },
    --</f> /Wireframe

    --<f> Solid
    {   path = paths["siegeBallista"];  position = {solid_x, 0, -80},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["siegeTower"];     position = {solid_x, 0, -110},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["siegeRam"];       position = {solid_x, 0, -140},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["siegeTrebuchet"]; position = {solid_x, 0, -170},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    {   path = paths["siegeCatapult"];  position = {solid_x, 0, -200},  rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = default_shader; model_type = ""    },
    --</f> /Solid

    --<f> Toon
    {   path = paths["siegeBallista"];  position = {toon_x, 0, 80},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = toon_shader; model_type = ""    },
    {   path = paths["siegeTower"];     position = {toon_x, 0, 50},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = toon_shader; model_type = ""    },
    {   path = paths["siegeRam"];       position = {toon_x, 0, 20},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = toon_shader; model_type = ""    },
    {   path = paths["siegeTrebuchet"]; position = {toon_x, 0, -10},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = toon_shader; model_type = ""    },
    {   path = paths["siegeCatapult"];  position = {toon_x, 0, -40},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = toon_shader; model_type = ""    },
    --</f> /Toon

    --<f> Points
    {   path = paths["siegeBallista"];  position = {points_x, 0, 80},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = points_shader; model_type = ""    },
    {   path = paths["siegeTower"];     position = {points_x, 0, 50},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = points_shader; model_type = ""    },
    {   path = paths["siegeRam"];       position = {points_x, 0, 20},    rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = points_shader; model_type = ""    },
    {   path = paths["siegeTrebuchet"]; position = {points_x, 0, -10},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = points_shader; model_type = ""    },
    {   path = paths["siegeCatapult"];  position = {points_x, 0, -40},   rotation = {0, 90, 0}, scale = {1, 1, 1};  shader = points_shader; model_type = ""    },
    --</f> /Points
}
